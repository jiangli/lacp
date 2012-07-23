#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "../lacp_out.h"

extern int max_port;
static lacp_sys_t *g_lacp_sys_inst = NULL;

lacp_sys_t *lacp_get_sys_inst (void)
{
    return g_lacp_sys_inst;
}

lacp_sys_t *lacp_sys_create ()
{
    LAC_MALLOC(g_lacp_sys_inst, lacp_sys_t, "Can't alloc for lac system");
    return g_lacp_sys_inst;
}

lacp_sys_t *lacp_sys_init ()
{
    lacp_sys_t *this;
    unsigned char sys_mac[6];

    this = lacp_get_sys_inst();
    if (this)
    {
        return this;
    }

    this = lacp_sys_create();
    this->ports = NULL;
    LAC_MALLOC (this->portmap, lacp_bitmap_t, "Can't alloc for ports");
    this->number_of_ports = 0;
    this->priority = Default_system_priority;
    lacp_ssp_get_port_mac(sys_mac);
    memcpy(this->mac, sys_mac, 6);
    this->short_timeout_time = Short_timeout_ticks;
    this->long_timeout_time = Long_timeout_ticks;
    this->admin_state = LACP_ENABLED;
    this->tx_hold_count = Max_tx_per_interval;
    this->slow_periodic_time = Slow_periodic_ticks;
    this->fast_periodic_time = Fast_periodic_ticks;
    this->aggregate_wait_time = Aggregate_wait_ticks;
    LAC_INIT_CRITICAL_PATH_PROTECTIO;

    return this;
}

static int
lacp_iterate_machines (lacp_sys_t * this,
                      int (*iter_callb) (lacp_state_mach_t *),
                      Bool exit_on_non_zero_ret)
{
    register lacp_state_mach_t *stater;
    register lacp_port_t *port;
    int iret, mret = 0;

    /* state machines per port */
    for (port = this->ports; port; port = port->next) {
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


int lacp_sys_update(lacp_sys_t *this, int reason)
{
    register Bool need_state_change;
    register int number_of_loops = 0;
    register int count = 0;

    need_state_change = False;
    if (reason != 2)
        printf("\r\n update reason:%d", reason);
    for (;;) {			  /* loop until not need changes */
        need_state_change = lacp_iterate_machines (this,
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
        number_of_loops += lacp_iterate_machines (this,
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



