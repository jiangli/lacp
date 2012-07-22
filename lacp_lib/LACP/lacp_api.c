
#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_ssp.h"
#include "lacp_util.h"

uint32_t max_port = 4;

uint32_t lacp_port_get_actor_init(uint32_t port_index, lacp_port_info_t  *admin)
{
    lacp_mac_t sys_mac;

    lacp_ssp_get_mac(sys_mac);

    admin->port_no = port_index + 1;
    memcpy(admin->system_mac, sys_mac, sizeof(lacp_mac_t));
    admin->key = Default_key;
    admin->system_priority = Default_system_priority;
    admin->port_priority = Default_port_priority;

    LACP_STATE_SET_BIT(admin->state, LACP_STATE_ACT, Default_lacp_activity);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_TMT, Default_lacp_timeout);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_AGG, Default_aggregation);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_SYN, False);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_DEF, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_EXP, False);

    return 0;
}


uint32_t lacp_port_get_partner_init(uint32_t port_index, lacp_port_info_t  *admin)
{
    lacp_mac_t partner_def_mac = {0};

    memset(admin, 0, sizeof(lacp_port_info_t));

    admin->port_priority		 = Default_port_priority;
    admin->port_no				 = port_index + 1; /* TODO:: */
    admin->system_priority		 = Default_system_priority;
    memcpy(admin->system_mac, partner_def_mac, 6);
    admin->key					 = port_index + 1;

    LACP_STATE_SET_BIT(admin->state, LACP_STATE_ACT, False); /* Passive	  */
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_TMT, False); /* Long timeout */
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_AGG, False); /* Individual   */
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_SYN, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_COL, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_DIS, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_DEF, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_EXP, False);

    return 0;
}

lacp_port_t *_lacp_port_find (uint32_t port_index)
{
    lacp_sys_t * sys = lacp_get_sys_inst();
    register lacp_port_t *port;

    for (port = sys->ports; port; port = port->next)
        if (port_index == port->port_index) {
            return port;
        }

    return NULL;
}


uint32_t lacp_port_set_cfg(lacp_port_cfg_t * uid_cfg)
{
    register lacp_sys_t *sys;
    register lacp_port_t *port;
    uint32_t port_index;

    LACP_CRITICAL_PATH_START;
    sys = lacp_get_sys_inst();

    for (port_index = 0; port_index < max_port; port_index++) {
        if (!lacp_bitmap_get_bit (&uid_cfg->port_bmp, port_index))
            continue;

        port = _lacp_port_find (port_index);
        if (!port) {		  /* port is absent in the stpm :( */
            //ERR_LOG(port_index, 0, 0, 0);
            continue;
        }

        if (uid_cfg->field_mask & PT_CFG_STATE)
        {
            if (uid_cfg->lacp_enabled)
            {
                /* add to agg */
                LACP_STATE_SET_BIT(port->actor_admin.state, LACP_STATE_AGG, True);
                LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_AGG, True);
                port->agg_id = uid_cfg->agg_id;
                lacp_port_set_reselect(port);
            } else {			/* delete from agg. */
                /* notify to partner*/
                port->ntt = True;
                LACP_STATE_SET_BIT(port->actor_admin.state, LACP_STATE_AGG, False);
                LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_AGG, False);
                lacp_sys_update (sys, LACP_SYS_UPDATE_READON_PORT_CFG);

                /* set agg member reselect */
                lacp_port_set_reselect(port);
                port->agg_id = 0;
            }
            port->lacp_enabled = uid_cfg->lacp_enabled;
        }
        if (uid_cfg->field_mask & PT_CFG_COST)
        {
            lacp_port_set_reselect(port);
        }

        if (uid_cfg->field_mask & PT_CFG_STAT)
        {
            port->rx_lacpdu_cnt = 0;
            port->tx_lacpdu_cnt = 0;
        }
        if (uid_cfg->field_mask & PT_CFG_PRIO)
        {
            port->actor_admin.port_priority = uid_cfg->port_priority;
            port->actor.port_priority = uid_cfg->port_priority;
            lacp_port_set_reselect(port);
        }
    }

    lacp_sys_update (sys, LACP_SYS_UPDATE_READON_PORT_CFG);

    LACP_CRITICAL_PATH_END;
    return 0;
}

uint32_t _lacp_copy_port_state(lacp_port_state_t * state_para, lacp_port_t *port)
{
    state_para->valid = True;
    // lacp_ssp_change_to_slot_port(port->port_index, &state_para->slot, &state_para->port);
    state_para->port_index = port->port_index;
    state_para->agg_id = port->agg_id;
    state_para->rx_cnt = port->rx_lacpdu_cnt;
    state_para->tx_cnt = port->tx_lacpdu_cnt;
    state_para->master_port = port->master_port->port_index;

    if (port->selected && !port->standby)
    {
        state_para->sel_state = True;
    }
    else
    {
        state_para->sel_state = False;
    }

    memcpy(&state_para->actor, &port->actor, sizeof(lacp_port_info_t));
    memcpy(&state_para->partner, &port->partner, sizeof(lacp_port_info_t));
    return 0;
}

