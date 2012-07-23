#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "../lacp_ssp.h"

/* The Receive Machine */

#define STATES { \
		       CHOOSE(RXM_INITIALIZE), \
			   CHOOSE(RXM_PORT_DISABLED), \
			   CHOOSE(RXM_EXPIRED),	 \
			   CHOOSE(RXM_CURRENT),  \
			   CHOOSE(RXM_LACP_DISABLED),  \
			   CHOOSE(RXM_DEFAULTED), \
			 }

#define GET_STATE_NAME lacp_rx_get_state_name
#include "lacp_choose.h"


static void _lacp_rxm_copy_info(lacp_port_info_t *from, lacp_port_info_t *to)
{
    to->port_priority		  = from->port_priority;
    to->port_no            = from->port_no;
    to->system_priority 	  = from->system_priority;
    memcpy(to->system_mac, from->system_mac, 6);
    to->key 				  = from->key;
    to->state                 = from->state;
}

static void _lacp_rxm_copy_info_from_net(lacp_port_info_t *from, lacp_port_info_t *to)
{
    to->port_priority		  = ntohs(from->port_priority);
    to->port_no            = ntohs(from->port_no);
    to->system_priority 	  = ntohs(from->system_priority);
    memcpy(to->system_mac, from->system_mac, 6);
    to->key 				  = ntohs(from->key);
    to->state		          = from->state;
}

static Bool _lacp_rxm_same_port(lacp_port_info_t *a, lacp_port_info_t *b)
{
    return (  (a->port_no == b->port_no)
              && (!memcmp(a->system_mac, b->system_mac, 6))
           );
}
static Bool _lacp_rxm_same_partner(lacp_port_info_t *a, lacp_port_info_t *b)
{
    return (  (a->port_priority == b->port_priority)
              && (a->port_no == b->port_no)
              && (a->system_priority == b->system_priority)
              && (!memcmp(a->system_mac, b->system_mac, 6))
              && (a->key == b->key)
              && (LACP_STATE_GET_BIT(a->state, LACP_STATE_AGG) == LACP_STATE_GET_BIT(b->state, LACP_STATE_AGG))
           );
}

uint32_t lacp_rxm_rx_lacpdu (lacp_port_t * port, lacp_pdu_t *Lacpdu, uint32_t len)
{
    lacp_port_t *p;

    /* msg check */

    /* statistic */
    port->rx_lacpdu_cnt++;

    if (!port->lacp_enabled)
        return 0;

    port->rcvd_lacpdu = True;

    _lacp_rxm_copy_info_from_net(&Lacpdu->actor, &port->msg_actor);
    _lacp_rxm_copy_info_from_net(&Lacpdu->partner, &port->msg_partner);

    for (p = port->system->ports; p; p=p->next)
    {
        if (p == port) continue;

        if (p->rx->state == RXM_PORT_DISABLED
                && _lacp_rxm_same_port(&p->partner, &port->msg_actor))
        {
            lacp_trace("port %d ' partner  moved to port %d \r\n", p->port_index, port->port_index);
            p->port_moved = True;
            break;
        }
    }

    return 0;
}

static void actor_default(lacp_port_t *port)
{
    _lacp_rxm_copy_info(&port->actor_admin, &port->actor);
}

static void record_default(lacp_port_t *port)
{
    _lacp_rxm_copy_info(&port->partner_admin, &port->partner);
    LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_DEF, True);
}

static void update_partner_syn(lacp_port_t *port)
{
    Bool partner_sync = False;
    Bool partner_matched = False;

    if ((_lacp_rxm_same_partner(&port->msg_partner, &port->actor)
            || (!LACP_STATE_GET_BIT(port->msg_actor.state, LACP_STATE_AGG)))
            && ((LACP_STATE_GET_BIT(port->msg_actor.state, LACP_STATE_ACT))
                || (LACP_STATE_GET_BIT(port->actor.state, LACP_STATE_ACT)
                    &&  LACP_STATE_GET_BIT(port->msg_partner.state, LACP_STATE_ACT))))
    {
        partner_matched = True;
    }
    else
    {
        lacp_trace("\r\n port:%d 's partner didn't catch up with him  ! ", port->port_index);
        port->ntt = True;
        partner_matched = False;
    }

    partner_sync = partner_matched && LACP_STATE_GET_BIT(port->partner.state, LACP_STATE_SYN);
    lacp_trace("rx fsm partner match check. port %d partner  syn ---> %d", port->port_index, partner_sync);
    LACP_STATE_SET_BIT(port->partner.state, LACP_STATE_SYN, partner_sync);
}

static void update_selected(lacp_port_t *port)
{
    if (!_lacp_rxm_same_partner(&port->msg_actor, &port->partner))
    {
        lacp_trace("<%s.%d> port:%d has new partner", __FUNCTION__, __LINE__, port->port_index);
        port->selected	= False;
        lacp_port_set_reselect(port);
//        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_SYN, False);
    }
}

