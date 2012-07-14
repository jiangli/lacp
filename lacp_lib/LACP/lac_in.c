#include "lac_base.h"
#include "statmch.h"
#include "lac_port.h"
#include "lac_sys.h"
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

int lac_in_rx(int port_index, LACPDU_T * bpdu, int len)
{
    register LAC_PORT_T *port;
    register LAC_SYS_T *this;
    int iret;
    LAC_CRITICAL_PATH_START;
    lac_trace("\r\n port %d rx lacpdu", port_index);

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

    iret = lac_port_rx (port, bpdu, len);
//	  lac_set_port_reselect(port);


    lac_sys_update (this, LAC_SYS_UPDATE_READON_RX);
    LAC_CRITICAL_PATH_END;

    return iret;
}

int lac_port_set_cfg(UID_LAC_PORT_CFG_T * uid_cfg)
{
    register LAC_SYS_T *this;
    register LAC_PORT_T *port;
    int port_no;



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
            /* maybe delete from agg. so update selected first */
            if (port->agg_id && !uid_cfg->lacp_enabled)
            {
                port->ntt = True;
//                        LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_TMT, SHORT_TIMEOUT);
                LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_AGG, False);
                lac_sys_update (this, LAC_SYS_UPDATE_READON_PORT_CFG);

            } else {
                LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_AGG, True);
            }


            lac_set_port_reselect(port);
            port->lacp_enabled = uid_cfg->lacp_enabled;
            port->static_agg = True;
            port->agg_id = uid_cfg->agg_id;

            if (port->agg_id)
                lac_set_port_reselect(port);


        }
        else if (uid_cfg->field_mask & PT_CFG_COST)
        {
            lac_set_port_reselect(port);

        }



    }

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
    if (port->selected && !port->standby)
        uid_cfg->sel_state = True;
    else
        uid_cfg->sel_state = False;


    return 0;

}

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

int lac_in_link_change(int port_index, int link_status)
{
    LAC_SYS_T *this;
    LAC_PORT_T *p;

    lac_trace("\r\n %s.%d",  __FUNCTION__, __LINE__);
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

    lac_set_port_reselect(p);

    lac_sys_update(this, LAC_SYS_UPDATE_READON_LINK);

    LAC_CRITICAL_PATH_END;
    return 0;

}
int lac_in_enable_port(int port_index, Bool enable)
{
    lac_trace ("port p%02d => %sABLE", (int) port_index, enable ? "EN" : "DIS");

    lac_in_link_change(port_index, enable);

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

    }

    //lac_set_port_reselect(NULL);
    lac_sys_update (this, LAC_SYS_UPDATE_READON_SYS_CFG);

    LAC_CRITICAL_PATH_END;
    return 0;
}

