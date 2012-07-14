#include "lac_base.h"
#include "statmch.h"
#include "lac_port.h"
#include "lac_sys.h"
#include "../lac_out.h"

/* The Receive Machine */

#define STATES { \
		       CHOOSE(RXM_INITIALIZE), \
			   CHOOSE(RXM_PORT_DISABLED), \
			   CHOOSE(RXM_EXPIRED),	 \
			   CHOOSE(RXM_CURRENT),  \
			   CHOOSE(RXM_LACP_DISABLED),  \
			   CHOOSE(RXM_DEFAULTED), \
			 }

#define GET_STATE_NAME lac_rx_get_state_name
#include "choose.h"


static void _lac_rxm_copy_info(LAC_PORT_INFO *from, LAC_PORT_INFO *to)
{
    to->port_priority		  = from->port_priority;
    to->port_index            = from->port_index;
    to->system_priority 	  = from->system_priority;
    memcpy(to->system_mac, from->system_mac, 6);
    to->key 				  = from->key;
    to->state                 = from->state;
}

static void _lac_rxm_copy_info_from_net(LAC_PORT_INFO *from, LAC_PORT_INFO *to)
{
    to->port_priority		  = ntohs(from->port_priority);
    to->port_index            = ntohs(from->port_index);
    to->system_priority 	  = ntohs(from->system_priority);
    memcpy(to->system_mac, from->system_mac, 6);
    to->key 				  = ntohs(from->key);
    to->state		          = from->state;
}

static Bool _lac_rxm_same_port(LAC_PORT_INFO *a, LAC_PORT_INFO *b)
{
    return (  (a->port_index == b->port_index)
              && (!memcmp(a->system_mac, b->system_mac, 6))
           );
}
static Bool _lac_rxm_same_partner(LAC_PORT_INFO *a, LAC_PORT_INFO *b)
{   
    return (  (a->port_priority == b->port_priority)
              && (a->port_index == b->port_index)
              && (a->system_priority == b->system_priority)
              && (!memcmp(a->system_mac, b->system_mac, 6))
              && (a->key == b->key)
              && (LAC_STATE_GET_BIT(a->state, LAC_STATE_AGG) == LAC_STATE_GET_BIT(b->state, LAC_STATE_AGG))
           );
}

int lac_rxm_rx_lacpdu (LAC_PORT_T * port, LACPDU_T *Lacpdu, int len)
{
    LAC_PORT_T *p;

    /* msg check */

    /* statistic */
    port->rx_lacpdu_cnt++;

    if (!port->lacp_enabled)
        return 0;

    port->rcvdLacpdu = True;

    _lac_rxm_copy_info_from_net(&Lacpdu->actor, &port->msg_actor);
    _lac_rxm_copy_info_from_net(&Lacpdu->partner, &port->msg_partner);

    for (p = port->system->ports; p; p=p->next)
    {
        if (p == port) continue;

        if (p->rx->State == RXM_PORT_DISABLED
            && _lac_rxm_same_port(&p->partner, &port->msg_actor))
        {
            lac_trace("port %d ' partner  moved to port %d \r\n", p->port_index, port->port_index);
            p->port_moved = True;
            break;
        }
    }

    return 0;
}

static void actor_default(LAC_PORT_T *port)
{
    _lac_rxm_copy_info(&port->actor_admin, &port->actor);
}

static void record_default(LAC_PORT_T *port)
{
    _lac_rxm_copy_info(&port->partner_admin, &port->partner);
    LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DEF, True);
}

static void update_partner_syn(LAC_PORT_T *port)
{
    Bool partner_sync = False;
    Bool partner_matched = False;

    if ((_lac_rxm_same_partner(&port->msg_partner, &port->actor)
        || (!LAC_STATE_GET_BIT(port->msg_actor.state, LAC_STATE_AGG)))
        && ((LAC_STATE_GET_BIT(port->msg_actor.state, LAC_STATE_ACT))
            || (LAC_STATE_GET_BIT(port->actor.state, LAC_STATE_ACT)
                &&  LAC_STATE_GET_BIT(port->msg_partner.state, LAC_STATE_ACT))))
    {
            partner_matched = True;     
    }
    else
    {
        lac_trace("\r\n port:%d partner don't know him  ! ", port->port_index);
        port->ntt = True;
        partner_matched = False;
    }

    partner_sync = partner_matched && LAC_STATE_GET_BIT(port->partner.state, LAC_STATE_SYN);
    lac_trace("rx fsm partner match check. port %d partner  syn ---> %d", port->port_index, partner_sync);
    LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_SYN, partner_sync);
}

