#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_sys.h"
#include "trunk_ssp.h"
#define STATES {        \
  CHOOSE(SEL_INIT),    \
  CHOOSE(SEL_SELECTION),    \
}

#define GET_STATE_NAME lacp_sel_get_state_name
#include "lacp_choose.h"

static Bool partners_aport(lacp_port_t *port)
{
    lacp_port_t *p = NULL;

    for (p = port->system->ports; p; p = p->next)
    {
        if (port->agg_id != p->agg_id )
            continue;

        if(  (p->actor.system_priority   == port->partner.system_priority     )
                && (!memcmp(p->actor.system_mac, port->partner.system_mac, 6)           )
                && (p->actor.key               == port->partner.key                 )
                && (p->actor.port_no           == port->partner.port_no             )
                &&((p->actor.port_priority     <  port->actor.port_priority )
                   ||( (p->actor.port_priority  == port->actor.port_priority )
                       &&(p->actor.port_no        <  port->actor.port_no       )
                     )   ) )
            return(True);
    }

    return(False);
}

static lacp_port_t *select_master_port(lacp_sys_t *lac_sys, uint32_t agg_id)
{

    lacp_port_t *best = NULL;
    lacp_port_t *p = NULL;

    //update port info
    for (p = lac_sys->ports; p; p = p->next)
    {
        if (agg_id != p->agg_id )
            continue;

        p->actor.key = trunk_ssp_get_port_oper_key(p->port_index);
    }

    /* select best port(master port) */
    for (p = lac_sys->ports; p; p = p->next)
    {
        /* reduce the search range */
        if (agg_id != p->agg_id || !p->port_enabled || !p->lacp_enabled || !p->duplex)
        {
            continue;
        }

        if (!best)
        {
            best = p;
            continue;
        }

        if( p->speed > best->speed)
        {
            best = p;
            continue;

        }
        else if ( p->speed < best->speed)
        {
            continue;
        }
        else if (p->actor.port_priority < best->actor.port_priority)
        {
            best = p;
            continue;

        }
        else if     (p->actor.port_priority > best->actor.port_priority)
        {
            continue;

        }
        else if ( (!LACP_STATE_GET_BIT(p->actor.state, LACP_STATE_DEF))
                  &&  LACP_STATE_GET_BIT(best->actor.state, LACP_STATE_DEF))
        {
            best = p;
            continue;

        }
        else if ( (LACP_STATE_GET_BIT(p->actor.state, LACP_STATE_DEF))
                  &&  !LACP_STATE_GET_BIT(best->actor.state, LACP_STATE_DEF))
        {
            continue;

        }
        else if (p->actor.port_no < best->actor.port_no)
        {
            best = p;
            continue;

        }
    }
#if 0
    if (best)
        trunk_trace("\r\n agg %d get best port: %d", agg_id, best->port_index);
    else
        trunk_trace("\r\n agg %d NO best port.", agg_id);
#endif
    return best;

}

static uint32_t update_agg_ports_select(lacp_sys_t *lac_sys, uint32_t agg_id)
{
    lacp_port_t *best = NULL;
    lacp_port_t *port = NULL;
    best = select_master_port(lac_sys, agg_id);

    for (port = lac_sys->ports; port; port = port->next)
    {
        /* reduce the search range */
        if (port->agg_id != agg_id)
        {
            continue;
        }

        if (!best)
        {
            port->selected = True;
            port->standby = True;
            port->master_port = port;
            continue;
        }

        port->master_port = best;

        if(port->port_enabled
                && port->lacp_enabled
                && port->actor.key == best->actor.key
                && port->partner.system_priority == best->partner.system_priority
                && (!memcmp(port->partner.system_mac, best->partner.system_mac, 6))
                && (port->partner.key == best->partner.key)
                && LACP_STATE_GET_BIT(port->actor.state, LACP_STATE_AGG)
                && LACP_STATE_GET_BIT(port->partner.state, LACP_STATE_AGG)
                && LACP_STATE_GET_BIT(best->actor.state, LACP_STATE_AGG)
                && LACP_STATE_GET_BIT(best->partner.state, LACP_STATE_AGG)
                && !partners_aport(port))
        {
            port->selected = True;
            port->standby = False;
            if (port->sel->debug)
                trunk_trace("\r\n <%s.%d> port %d ---> Selected",  __FUNCTION__, __LINE__, port->port_index);
        } else {
            port->selected = True;
            port->standby = True;
            if (port->sel->debug)
                trunk_trace("\r\n <%s.%d> port %d ---> Standby ",  __FUNCTION__, __LINE__, port->port_index);
        }
    }
    return 0;

}


uint32_t selection_logic(lacp_port_t *port)
{
    lacp_sys_t *lac_sys = port->system;

    if (!port->lacp_enabled)
    {
        port->selected = True;
        port->standby = True;
        return 0;
    }

    if (!port->agg_id)
    {
        return 1;
    }

    update_agg_ports_select(lac_sys, port->agg_id);

    return 0;
}

static uint32_t lacp_select(lacp_port_t *port)
{
    return selection_logic(port);
}

void lacp_sel_enter_state (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    switch (fsm->state) {
    case LACP_BEGIN:
    case SEL_INIT:
        port->selected = False;
        break;

    case SEL_SELECTION:
        lacp_select(port);
        break;
    };

    return ;

}

Bool lacp_sel_check_conditions (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    switch (fsm->state) {
    case LACP_BEGIN:
    case SEL_INIT:
        return lacp_hop_2_state (fsm, SEL_SELECTION);

    case SEL_SELECTION:
        if (!port->selected)
        {
            return lacp_hop_2_state (fsm, SEL_SELECTION);
        }
        break;

    default:
        break;
    };
    return False;
}