uint32_t lacp_agg_get_port_state(uint32_t agg_id, lacp_port_state_t * state_para, uint32_t *master_index)
{
    register lacp_sys_t *lacp_sys;
    register lacp_port_t *port;
    uint32_t cnt = 0;

    lacp_sys = lacp_get_sys_inst();
    for (port = lacp_sys->ports; port; port = port->next)
    {
        if (port->agg_id != agg_id)
        {
            continue;
        }
        if (cnt >= 8)
        {
            break;
        }

        _lacp_copy_port_state(&state_para[cnt], port);
        if (port->selected && !port->standby && (port->master_port == port))
        {
            *master_index = cnt;
        }
        cnt++;
    }

    return 0;
}

uint32_t lacp_port_get_port_state(uint32_t port_index, lacp_port_state_t * state_para)
{
    register lacp_sys_t *lacp_sys;
    register lacp_port_t *port;

    lacp_sys = lacp_get_sys_inst();
    port = _lacp_port_find (port_index);
    if (!port) {		  /* port is absent in the stpm :( */
        ERR_LOG(port_index, 0, 0, 0);
        return M_LACP_NOT_FOUND;
    }

    _lacp_copy_port_state(state_para, port);

    return 0;
}

uint32_t lacp_port_get_dbg_cfg(uint32_t port_index, lacp_port_t * port)
{
    register lacp_sys_t *sys;
    register lacp_port_t *p;

    sys = lacp_get_sys_inst();

    p = _lacp_port_find (port_index);
    if (!p) {		  /* port is absent in the stpm :( */
        ERR_LOG(port_index, 0, 0, 0);
        return M_LACP_NOT_FOUND;

    }

    memcpy(port, p, sizeof(lacp_port_t));
    return 0;

}

void lacp_one_second ()
{
    lacp_sys_t *sys = lacp_get_sys_inst();
    register lacp_port_t *port;
    register uint32_t timer_index;

    if (LACP_ENABLED != sys->admin_state)
        return;

    LACP_CRITICAL_PATH_START;
    for (port = sys->ports; port; port = port->next) {
        for (timer_index = 0; timer_index < TIMERS_NUMBER; timer_index++) {
            if (*(port->timers[timer_index]) > 0) {
                (*port->timers[timer_index])--;
            }
        }
        port->hold_count = 0;
    }

    lacp_sys_update (sys, LACP_SYS_UPDATE_READON_TIMER);

    LACP_CRITICAL_PATH_END;
}

uint32_t lacp_create_ports(lacp_bitmap_t *ports)
{
    uint32_t port_index;
    lacp_sys_t *lacp_sys = lacp_get_sys_inst();

    for (port_index = 0; port_index < max_port; port_index++)
    {
        if (lacp_bitmap_get_bit(ports, port_index))
        {
            if (lacp_port_create(lacp_sys, port_index))
            {
                lacp_sys->number_of_ports ++;
                lacp_bitmap_set_bit(lacp_sys->portmap, port_index);
            }
            else
            {
                ERR_LOG(port_index, 0, 0, 0);
                continue;
            }
        }
    }

    lacp_sys_update (lacp_sys, LACP_SYS_UPDATE_READON_PORT_CREATE);
    return 0;

}
uint32_t lacp_remove_ports(lacp_bitmap_t *ports)
{
    uint32_t port_index;
    lacp_port_t *port;
    lacp_sys_t *lacp_sys = lacp_get_sys_inst();

    for (port_index = 0; port_index < max_port; port_index++)
    {
        if (lacp_bitmap_get_bit(ports, port_index))
        {
            port = _lacp_port_find(port_index);
            if (!port)
                continue;

            lacp_port_delete(port);
            lacp_sys->number_of_ports --;
            lacp_bitmap_clear_bit(lacp_sys->portmap, port_index);
        }
    }
    return 0;
}

uint32_t lacp_port_link_change(uint32_t port_index, uint32_t link_status)
{
    lacp_sys_t *sys = lacp_get_sys_inst();
    lacp_port_t *p;

    lacp_trace ("port p%02d => %sABLE", (uint32_t) port_index, link_status ? "EN" : "DIS");

    LACP_CRITICAL_PATH_START;

    p = _lacp_port_find (port_index);
    if (!p) {		/* port is absent in the stpm :( */
        return M_LACP_NOT_ENABLE;
    }

    if (p->port_enabled == link_status) {	/* nothing to do :) */
        LACP_CRITICAL_PATH_END;
        return 0;
    }

    if (link_status)
    {
        p->port_enabled = True;
    }
    else
    {
        p->port_enabled = False;
    }

    p->speed = lacp_ssp_get_port_oper_speed(p->port_index);
    p->duplex = lacp_ssp_get_port_oper_duplex(p->port_index);
    if (!p->duplex)
    {
        LACP_STATE_SET_BIT(p->actor.state, LACP_STATE_AGG, False);
    }

    lacp_port_set_reselect(p);

    lacp_sys_update(sys, LACP_SYS_UPDATE_READON_LINK);

    LACP_CRITICAL_PATH_END;
    return 0;

}

