#include "bitmap.h"
#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_ssp.h"
#include "lacp_util.h"
#include "lacp_stub.h"
#include "trunk_api.h"

uint32_t trunk_sys_set_prio(uint32_t prio)
{
    lacp_sys_cfg_t uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_PRIO;
    uid_cfg.priority = prio;
    lacp_sys_set_cfg(&uid_cfg);

    stub_db_set_sys_prio(prio);
    return 0;

}

#if 0
uint32_t trunk_sys_get_prio(uint32_t *prio)
{
    uint32_t ret = 0;
    ret = stub_db_get_sys_prio(prio);
    if (ret != 0)
    {
        *prio  = Default_system_priority;
    }
    return 0;
}
#endif

uint32_t trunk_sys_set_long_period(uint32_t period)
{
    lacp_sys_cfg_t uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_LONG_PERIOD;
    uid_cfg.long_period = period;
    lacp_sys_set_cfg(&uid_cfg);
    return 0;
}
uint32_t trunk_sys_set_short_period(uint32_t period)
{
    lacp_sys_cfg_t uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_SHORT_PERIOD;
    uid_cfg.short_period = period;
    lacp_sys_set_cfg(&uid_cfg);
    return 0;
}

uint32_t trunk_sys_set_period(uint32_t is_short)
{
    lacp_sys_cfg_t uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_PERIOD;
    uid_cfg.period = is_short;
    lacp_sys_set_cfg(&uid_cfg);
    return 0;
}

uint32_t trunk_sys_get_cfg(trunk_sys_cfg_t *cfg)
{
    uint32_t ret = 0;
    lacp_sys_cfg_t lacp_cfg;

    ret = lacp_sys_get_cfg(&lacp_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
    }

    cfg->prio = lacp_cfg.priority;
    cfg->long_period = lacp_cfg.long_period;
    cfg->short_period = lacp_cfg.short_period;
    cfg->period = lacp_cfg.period;

    return 0;
}
uint32_t trunk_sys_get_state(trunk_sys_state_t *cfg)
{
    uint32_t ret = 0;
    lacp_sys_state_t lacp_cfg;

    ret = lacp_sys_get_state(&lacp_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
    }

    memcpy(cfg->mac, lacp_cfg.mac, 6);

    return 0;
}

uint32_t trunk_port_lacp_enable(uint32_t slot, uint32_t port, uint32_t agg_id)
{
    uint32_t ret = 0;
    uint32_t port_index = LACP_UINT_INVALID;
    lacp_port_cfg_t uid_cfg;
    lacp_bitmap_t ports;

    memset(&uid_cfg, 0, sizeof(uid_cfg));
    lacp_bitmap_clear(&ports);

    ret = lacp_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    lacp_bitmap_set_bit(&ports, port_index);

    lacp_create_ports(&ports);

    stub_db_agg_add_member(agg_id, slot, port);

    uid_cfg.field_mask = PT_CFG_STATE;
    uid_cfg.lacp_enabled = True;
    uid_cfg.agg_id = agg_id;

    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);

    uid_cfg.field_mask |= PT_CFG_COST;
    uid_cfg.field_mask |= PT_CFG_STAT;
    uid_cfg.field_mask |= PT_CFG_PRIO;
    ret = trunk_port_get_prio(slot, port, &uid_cfg.port_priority);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, port_index);
        return ret;
    }

    lacp_port_set_cfg(&uid_cfg);

    if (lacp_ssp_get_port_link_status(port_index))
        lacp_port_link_change(port_index, 1);
    else
        lacp_port_link_change(port_index, 0);

    return 0;
}

