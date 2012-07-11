#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_pdu.h"
#include "lac_in.h"

int max_port = 30;

int lac_in_rx(int port_index, LACPDU_T * bpdu, int len)
{
    register LAC_PORT_T *port;
    register LAC_SYS_T *this;
    int iret;
    LAC_CRITICAL_PATH_START;
    printf("\r\n %s.%d ********* ",  __FUNCTION__, __LINE__);
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
            port->lacp_enabled = uid_cfg->lacp_enabled;
        }
//        port->reselect = True;
        lac_set_port_reselect(port);
        
    }

    lac_sys_update (this, LAC_SYS_UPDATE_READON_PORT_CFG);

    LAC_CRITICAL_PATH_END;
    return 0;
}

int lac_port_get_cfg(UID_LAC_PORT_CFG_T * uid_cfg)
{
    register LAC_SYS_T *this;
    register LAC_PORT_T *port;
    int port_no;

    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);

    this = lac_get_sys_inst();

    for (port_no = 0; port_no < max_port; port_no++) {
        if (!BitmapGetBit (&uid_cfg->port_bmp, port_no))
            continue;

        port = lac_port_find (this, port_no);
        if (!port) {		  /* port is absent in the stpm :( */
            continue;
        }

        uid_cfg->lacp_enabled = port->lacp_enabled;
    }
}

int lac_port_get_dbg_cfg(int port_index, LAC_PORT_T * port)
{
    register LAC_SYS_T *this;
    register LAC_PORT_T *p;
    int port_no;

    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);

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
//            printf("\r\n*******************88^^^port:%d iii:%d,value:%d\r\n", port->port_index, iii, *(port->timers[iii]));
            if (*(port->timers[iii]) > 0) {
                (*port->timers[iii])--;
            }
        }
    }

    lac_sys_update (this, LAC_SYS_UPDATE_READON_TIMER);

    LAC_CRITICAL_PATH_END;
}
int lac_in_create_port()
{
}
int lac_in_remove_port()
{
}
int lac_in_init()
{
}
static void
_stp_in_enable_port_on_stpm (LAC_SYS_T * stpm, int port_index, Bool enable)
{
    register LAC_PORT_T *port;
    printf("port_index:%x.%d", stpm, port_index);

    port = lac_port_find (stpm, port_index);
    if (!port)
        return;
    if (port->port_enabled == enable) {	/* nothing to do :) */
        return;
    }

    port->port_enabled = enable;

    if (enable) {			/* clear port statistics */
        port->rx_lacpdu_cnt = 0;

        port->tx_lacpdu_cnt = 0;
    }

}

int lac_in_enable_port(int port_index, Bool enable)
{

    register LAC_SYS_T *stpm = lac_get_sys_inst();

    LAC_CRITICAL_PATH_START;

    lac_trace ("port p%02d => %sABLE", (int) port_index, enable ? "EN" : "DIS");

    _stp_in_enable_port_on_stpm (stpm, port_index, enable);
    LAC_CRITICAL_PATH_END;
}
int lac_in_port_disable()
{
}

int lac_in_link_change(int port_index, int link_status)
{
    LAC_SYS_T *this;
    LAC_PORT_T *p;

    printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
    LAC_CRITICAL_PATH_START;
    this = lac_get_sys_inst();
    p = lac_port_find (this, port_index);
    if (!p) {		/* port is absent in the stpm :( */
        return -1;
    }

    if (link_status)
    {
        p->port_enabled = True;
    }
    else
    {
        p->port_enabled = False;
    }

//    p->reselect = True;
    lac_set_port_reselect(p);
    
    lac_sys_update(this, LAC_SYS_UPDATE_READON_LINK);

    LAC_CRITICAL_PATH_END;
}

int lac_in_lacp_mngt()
{
}

