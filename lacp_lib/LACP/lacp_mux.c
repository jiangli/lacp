#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "trunk_ssp.h"

#define STATES {        \
  CHOOSE(MUX_DETACHED),    \
  CHOOSE(MUX_DISABLE),    \
  CHOOSE(MUX_WAITING),    \
  CHOOSE(MUX_READY_N),         \
  CHOOSE(MUX_ATTACHED),         \
  CHOOSE(MUX_RX_TX),  \
}

#define GET_STATE_NAME lacp_mux_get_state_name
#include "lacp_choose.h"

static Bool sys_is_ready(lacp_port_t *port)
{
    lacp_port_t *p = NULL;
    lacp_sys_t *lac_sys = lacp_get_sys_inst();

    for (p = lac_sys->ports; p; p = p->next)
    {
        if (port->agg_id != p->agg_id )
            continue;

        if (p->selected && !p->standby && !p->ready_n)
            return False;
    }

    return True;
}

static uint32_t disable_collecting_distributing(lacp_port_t  *port)
{
    return trunk_ssp_set_port_cd(port->port_index, False);
}

static uint32_t enable_collecting_distributing(lacp_port_t  *port)
{
    return trunk_ssp_set_port_cd(port->port_index, True);
}

static uint32_t detach_mux_from_aggregator(lacp_port_t  *port)
{
    return trunk_ssp_attach_port(port->port_index, False, port->agg_id);
}

static uint32_t attach_mux_to_aggregator(lacp_port_t  *port)
{
    return trunk_ssp_attach_port(port->port_index, True, port->agg_id);
}

void lacp_mux_enter_state (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    switch (fsm->state) {
    case LACP_BEGIN:
    case MUX_DISABLE:
        detach_mux_from_aggregator(port);
        if (fsm->debug)
            trunk_trace("mux to disable state. port %d actor syn ---> False", port->port_index);

        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_SYN, False);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_COL, True);
        enable_collecting_distributing(port);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_DIS, True);
        port->ntt = False;

        if (fsm->debug)
            trunk_trace("mux to disable state. port %d collecting and distributing ---> True", port->port_index);
        break;

    case MUX_DETACHED:
        detach_mux_from_aggregator(port);
        if (fsm->debug)
            trunk_trace("mux to detached state.port %d actor syn ---> False", port->port_index);

        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_SYN, False);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_COL, False);
        disable_collecting_distributing(port);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_DIS, False);
        if (fsm->debug)
            trunk_trace("mux to DETACHED state. port %d collecting and distributing ---> False", port->port_index);
        port->ntt = True;
        break;

    case MUX_WAITING:
        port->wait_while = port->system->aggregate_wait_time;
        if (fsm->debug)
            trunk_trace("mux to WAITING state. port %d start wait_while timer %d", port->port_index, port->wait_while);
        port->ready_n = False;
        break;

    case MUX_READY_N:
        if (fsm->debug)
            trunk_trace(" port %d is ready.", port->port_index);

        port->ready_n = True;
        break;

    case MUX_ATTACHED:
        attach_mux_to_aggregator(port);

        if (fsm->debug)
        {
            trunk_trace("attch port %d to agg %d", port->port_index, port->agg_id);
            trunk_trace("mux to MUX_ATTACHED state. port %d actor syn ---> True", port->port_index);
        }

        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_SYN, True);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_COL, False);
        disable_collecting_distributing(port);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_DIS, False);

        if (fsm->debug)
            trunk_trace("mux to MUX_ATTACHED state. port %d collecting and distributing ---> False", port->port_index);

        port->ntt = True;
        break;

    case MUX_RX_TX:
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_DIS, True);
        enable_collecting_distributing(port);
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_COL, True);
        if (fsm->debug)
            trunk_trace("mux to RX_TX status. port %d collecting and distributing ---> True", port->port_index);

        port->ntt = True;
        break;

    };
}

Bool lacp_mux_check_conditions (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;
    if (!port->lacp_enabled)
    {
        if (fsm->state == MUX_DISABLE)
            return False;
        else
            return lacp_hop_2_state (fsm, MUX_DISABLE);
    }

    switch (fsm->state) {
    case LACP_BEGIN:
        return lacp_hop_2_state (fsm, MUX_DETACHED);

    case MUX_DISABLE:
        if (port->lacp_enabled)
            return lacp_hop_2_state (fsm, MUX_DETACHED);

    case MUX_DETACHED:
        if (port->selected && !port->standby)
            return lacp_hop_2_state (fsm, MUX_WAITING);
        break;

    case MUX_WAITING:
        if (!port->wait_while)
            return lacp_hop_2_state (fsm, MUX_READY_N);
        break;

    case MUX_READY_N:
        if (port->selected && !port->standby && sys_is_ready(port))
            return lacp_hop_2_state (fsm, MUX_ATTACHED);

        if (!port->selected || port->standby)
            return lacp_hop_2_state (fsm, MUX_DETACHED);
        break;


    case MUX_ATTACHED:
        if (port->selected && !port->standby && LACP_STATE_GET_BIT(port->partner.state, LACP_STATE_SYN))
            return lacp_hop_2_state (fsm, MUX_RX_TX);

        if (!port->selected || port->standby)
            return lacp_hop_2_state (fsm, MUX_DETACHED);
        break;

    case MUX_RX_TX:
        if (!port->selected || port->standby || !LACP_STATE_GET_BIT(port->partner.state, LACP_STATE_SYN))
            return lacp_hop_2_state (fsm, MUX_ATTACHED);
        break;

    default:
        break;
    };
    return False;
}
