#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_rx.h"
#include "lacp_tx.h"
#include "lacp_mux.h"
#include "lacp_sel.h"
#include "../lacp_ssp.h"

lacp_port_t *lacp_port_create (lacp_sys_t * lacp_sys, int port_index)
{
    lacp_port_t *port;
    register unsigned int iii;

    lacp_trace("create portIndex:%d", port_index);

    /* check, if the port has just been added */
    for (port = lacp_sys->ports; port; port = port->next) {
        if (port->port_index == port_index) {
            return NULL;
        }
    }

    /* add port to system */
    LAC_NEW_IN_LIST (port, lacp_port_t, lacp_sys->ports, "port create");
    port->system = lacp_sys;
    port->port_index = port_index;
    LAC_STRDUP (port->port_name, lacp_ssp_get_port_name (port_index), "port_name");
    port->machines = NULL;

	lacp_port_get_actor_admin(port_index, &port->actor_admin);
	lacp_port_get_partner_admin(port_index, &port->partner_admin);
	
    port->master_port = port;
    port->selected = False;

    port->lacp_enabled = False;
    port->port_moved = False;
    port->agg_id = 0;
    port->speed = lacp_ssp_get_port_oper_speed(port->port_index);
    port->duplex = lacp_ssp_get_port_oper_duplex(port->port_index);
    port->rx_lacpdu_cnt = 0;
    port->tx_lacpdu_cnt = 0;

    iii = 0;
    port->timers[iii++] = &port->current_while;
    port->timers[iii++] = &port->periodic_timer;
    port->timers[iii++] = &port->wait_while;

    /* create and bind port state machines */
    LACP_STATE_MACH_IN_LIST (tx);
    LACP_STATE_MACH_IN_LIST (mux);
    LACP_STATE_MACH_IN_LIST (sel);
    LACP_STATE_MACH_IN_LIST (rx);

    port->mux->debug = 1;
    port->sel->debug = 1;
    port->rx->debug = 1;
    port->tx->debug = 1;

    /* reset timers */
    port->current_while =
        port->periodic_timer =
            port->wait_while = 0;

    return port;
}

void
lacp_port_init (lacp_port_t * this, lacp_sys_t * LACm)
{
}

void
lacp_port_delete (lacp_port_t * this)
{
    lacp_sys_t *lacp_sys;
    register lacp_port_t *prev;
    register lacp_port_t *tmp;
    register lacp_state_mach_t *stater;
    register void *pv;

    lacp_sys = this->system;

    LAC_FREE (this->port_name, "port name");
    for (stater = this->machines; stater;) {
        pv = (void *) stater->next;
        lacp_state_mach_delete (stater);
        stater = (lacp_state_mach_t *) pv;
    }

    prev = NULL;
    for (tmp = lacp_sys->ports; tmp; tmp = tmp->next) {
        if (tmp->port_index == this->port_index) {
            if (prev) {
                prev->next = this->next;
            } else {
                lacp_sys->ports = this->next;
            }
            LAC_FREE (this, "LAC instance");
            break;
        }
        prev = tmp;
    }
}

unsigned int lacp_port_rx_lacpdu (lacp_port_t * this, lacp_pdu_t * bpdu, size_t len)
{
    int ret = 0;


    ret = lacp_rxm_rx_lacpdu (this, bpdu, len);
    if (ret)
    {
        return ret;
    }

    return 0;
}

int lacp_port_set_reselect(lacp_port_t *port)
{
    lacp_port_t  *p;
    lacp_sys_t *lacp_sys;
    lacp_sys = lacp_get_sys_inst();
    if (!port)
    {
        for (p = lacp_sys->ports; p; p=p->next)
        {
            lacp_trace("\r\n<%s.%d> port:%d, selected:%d",__FUNCTION__, __LINE__,  p->port_index, False);
            p->selected = False;
        }
        return 0;
    }
    else
    {
        if (port->agg_id)
        {
            for (p = port->system->ports; p; p=p->next)
            {
                if (p->agg_id == port->agg_id)
                {
                    lacp_trace("\r\n<%s.%d> port:%d, selected:%d",__FUNCTION__, __LINE__,  p->port_index, False);
                    p->selected = False;
                }
            }
        }
        else
        {
            //TODO::
        }
    }
    return 0;
}


#if 0
#ifdef LAC_DBG
unsigned int
LAC_LAC_PORT_Trace_state_machine (lacp_port_t * this, char *mach_name, unsigned int enadis,
                                  unsigned int id)
{
    register struct lacp_state_mach_s *stater;

    for (stater = this->machines; stater; stater = stater->next) {
        if (!strcmp (mach_name, "all") || !strcmp (mach_name, stater->name)) {
            /* if (stater->debug != enadis) */
            {
                lacp_trace ("port %s on %s trace %-8s (was %s) now %s",
                           this->port_name, this->owner->name,
                           stater->name,
                           stater->debug ? " enabled" : "disabled",
                           enadis ? " enabled" : "disabled");
            }
            stater->debug = enadis;
        }
    }

    return 0;
}
#endif
#endif

