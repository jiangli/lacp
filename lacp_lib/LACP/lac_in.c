#include "lac_base.h"
#include "statmch.h"
#include "lac_port.h"
#include "lac_sys.h"
#include "uid_lac.h"
#include "lac_in.h"
#include "../lac_out.h"

int max_port = 4;

LAC_PORT_T *lac_port_find (LAC_SYS_T * this, int port_index)
{
    register LAC_PORT_T *port;

    for (port = this->ports; port; port = port->next)
        if (port_index == port->port_index) {
            return port;
        }

    return NULL;
}

int lac_rx_lacpdu(int port_index, LACPDU_T * bpdu, int len)
{
    register LAC_PORT_T *port;
    register LAC_SYS_T *this;
    int iret;
    LAC_CRITICAL_PATH_START;
    lac_trace(" port %d rx lacpdu", port_index);

    this = lac_get_sys_inst();
    if (!this) {
        lac_trace("the instance had not yet been created");

        LAC_CRITICAL_PATH_END;
        return M_LAC_NOT_CREATED;
    }

    port = lac_port_find (this, port_index);
    if (!port) {			/* port is absent in the stpm :( */
        lac_trace ("RX lacpdu port=%d port is absent :(", (int) port_index);
        LAC_CRITICAL_PATH_END;
        return M_RSTP_PORT_IS_ABSENT;
    }

    if (!port->port_enabled) { /* port link change indication will come later :( */
        lac_trace ("disable port receive lacpdu port=%d :(", (int) port_index);
        LAC_CRITICAL_PATH_END;
        return M_RSTP_NOT_ENABLE;
    }

    if (LAC_ENABLED != this->admin_state) {	/* the stpm had not yet been enabled :( */
        LAC_CRITICAL_PATH_END;
        return M_RSTP_NOT_ENABLE;
    }

    iret = lac_port_rx_lacpdu (port, bpdu, len);
    if (iret)
    {
            lac_trace("rx process error");
            return -1;
            
    }
    
    lac_sys_update (this, LAC_SYS_UPDATE_READON_RX);
    LAC_CRITICAL_PATH_END;

    return iret;
}

int lac_port_set_cfg(UID_LAC_PORT_CFG_T * uid_cfg)
{
    register LAC_SYS_T *this;
    register LAC_PORT_T *port;
    int port_no;
    Bool update_fsm = True;

    LAC_CRITICAL_PATH_START;
    this = lac_get_sys_inst();

    for (port_no = 0; port_no < max_port; port_no++) {
        if (!BitmapGetBit (&uid_cfg->port_bmp, port_no))
            continue;

        port = lac_port_find (this, port_no);
        if (!port) {		  /* port is absent in the stpm :( */
            continue;
        }

        if (uid_cfg->field_mask & PT_CFG_STATE)
        {
            if (uid_cfg->lacp_enabled)
            {
                /* add to agg */
                LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_AGG, True);
                port->static_agg = True;
                port->agg_id = uid_cfg->agg_id;
                lac_port_set_reselect(port);
            } else {            /* delete from agg. */
                /* notify to partner*/
                port->ntt = True;
                LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_AGG, False);
                lac_sys_update (this, LAC_SYS_UPDATE_READON_PORT_CFG);

                /* set agg member reselect */
                lac_port_set_reselect(port);
                port->static_agg = False;
                port->agg_id = 0;
            }
            port->lacp_enabled = uid_cfg->lacp_enabled;
        }
        else if (uid_cfg->field_mask & PT_CFG_COST)
        {
            lac_port_set_reselect(port);
        }

        else if (uid_cfg->field_mask & PT_CFG_STAT)
        {
                port->rx_lacpdu_cnt = 0;
                port->tx_lacpdu_cnt = 0;
                update_fsm = False;
        }

    }

    if (update_fsm)
            lac_sys_update (this, LAC_SYS_UPDATE_READON_PORT_CFG);

    LAC_CRITICAL_PATH_END;
    return 0;
}