static uint32_t _lacp_port_update_info()
{
    register lacp_sys_t * lacp_sys = lacp_get_sys_inst();
    register lacp_port_t *port;

    for (port = lacp_sys->ports; port; port = port->next)
    {
        port->actor.system_priority = lacp_sys->priority;
        LACP_STATE_SET_BIT(port->actor.state, LACP_STATE_TMT, lacp_sys->lacp_timeout);
        port->ntt = True;
    }

    return 0;
}

uint32_t lacp_sys_set_cfg(lacp_sys_cfg_t * uid_cfg)
{
    lacp_sys_t *sys = lacp_get_sys_inst();
    uint32_t port_loop = 0;
    lacp_port_t *p;

    LACP_CRITICAL_PATH_START;
    if (uid_cfg->field_mask & BR_CFG_PBMP_ADD)
    {
        for (port_loop = 0; port_loop < max_port; port_loop++)
            if (lacp_bitmap_get_bit(&uid_cfg->ports, port_loop))
                lacp_port_create(sys, port_loop);
    }

    if (uid_cfg->field_mask & BR_CFG_PBMP_DEL)
    {
        for (port_loop = 0; port_loop < max_port; port_loop++)
            if (lacp_bitmap_get_bit(&uid_cfg->ports, port_loop))
            {
                p = _lacp_port_find( port_loop);
                lacp_port_delete(p);
            }
    }
    else if (uid_cfg->field_mask & BR_CFG_PRIO)
    {
        sys->priority = uid_cfg->priority;
    }

    else if (uid_cfg->field_mask & BR_CFG_LONG_PERIOD)
    {
        sys->slow_periodic_time = uid_cfg->long_period;
    }
    else if (uid_cfg->field_mask & BR_CFG_SHORT_PERIOD)
    {
        sys->fast_periodic_time = uid_cfg->short_period;
    }
    else if (uid_cfg->field_mask & BR_CFG_PERIOD)
    {
        sys->lacp_timeout = uid_cfg->period;
    }

    _lacp_port_update_info();
    lacp_sys_update (sys, LACP_SYS_UPDATE_READON_SYS_CFG);

    LACP_CRITICAL_PATH_END;
    return 0;
}

uint32_t lacp_sys_get_cfg(lacp_sys_cfg_t * uid_cfg)
{
    lacp_sys_t *sys = lacp_get_sys_inst();
    lacp_port_t *port;

    uid_cfg->number_of_ports = max_port;
    for (port = sys->ports; port; port = port->next)
    {
        lacp_bitmap_set_bit(&uid_cfg->ports, port->port_index);
    }

    uid_cfg->priority = sys->priority;
    memcpy(uid_cfg->sys_mac, sys->mac, 6);
    uid_cfg->short_period = sys->fast_periodic_time;
    uid_cfg->long_period = sys->slow_periodic_time;
    return 0;
}
uint32_t lacp_sys_get_state(lacp_sys_state_t * uid_cfg)
{
    lacp_sys_t *sys = lacp_get_sys_inst();
    memcpy(uid_cfg->mac, sys->mac, 6);
    return 0;
}
uint32_t lacp_rx_lacpdu(uint32_t port_index, lacp_pdu_t * bpdu, uint32_t len)
{
    uint32_t ret = 0;
    register lacp_port_t *port;
    register lacp_sys_t *sys;

    LACP_CRITICAL_PATH_START;

    sys = lacp_get_sys_inst();
    if (!sys) {
        lacp_trace("the instance had not yet been created");

        LACP_CRITICAL_PATH_END;
        return M_LACP_NOT_CREATED;
    }

    port = _lacp_port_find (port_index);
    if (!port) {			/* port is absent in the stpm :( */
        LACP_CRITICAL_PATH_END;
        return M_RSTP_PORT_IS_ABSENT;
    }

    if (!port->port_enabled) { /* port link change indication will come later :( */
        lacp_trace ("disable port receive lacpdu port=%d :(", (uint32_t) port_index);
        LACP_CRITICAL_PATH_END;
        return M_RSTP_NOT_ENABLE;
    }

    if (LACP_ENABLED != sys->admin_state) { /* the stpm had not yet been enabled :( */
        LACP_CRITICAL_PATH_END;
        return M_RSTP_NOT_ENABLE;
    }

    ret = lacp_port_rx_lacpdu (port, bpdu, len);
    if (ret != 0)
    {
        ERR_LOG(ret, port->port_index, len, 0);
        LACP_CRITICAL_PATH_END;
        return ret;

    }

    lacp_sys_update (sys, LACP_SYS_UPDATE_READON_RX);
    LACP_CRITICAL_PATH_END;

    return ret;
}

uint32_t lacp_dbg_trace(uint32_t port_index, char *state_name, Bool on)
{
    lacp_port_t *port;
    port = _lacp_port_find(port_index);
    if (!port)
        return 0;

    lacp_dbg_trace_state_machine(port, state_name, on);
    return 0;
}