uint32_t trunk_port_lacp_disable_batch(lacp_bitmap_t *ports)
{
    lacp_port_cfg_t uid_cfg;

    memset(&uid_cfg, 0, sizeof(uid_cfg));

    uid_cfg.field_mask = PT_CFG_STATE;
    uid_cfg.lacp_enabled = False;
    uid_cfg.agg_id = 0;
    lacp_bitmap_copy(&uid_cfg.port_bmp, ports);
    lacp_port_set_cfg(&uid_cfg);
    lacp_remove_ports(ports);
    return 0;
}
uint32_t trunk_agg_delete(uint32_t agg_id)
{
    uint32_t ret = 0;
    lacp_bitmap_t ports;
    uint32_t port_index;
    uchar_t pbmp[19];
    uint32_t slot;
    uint32_t port;

    stub_db_agg_get_member(agg_id, pbmp);
    for (slot = 0; slot <= 18; slot++)
    {
        for (port  = 1; port <= 8; port++)
        {
            if (BCM_HWW_TRUNK_GET_PBMP(slot, port, pbmp))
            {
                ret = lacp_ssp_get_global_index(slot, port, &port_index);
                if (ret != 0)
                {
                    continue;
//                                        return ret;
                }

                stub_db_agg_del_member(slot, port);
                lacp_bitmap_set_bit(&ports, port_index);
            }
        }
    }
    ret = trunk_port_lacp_disable_batch(&ports);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, port_index);
        return ret;
    }
    return 0;
}

uint32_t trunk_port_lacp_disable(uint32_t slot, uint32_t port)
{
    uint32_t ret = 0;
    uint32_t port_index;
    lacp_bitmap_t ports;

    ret = lacp_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    stub_db_agg_del_member(slot, port);

    lacp_bitmap_set_bit(&ports, port_index);
    ret = trunk_port_lacp_disable_batch(&ports);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, port_index);
        return ret;
    }

    return 0;
}
uint32_t trunk_port_get_prio(uint32_t slot, uint32_t port, uint32_t *prio)
{
    uint32_t ret;
    uint32_t port_index;

    ret = lacp_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = stub_get_port_priority(port_index, prio);
    if (ret != 0)
    {
        *prio = Default_port_priority;
    }

    return 0;
}

uint32_t trunk_port_set_prio(uint32_t slot, uint32_t port, uint32_t prio)
{
    lacp_port_cfg_t uid_cfg;
    uint32_t port_index;
    uint32_t ret = 0;

    lacp_ssp_get_global_index(slot, port, &port_index);

    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = PT_CFG_PRIO;
    uid_cfg.port_priority = prio;
    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);
    lacp_port_set_cfg(&uid_cfg);

    stub_set_port_priority(port_index, prio);

    return 0;

}

uint32_t trunk_port_set_speed(uint32_t slot, uint32_t port, uint32_t speed)
{
    uint32_t ret = 0;
    uint32_t port_index;

    ret = lacp_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = lacp_ssp_set_port_speed(port_index, speed);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}
uint32_t trunk_port_set_duplex(uint32_t slot, uint32_t port, uint32_t duplex)
{
    uint32_t ret = 0;
    uint32_t port_index;

    ret = lacp_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = lacp_ssp_set_port_duplex(port_index, duplex);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

uint32_t trunk_port_link_change(uint32_t slot, uint32_t port, uint32_t link_status)
{

    uint32_t ret = 0;
    uint32_t port_index;

    lacp_ssp_get_global_index(slot, port, &port_index);

    lacp_port_link_change(port_index, link_status);
    return 0;

}

uint32_t trunk_port_get_lacp_info(uint32_t slot, uint32_t port, lacp_port_state_t *uid_port_state)
{

    uint32_t ret = 0;
    uint32_t port_index;
    lacp_ssp_get_global_index(slot, port, &port_index);
    ret = lacp_port_get_port_state(port_index, uid_port_state);
    if ( 0 != ret || 0 == uid_port_state->agg_id)
    {
        return M_LACP_NOT_ENABLE;
    }

    return 0;

}

uint32_t trunk_port_clear_stat(uint32_t slot, uint32_t port)
{
    lacp_port_cfg_t uid_cfg;
    uint32_t port_index;

    uint32_t ret = 0;
    lacp_ssp_get_global_index(slot, port, &port_index);

    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = PT_CFG_STAT;
    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);
    lacp_port_set_cfg(&uid_cfg);
    return 0;
}


