#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_default.h"
#include "lac_in.h"

extern int max_port;
static LAC_SYS_T *g_lac_sys_inst = NULL;

LAC_SYS_T *lac_get_sys_inst (void)
{
    return g_lac_sys_inst;
}
LAC_SYS_T *lac_sys_create ()
{
    LAC_MALLOC(g_lac_sys_inst, LAC_SYS_T, "Can't alloc for lac system");
    return g_lac_sys_inst;
}

LAC_SYS_T *lac_sys_init ()
{
    LAC_SYS_T *this;
    unsigned char sys_mac[6];

    this = lac_get_sys_inst();
    if (this)
    {
        return this;
    }

    this = lac_sys_create();
    this->ports = NULL;
    LAC_MALLOC (this->portmap, BITMAP_T, "Can't alloc for ports");
    this->number_of_ports = 0;
    this->priority = Default_system_priority;
    LAC_OUT_get_port_mac(sys_mac);
    memcpy(this->id, sys_mac, 6);
    this->short_timeout_time = 3;
    this->long_timeout_time = 90;
    this->admin_state = LACP_ENABLED;
    this->tx_hold_count = Max_tx_per_interval;
    this->slow_periodic_time = 30;
    this->fast_periodic_time = 1;

    LAC_INIT_CRITICAL_PATH_PROTECTIO;

    return this;
}
LAC_PORT_T *lac_port_find (LAC_SYS_T * this, int port_index)
{
    register LAC_PORT_T *port;

    for (port = this->ports; port; port = port->next)
        if (port_index == port->port_index) {
            return port;
        }

    return NULL;
}

int lac_sys_set_cfg(UID_LAC_CFG_T * uid_cfg)
{
    LAC_SYS_T *this = lac_get_sys_inst();
    int port_loop = 0;
    LAC_PORT_T *p;

    LAC_CRITICAL_PATH_START;
    if (uid_cfg->field_mask & BR_CFG_PBMP_ADD)
    {
        for (port_loop = 0; port_loop < max_port; port_loop++)
            if (BitmapGetBit(&uid_cfg->ports, port_loop))
                lac_port_create(this, port_loop);
    }

    if (uid_cfg->field_mask & BR_CFG_PBMP_DEL)
    {
        for (port_loop = 0; port_loop < max_port; port_loop++)
            if (BitmapGetBit(&uid_cfg->ports, port_loop))
            {
                p = lac_port_find(this, port_loop);
                lac_port_delete(p);
            }
    }
    else if (uid_cfg->field_mask & BR_CFG_PRIO)
    {

    }

    //lac_set_port_reselect(NULL);
    lac_sys_update (this, LAC_SYS_UPDATE_READON_SYS_CFG);

    LAC_CRITICAL_PATH_END;
    return 0;
}
static int
lac_iterate_machines (LAC_SYS_T * this,
                      int (*iter_callb) (LAC_STATE_MACH_T *),
                      Bool exit_on_non_zero_ret)
{
    register LAC_STATE_MACH_T *stater;
    register LAC_PORT_T *port;
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


int lac_sys_update(LAC_SYS_T *this, int reason)
{
    register Bool need_state_change;
    register int number_of_loops = 0;
    register int count = 0;

    need_state_change = False;
    if (reason != 2)
        printf("\r\n update reason:%d", reason);
    for (;;) {			  /* loop until not need changes */
        need_state_change = lac_iterate_machines (this,
                            lac_check_condition,
                            True);
//	  printf(" need_state_change:%d", need_state_change);
        if (!need_state_change)
        {

            return number_of_loops;
        }

        number_of_loops++;
        /* here we know, that at least one stater must be
         updated (it has changed state) */
        number_of_loops += lac_iterate_machines (this,
                           lac_change_state, False);

        count++;
        if (count % 64 == 0)
        {
            //taskDelay(10);
        }

//	  printf("\r\n number_of_loops:%d", number_of_loops);
    }

    return number_of_loops;
}