static void update_selected(LAC_PORT_T *port)
{
    if (!_lac_rxm_same_partner(&port->msg_actor, &port->partner))
    {
        lac_trace("<%s.%d> port:%d has new partner", __FUNCTION__, __LINE__, port->port_index);
        port->selected	= False;
        lac_port_set_reselect(port);
//        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
    }
}

static void update_ntt(LAC_PORT_T *port)
{
    if ( !(_lac_rxm_same_partner(&port->msg_partner, &port->actor))
         || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_ACT))
         || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_AGG))
         || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_SYN))
         || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_COL))
       )
        port->ntt = True;
}

static void record_pdu(LAC_PORT_T *port)
{
    _lac_rxm_copy_info(&port->msg_actor, &port->partner);
    LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DEF, False);
}

static void update_default_selected(LAC_PORT_T *port)
{
    if (!_lac_rxm_same_partner(&port->partner_admin, &port->partner))
    {
        lac_trace("<%s.%d> port:%d to default make selected false.", __FUNCTION__, __LINE__, port->port_index);
        port->selected = False;
        lac_trace("rx fsm to default. port %d actor  syn ---> False", port->port_index);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
    }
}

static void start_current_while_timer(LAC_PORT_T *port, Bool timeout)
{
    if (timeout == SHORT_TIMEOUT)
        port->current_while = port->system->short_timeout_time;
    else
        port->current_while = port->system->long_timeout_time;
}

void lac_rx_enter_state (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;

    switch (this->State) {
    case BEGIN:
    case RXM_INITIALIZE:
        port->selected = False;
        actor_default(port);
        record_default(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP, False);
        port->port_moved = False;
        break;

    case RXM_PORT_DISABLED:
            lac_trace("rx fsm to disabled. port %d partner  syn ---> False", port->port_index);
        LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_SYN, False);
        port->rcvdLacpdu = False;
        port->current_while = 0;
        port->selected = False;
        break;

    case RXM_LACP_DISABLED:
        port->selected 				 = False;
        record_default(port);
        LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_AGG, False);
        break;

    case RXM_EXPIRED:
            lac_trace("rx fsm to expired. port %d partner syn ---> False", port->port_index);
        LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_SYN, False);
        LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_TMT, SHORT_TIMEOUT);
        start_current_while_timer(port, SHORT_TIMEOUT);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP, True);
        break;

    case RXM_DEFAULTED:
        update_default_selected(port);
        record_default(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP, False);
        break;

    case RXM_CURRENT:
        update_selected(port);
        update_ntt(port);
        record_pdu(port);
        update_partner_syn( port);
        start_current_while_timer(port, LAC_STATE_GET_BIT(port->actor.state, LAC_STATE_TMT));
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP , False);
        port->rcvdLacpdu = False;
        break;
    }
}

Bool lac_rx_check_conditions (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;

    if (!port->port_enabled && !port->port_moved && BEGIN != this->State) 
    {
        if (this->State == RXM_PORT_DISABLED)
            return False;
        else
            return lac_hop_2_state (this, RXM_PORT_DISABLED);
    }

    switch (this->State) {
    case BEGIN:
        return lac_hop_2_state (this, RXM_INITIALIZE);

    case RXM_INITIALIZE:
        return lac_hop_2_state (this, RXM_PORT_DISABLED);

    case RXM_PORT_DISABLED:
            if (port->port_moved)
                    return lac_hop_2_state (this, RXM_INITIALIZE);

        if (port->port_enabled && port->lacp_enabled)
        {
            return lac_hop_2_state (this, RXM_EXPIRED);
        }
        if (port->port_enabled && !port->lacp_enabled)
        {
            return lac_hop_2_state (this, RXM_LACP_DISABLED);
        }

        break;

    case RXM_LACP_DISABLED:
        if (port->port_enabled 
                && port->lacp_enabled )
        {
            return lac_hop_2_state (this, RXM_EXPIRED);
        }
        break;

    case RXM_EXPIRED:
        if (!port->current_while)
        {
            return lac_hop_2_state (this, RXM_DEFAULTED);
        }
        if (port->rcvdLacpdu)
        {
            return lac_hop_2_state (this, RXM_CURRENT);
        }
        break;

    case RXM_CURRENT:
        if (!LAC_STATE_GET_BIT(port->msg_actor.state, LAC_STATE_AGG)
                || (!port->current_while && !port->rcvdLacpdu)) {
            return lac_hop_2_state (this, RXM_EXPIRED);
        }
        if (port->rcvdLacpdu) {
            return lac_hop_2_state (this, RXM_CURRENT);
        }
        break;

    case RXM_DEFAULTED:
        if (port->rcvdLacpdu)
        {
            return lac_hop_2_state (this, RXM_CURRENT);
        }
        break;

    default:
        break;
    }

    return False;
}