int lac_port_get_cfg(int port_index, UID_LAC_PORT_CFG_T * uid_cfg)
{
    register LAC_SYS_T *this;
    register LAC_PORT_T *port;

    this = lac_get_sys_inst();


    port = lac_port_find (this, port_index);
    if (!port) {		  /* port is absent in the stpm :( */
        return -1;
    }

    uid_cfg->lacp_enabled = port->lacp_enabled;


    return 0;

}

int _lac_copy_port_state(UID_LAC_PORT_STATE_T * uid_cfg, LAC_PORT_T *port)
{
        uid_cfg->port_index = port->port_index;
        uid_cfg->agg_id = port->agg_id;
        uid_cfg->rx_cnt = port->rx_lacpdu_cnt;
        uid_cfg->tx_cnt = port->tx_lacpdu_cnt;
        uid_cfg->master_port = port->aport->port_index;

        if (port->selected && !port->standby)                                                                    
        {

                uid_cfg->sel_state = True;                     
        }                                                  
        else                                                                                                     
                uid_cfg->sel_state = False;                                                                      
        
        memcpy(&uid_cfg->actor, &port->actor, sizeof(LAC_PORT_INFO));
        memcpy(&uid_cfg->partner, &port->partner, sizeof(LAC_PORT_INFO)); 
        return 0;
}
int lac_agg_get_port_state(int agg_id, UID_LAC_PORT_STATE_T * uid_cfg, int *master_index)
{
    register LAC_SYS_T *lac_sys;
    register LAC_PORT_T *port;
    int cnt = 0;

    lac_sys = lac_get_sys_inst();
    for (port = lac_sys->ports; port; port = port->next)
    {
            if (port->agg_id != agg_id)
            {
                    continue;
            }
            if (cnt >= 8)
            {
                    break;
            }

            _lac_copy_port_state(&uid_cfg[cnt], port);      
            if (port->selected && !port->standby && (port->aport == port))
            {
                    *master_index = cnt;                    
            }
            cnt++;
    }

    return 0;
 }
int lac_port_get_port_state(int port_index, UID_LAC_PORT_STATE_T * uid_cfg)
{
    register LAC_SYS_T *lac_sys;
    register LAC_PORT_T *port;
    int cnt = 0;

    lac_sys = lac_get_sys_inst();
    port = lac_port_find (lac_sys, port_index);
    if (!port) {		  /* port is absent in the stpm :( */
        return -1;
    }

    _lac_copy_port_state(uid_cfg, port);      

    return 0;
 }

#if 0
int lac_agg_get_master_port(int agg_id, UID_LAC_PORT_CFG_T * uid_cfg)
{
    register LAC_SYS_T *lac_sys;
    register LAC_PORT_T *port;

    lac_sys = lac_get_sys_inst();
    for (port = lac_sys->ports; port; port = port->next)
    {
            if (port->agg_id != agg_id)
            {
                    continue;
            }
            
            uid_cfg->key = port->aport->key;
            uid_cfg->agg_id = port->aport->agg_id;
            uid_cfg->lacp_enabled = port->aport->lacp_enabled;
            BitmapSetBit(uid_cfg->port_bmp, port->aport->port_index);
            uid_cfg->sel_state = !port->aport->standby;
            memcpy(&uid_cfg->partner, port->aport->partner, sizeof(LAC_PORT_INFO);)
            memcpy(&uid_cfg->partner, port->aport->partner, sizeof(LAC_PORT_INFO));
            
    }
    return 0;
}
#endif

int lac_port_get_dbg_cfg(int port_index, LAC_PORT_T * port)
{
    register LAC_SYS_T *this;
    register LAC_PORT_T *p;


    lac_trace("\r\n %s.%d",  __FUNCTION__, __LINE__);

    this = lac_get_sys_inst();

    p = lac_port_find (this, port_index);
    if (!port) {		  /* port is absent in the stpm :( */
        return 1;
    }

    memcpy(port, p, sizeof(LAC_PORT_T));
    return 0;

}

