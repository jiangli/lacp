#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "trunk_ssp.h"

static lacp_sys_t *g_lacp_sys_inst = NULL;

lacp_sys_t *lacp_get_sys_inst (void)
{
    return g_lacp_sys_inst;
}

lacp_sys_t *lacp_sys_create ()
{
    LACP_MALLOC(g_lacp_sys_inst, lacp_sys_t, "Can't alloc for lac system");
    return g_lacp_sys_inst;
}

lacp_sys_t *lacp_sys_init ()
{
    lacp_sys_t *sys;
    unsigned char sys_mac[6];

    sys = lacp_get_sys_inst();
    if (sys)
    {
        return sys;
    }

    sys = lacp_sys_create();
    sys->ports = NULL;
    LACP_MALLOC (sys->portmap, lacp_bitmap_t, "Can't alloc for ports");
    sys->number_of_ports = 0;
    sys->priority = Default_system_priority;
    trunk_ssp_get_mac(sys_mac);
    memcpy(sys->mac, sys_mac, 6);
    sys->short_timeout_time = Short_timeout_ticks;
    sys->long_timeout_time = Long_timeout_ticks;
    sys->admin_state = LACP_ENABLED;
    sys->tx_hold_count = Max_tx_per_interval;
    sys->slow_periodic_time = Slow_periodic_ticks;
    sys->fast_periodic_time = Fast_periodic_ticks;
    sys->aggregate_wait_time = Aggregate_wait_ticks;
    LACP_INIT_CRITICAL_PATH_PROTECTIO

    return sys;
}

static uint32_t
lacp_iterate_machines (lacp_sys_t * sys,
                       uint32_t (*iter_callb) (lacp_state_mach_t *),
                       Bool exit_on_non_zero_ret)
{
    register lacp_state_mach_t *stater;
    register lacp_port_t *port;
    uint32_t iret, mret = 0;

    /* state machines per port */
    for (port = sys->ports; port; port = port->next) {
        for (stater = port->machines; stater; stater = stater->next) {
            iret = (*iter_callb) (stater);
            if (exit_on_non_zero_ret && iret)
                return iret;
            else
                mret += iret;
        }
    }

    return mret;
}


uint32_t lacp_sys_update(lacp_sys_t *sys, uint32_t reason)
{
    register Bool need_state_change;
    register uint32_t number_of_loops = 0;
    register uint32_t count = 0;

    need_state_change = False;

    for (;;) {			  /* loop until not need changes */
        need_state_change = lacp_iterate_machines (sys,
                            lacp_check_condition,
                            True);
//	  printf(" need_state_change:%d", need_state_change);
        if (!need_state_change)
        {

            return number_of_loops;
        }

        number_of_loops++;
        /* here we know, that at least one stater must be
         updated (it has changed state) */
        number_of_loops += lacp_iterate_machines (sys,
                           lacp_change_state, False);

        count++;
        if (count % 64 == 0)
        {
            //taskDelay(10);
        }

//	  printf("\r\n number_of_loops:%d", number_of_loops);
    }

    return number_of_loops;
}