static void update_ntt(lacp_port_t *port)
{
    if ( !(_lacp_rxm_same_partner(&port->msg_partner, &port->actor))
            || (LACP_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LACP_STATE_ACT))
            || (LACP_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LACP_STATE_TMT))
            || (LACP_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LACP_STATE_SYN))
            || (LACP_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LACP_STATE_AGG))
       )
        port->ntt = True;
}

static void record_pdu(lacp_port_t *port)
{
    _lacp_rxm_copy_info(&port->msg_actor, &port->partner);
    LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_DEF, False);
}

static void update_default_selected(lacp_port_t *port)
{
    if (!_lacp_rxm_same_partner(&port->partner_admin, &port->partner))
    {
        lacp_trace("<%s.%d> port:%d to default make selected false.", __FUNCTION__, __LINE__, port->port_index);
        port->selected = False;
        lacp_trace("rx fsm to default. port %d actor  syn ---> False", port->port_index);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_SYN, False);
    }
}

static void start_current_while_timer(lacp_port_t *port, Bool timeout)
{
    if (timeout == LACP_SHORT_TIMEOUT)
        port->current_while = port->system->short_timeout_time;
    else
        port->current_while = port->system->long_timeout_time;
}

void lacp_rx_enter_state (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    switch (fsm->state) {
    case LACP_BEGIN:
    case RXM_INITIALIZE:
        port->selected = False;
        actor_default(port);
        record_default(port);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_EXP, False);
        port->port_moved = False;
        break;

    case RXM_PORT_DISABLED:
        lacp_trace("rx fsm to disabled. port %d partner  syn ---> False", port->port_index);
        LACP_STATE_SET_BIT(port->partner.state, LACP_STATE_SYN, False);
        port->rcvd_lacpdu = False;
        port->current_while = 0;
        port->selected = False;
        break;

    case RXM_LACP_DISABLED:
        port->selected 				 = False;
        record_default(port);
        LACP_STATE_SET_BIT(port->partner.state, LACP_STATE_AGG, False);
        break;

    case RXM_EXPIRED:
        lacp_trace("rx fsm to expired. port %d partner syn ---> False", port->port_index);
        LACP_STATE_SET_BIT(port->partner.state, LACP_STATE_SYN, False);
        LACP_STATE_SET_BIT(port->partner.state, LACP_STATE_TMT, LACP_SHORT_TIMEOUT);
        start_current_while_timer(port, LACP_SHORT_TIMEOUT);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_EXP, True);
        break;

    case RXM_DEFAULTED:
        update_default_selected(port);
        record_default(port);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_EXP, False);
        break;

    case RXM_CURRENT:
        update_selected(port);
        update_ntt(port);
        record_pdu(port);
        update_partner_syn( port);
        start_current_while_timer(port, LACP_STATE_GET_BIT(port->actor.state, LACP_STATE_TMT));
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_EXP , False);
        port->rcvd_lacpdu = False;
        break;
    }
}

Bool lacp_rx_check_conditions (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    if (!port->port_enabled && !port->port_moved && LACP_BEGIN != fsm->state)
    {
        if (fsm->state == RXM_PORT_DISABLED)
            return False;
        else
            return lacp_hop_2_state (fsm, RXM_PORT_DISABLED);
    }

    switch (fsm->state) {
    case LACP_BEGIN:
        return lacp_hop_2_state (fsm, RXM_INITIALIZE);

    case RXM_INITIALIZE:
        return lacp_hop_2_state (fsm, RXM_PORT_DISABLED);

    case RXM_PORT_DISABLED:
        if (port->port_moved)
            return lacp_hop_2_state (fsm, RXM_INITIALIZE);

        if (port->port_enabled && port->lacp_enabled)
        {
            return lacp_hop_2_state (fsm, RXM_EXPIRED);
        }
        if (port->port_enabled && !port->lacp_enabled)
        {
            return lacp_hop_2_state (fsm, RXM_LACP_DISABLED);
        }

        break;

    case RXM_LACP_DISABLED:
        if (port->port_enabled && port->lacp_enabled )
        {
            return lacp_hop_2_state (fsm, RXM_EXPIRED);
        }
        break;

    case RXM_EXPIRED:
        if (!port->current_while)
        {
            return lacp_hop_2_state (fsm, RXM_DEFAULTED);
        }
        if (port->rcvd_lacpdu)
        {
            return lacp_hop_2_state (fsm, RXM_CURRENT);
        }
        break;

    case RXM_CURRENT:
        if (!port->current_while && !port->rcvd_lacpdu) {
            return lacp_hop_2_state (fsm, RXM_EXPIRED);
        }
        if (port->rcvd_lacpdu) {
            return lacp_hop_2_state (fsm, RXM_CURRENT);
        }
        break;

    case RXM_DEFAULTED:
        if (port->rcvd_lacpdu)
        {
            return lacp_hop_2_state (fsm, RXM_CURRENT);
        }
        break;

    default:
        break;
    }

    return False;
}

