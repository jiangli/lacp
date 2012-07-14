#include "lac_base.h"
#include "statmch.h"
#include "lac_port.h"
#include "lac_sys.h"
#include "../lac_out.h"

#define STATES {        \
  CHOOSE(DETACHED),    \
  CHOOSE(DISABLE),    \
  CHOOSE(WAITING),    \
CHOOSE(READY_N),         \
  CHOOSE(ATTACHED),         \
  CHOOSE(RX_TX),  \
}

#define GET_STATE_NAME lac_mux_get_state_name
#include "choose.h"

int sys_is_ready(LAC_PORT_T *port)
{
    LAC_PORT_T *p = NULL;
    LAC_SYS_T *this = lac_get_sys_inst();

    //update port info
    for (p = this->ports; p; p = p->next)
    {
        if (port->agg_id != p->agg_id )
            continue;

        if (p->selected && !p->standby && !p->ready_n)
            return False;
    }

    return True;

}
int disable_collecting_distributing(LAC_PORT_T  *port)
{
    lac_trace("\r\n <%s.%d>",  __FUNCTION__, __LINE__);
    lac_set_port_cd(port->port_index, False);
    return 0;

}
int enable_collecting_distributing(LAC_PORT_T  *port)
{
    lac_trace("\r\n <%s.%d>",  __FUNCTION__, __LINE__);
    lac_set_port_cd(port->port_index, True);
    return 0;
}

int detach_mux_from_aggregator(LAC_PORT_T  *port)
{
    lac_trace("\r\n <%s.%d>, port:%d, agg:%d",  __FUNCTION__, __LINE__, port->port_index, port->agg_id);
    lac_set_port_attach_to_tid(port->port_index, False, port->agg_id);
    return 0;

}
int attach_mux_to_aggregator(LAC_PORT_T  *port)
{
    lac_trace("\r\n <%s.%d>, port:%d, agg:%d",  __FUNCTION__, __LINE__, port->port_index, port->agg_id);
    lac_set_port_attach_to_tid(port->port_index, True, port->agg_id);
    return 0;

}

void lac_mux_enter_state (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;

    switch (this->State) {
    case BEGIN:
    case DISABLE:
        detach_mux_from_aggregator(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, True);
        enable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, True);
        port->ntt = False;
        break;

    case DETACHED:
        detach_mux_from_aggregator(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, False);
        disable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, False);
        port->ntt = True;
        break;

    case WAITING:
        port->wait_while = port->system->aggregate_wait_time;
        port->ready_n = False;

        break;

    case READY_N:
        port->ready_n = True;
        break;

    case ATTACHED:
        attach_mux_to_aggregator(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, True);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, False);
        disable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, False);
        port->ntt = True;
        break;

    case RX_TX:
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DIS, True);
        enable_collecting_distributing(port);
        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_COL, True);
        port->ntt = True;
        break;

    };
}

Bool lac_mux_check_conditions (LAC_STATE_MACH_T * this)
{
    register LAC_PORT_T *port = this->owner.port;
    if (!port->port_enabled || !port->lacp_enabled)
    {
        if (this->State == DISABLE)
            return False;
        else
            return lac_hop_2_state (this, DISABLE);
    }

    switch (this->State) {
    case BEGIN:
        return lac_hop_2_state (this, DETACHED);
    case DISABLE:
        if (port->port_enabled && port->lacp_enabled)
            return lac_hop_2_state (this, DETACHED);

    case DETACHED:
        if (port->selected && !port->standby)
            return lac_hop_2_state (this, WAITING);
        break;


    case WAITING:
        if (!port->wait_while)
            return lac_hop_2_state (this, READY_N);
        break;

    case READY_N:
        if (port->selected && !port->standby && sys_is_ready(port))
            return lac_hop_2_state (this, ATTACHED);

        if (!port->selected || port->standby)
            return lac_hop_2_state (this, DETACHED);
        break;


    case ATTACHED:
        if (port->selected && !port->standby && LAC_STATE_GET_BIT(port->partner.state, LAC_STATE_SYN))
            return lac_hop_2_state (this, RX_TX);

        if (!port->selected || port->standby)
            return lac_hop_2_state (this, DETACHED);
        break;

    case RX_TX:
        if (!port->selected || port->standby || !LAC_STATE_GET_BIT(port->partner.state, LAC_STATE_SYN))
            return lac_hop_2_state (this, ATTACHED);
        break;

    default:
        break;
    };
    return False;
}