void
lac_one_second ()
{
    LAC_SYS_T *this = lac_get_sys_inst();
    register LAC_PORT_T *port;
    register int iii;

    if (LAC_ENABLED != this->admin_state)
        return;

    LAC_CRITICAL_PATH_START;
    for (port = this->ports; port; port = port->next) {
        for (iii = 0; iii < TIMERS_NUMBER; iii++) {
//            lac_trace("\r\n*******************88^^^port:%d iii:%d,value:%d\r\n", port->port_index, iii, *(port->timers[iii]));
            if (*(port->timers[iii]) > 0) {
                (*port->timers[iii])--;
            }
        }
        port->hold_count = 0;
    }

    lac_sys_update (this, LAC_SYS_UPDATE_READON_TIMER);

    LAC_CRITICAL_PATH_END;
}
int lac_in_create_port()
{
    //TODO:: create port
    return 0;

}
int lac_in_remove_port()
{
//TODO:: remove port
    return 0;

}

int lac_port_link_change(int port_index, int link_status)
{
    LAC_SYS_T *this;
    LAC_PORT_T *p;

    lac_trace("<%s.%d>",  __FUNCTION__, __LINE__);

    LAC_CRITICAL_PATH_START;
    this = lac_get_sys_inst();
    p = lac_port_find (this, port_index);
    if (!p) {		/* port is absent in the stpm :( */
        return -1;
    }
    if (p->port_enabled == link_status) {	/* nothing to do :) */
        return;
    }

    if (link_status)
    {
        p->port_enabled = True;
    }
    else
    {
        p->port_enabled = False;
    }

    lac_port_set_reselect(p);

    lac_sys_update(this, LAC_SYS_UPDATE_READON_LINK);

    LAC_CRITICAL_PATH_END;
    return 0;

}

int lac_in_enable_port(int port_index, Bool enable)
{
    lac_trace ("port p%02d => %sABLE", (int) port_index, enable ? "EN" : "DIS");

    lac_port_link_change(port_index, enable);

    return 0;

}
int     lac_update_port_info()
{
    register LAC_SYS_T *lac_sys;
    register LAC_PORT_T *port;

    lac_sys = lac_get_sys_inst();
    for (port = lac_sys->ports; port; port = port->next)
    {
            port->actor.system_priority = lac_sys->priority;
            LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_TMT, lac_sys->lacp_timeout);
            port->ntt = True;
    }

    return 0;
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
            this->priority = uid_cfg->priority;
    }

    else if (uid_cfg->field_mask & BR_CFG_LONG_PERIOD)
    {
            this->slow_periodic_time = uid_cfg->long_period;
    }
    else if (uid_cfg->field_mask & BR_CFG_SHORT_PERIOD)
    {
            this->fast_periodic_time = uid_cfg->short_period;
    }
    else if (uid_cfg->field_mask & BR_CFG_PERIOD)
    {
            this->lacp_timeout = uid_cfg->period;
    }

    lac_update_port_info();
    lac_sys_update (this, LAC_SYS_UPDATE_READON_SYS_CFG);

    LAC_CRITICAL_PATH_END;
    return 0;
}
int lac_sys_get_cfg(UID_LAC_CFG_T * uid_cfg)
{
    LAC_SYS_T *this = lac_get_sys_inst();
    int port_loop = 0;
    LAC_PORT_T *p;
    
    uid_cfg->number_of_ports = max_port;
    for (p = this->ports; p; p = p->next)
    {
            BitmapSetBit(&uid_cfg->ports, p->port_index);
    }
    uid_cfg->priority = this->priority;
    memcpy(uid_cfg->sys_mac, this->id, 6);
    uid_cfg->short_period = this->fast_periodic_time;
    uid_cfg->long_period = this->slow_periodic_time;
    return 0;
}    

