#include "bitmap.h"
#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_util.h"
#include "lacp_stub.h"
#include "trunk_api.h"
#include "trunk_ssp.h"

uint32_t trunk_sys_set_prio(uint32_t prio)
{
    uint32_t ret = 0;
    lacp_sys_cfg_t sys_cfg;

    memset(&sys_cfg, 0, sizeof(sys_cfg));

    sys_cfg.field_mask = BR_CFG_PRIO;
    sys_cfg.priority = prio;
    ret = lacp_sys_set_cfg(&sys_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, prio, 0, 0);
        return ret;
    }

    ret = stub_db_set_sys_prio(prio);
    if (ret != 0)
    {
        ERR_LOG(ret, prio, 0, 0);
        return ret;
    }
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
    uint32_t ret = 0;
    lacp_sys_cfg_t sys_cfg;
    memset(&sys_cfg, 0, sizeof(sys_cfg));
    sys_cfg.field_mask = BR_CFG_LONG_PERIOD;
    sys_cfg.long_period = period;
    ret = lacp_sys_set_cfg(&sys_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, period, 0, 0);
        return ret;
    }
    return 0;
}
uint32_t trunk_sys_set_short_period(uint32_t period)
{
    uint32_t ret = 0;
    lacp_sys_cfg_t sys_cfg;
    memset(&sys_cfg, 0, sizeof(sys_cfg));
    sys_cfg.field_mask = BR_CFG_SHORT_PERIOD;
    sys_cfg.short_period = period;
    ret = lacp_sys_set_cfg(&sys_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, period, 0, 0);
        return ret;
    }

    return 0;
}

uint32_t trunk_sys_set_period(uint32_t is_short)
{
    uint32_t ret = 0;
    lacp_sys_cfg_t sys_cfg;
    memset(&sys_cfg, 0, sizeof(sys_cfg));
    sys_cfg.field_mask = BR_CFG_PERIOD;
    sys_cfg.period = is_short;
    ret = lacp_sys_set_cfg(&sys_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, is_short, 0, 0);
        return ret;
    }

    return 0;
}

uint32_t trunk_sys_get_cfg(trunk_sys_cfg_t *cfg)
{
    uint32_t ret = 0;
    lacp_sys_cfg_t sys_cfg;

    memset(&sys_cfg, 0, sizeof(sys_cfg));

    ret = lacp_sys_get_cfg(&sys_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    cfg->prio = sys_cfg.priority;
    cfg->long_period = sys_cfg.long_period;
    cfg->short_period = sys_cfg.short_period;
    cfg->period = sys_cfg.period;

    return 0;
}
uint32_t trunk_sys_get_state(trunk_sys_state_t *cfg)
{
    uint32_t ret = 0;
    lacp_sys_state_t sys_cfg;

    memset(&sys_cfg, 0, sizeof(sys_cfg));

    ret = lacp_sys_get_state(&sys_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    memcpy(cfg->mac, sys_cfg.mac, 6);

    return 0;
}

uint32_t trunk_port_lacp_enable(uint32_t slot, uint32_t port, uint32_t agg_id)
{
    uint32_t ret = 0;
    uint32_t port_index = TRUNK_UINT_INVALID;
    lacp_port_cfg_t uid_cfg;
    lacp_bitmap_t ports;

    /* TODO:: check if already enable */

    memset(&uid_cfg, 0, sizeof(uid_cfg));
    lacp_bitmap_clear(&ports);

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
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
    ret = trunk_port_get_prio(slot, port, (uint32_t *)&uid_cfg.port_priority);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, port_index);
        return ret;
    }

    ret = lacp_port_set_cfg(&uid_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, port_index);
        return ret;
    }

    if (trunk_ssp_get_port_link_status(port_index))
    {
        ret = lacp_port_link_change(port_index, 1);
    }
    else
    {
        ret = lacp_port_link_change(port_index, 0);
    }
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, port_index);
        return ret;
    }

    return 0;
}

uint32_t trunk_port_lacp_disable_batch(lacp_bitmap_t *ports)
{
    uint32_t ret = 0;
    lacp_port_cfg_t port_cfg;

    memset(&port_cfg, 0, sizeof(port_cfg));

    port_cfg.field_mask = PT_CFG_STATE;
    port_cfg.lacp_enabled = False;
    port_cfg.agg_id = 0;
    lacp_bitmap_copy(&port_cfg.port_bmp, ports);

    ret = lacp_port_set_cfg(&port_cfg);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }
    ret = lacp_remove_ports(ports);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    return 0;
}
uint32_t trunk_agg_delete(uint32_t agg_id)
{
    uint32_t ret = 0;
    uint32_t port_index;
    uint32_t slot;
    uint32_t port;
    uchar_t pbmp[19] = {0};

    lacp_bitmap_t ports;

    lacp_bitmap_clear(&ports);

    stub_db_agg_get_member(agg_id, pbmp);
    lacp_memdump(pbmp, 19);

    for (slot = 0; slot <= 18; slot++)
    {

        for (port  = 1; port <= 8; port++)
        {
            if (BCM_HWW_TRUNK_GET_PBMP(slot, port, pbmp))
            {
                ret = trunk_ssp_get_global_index(slot, port, &port_index);
                if (ret != 0)
                {
                    continue;
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

    lacp_bitmap_clear(&ports);

    if (!stub_db_port_lacp_is_enable(slot, port))
        return 0;

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
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

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
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

    memset(&uid_cfg, 0, sizeof(lacp_port_cfg_t));
    trunk_ssp_get_global_index(slot, port, &port_index);

    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = PT_CFG_PRIO;
    uid_cfg.port_priority = prio;
    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);
    if (stub_db_port_lacp_is_enable(slot, port))
    {
        ret = lacp_port_set_cfg(&uid_cfg);
        if (ret != 0)
        {
            ERR_LOG(ret, slot, port, prio);
            return ret;
        }
    }

    stub_set_port_priority(port_index, prio);

    return 0;

}

uint32_t trunk_port_set_speed(uint32_t slot, uint32_t port, uint32_t speed)
{
    uint32_t ret = 0;
    uint32_t port_index;

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = trunk_ssp_set_port_speed(port_index, speed);
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

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = trunk_ssp_set_port_duplex(port_index, duplex);
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

    if (!stub_db_port_lacp_is_enable(slot, port))
    {
        return 0;
    }

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = lacp_port_link_change(port_index, link_status);
    if (ret != 0)
    {
        ERR_LOG(ret, port_index, link_status, 0);
        return ret;
    }
    return 0;

}

int trunk_port_state_compare(void *a, void *b)
{
    trunk_port_state_t *state1 = (trunk_port_state_t*)a;
    trunk_port_state_t *state2 = (trunk_port_state_t*)b;

    if (state1->valid && !state2->valid)
        return -1;
    if (state1->slot - state2->slot)
        return (state1->slot - state2->slot);
    else
        return (state1->port - state2->port);
}

uint32_t trunk_port_state_sort(trunk_port_state_t *uid_port_state)
{
    qsort(uid_port_state, 8, sizeof(trunk_port_state_t), trunk_port_state_compare);
    return 0;
}

uint32_t trunk_agg_get_state(int agg_id, trunk_agg_state_t *agg_state)
{
    uint32_t ret = 0;
    lacp_port_state_t port_state[8];
    uint32_t master_index = TRUNK_UINT_INVALID;
    uint32_t index = 0;

    memset(port_state, 0, sizeof(port_state));
    ret = lacp_agg_get_port_state(agg_id, port_state, &master_index);
    if (ret != 0)
    {
        ERR_LOG(ret, agg_id, 0, 0);
        return ret;
    }


    agg_state->master_index = master_index;

    for (index = 0; index < 8; index++)
    {
        agg_state->ports_state[index].valid = port_state[index].valid;

        lacp_ssp_change_to_slot_port(port_state[index].port_index, &agg_state->ports_state[index].slot, &agg_state->ports_state[index].port);
        agg_state->ports_state[index].key = port_state[index].key;
        agg_state->ports_state[index].agg_id = port_state[index].agg_id;
        agg_state->ports_state[index].master_port = port_state[index].master_port;
        agg_state->ports_state[index].sel_state = port_state[index].sel_state;
        agg_state->ports_state[index].rx_cnt = port_state[index].rx_cnt;
        agg_state->ports_state[index].tx_cnt = port_state[index].tx_cnt;
        memcpy(&agg_state->ports_state[index].actor, &port_state[index].actor, sizeof(lacp_port_info_t));
        memcpy(&agg_state->ports_state[index].partner, &port_state[index].partner, sizeof(lacp_port_info_t));
    }

    return 0;
}
uint32_t trunk_port_get_lacp_info(uint32_t slot, uint32_t port, trunk_port_state_t *trunk_port_state)
{
    uint32_t ret = 0;
    uint32_t port_index;
    lacp_port_state_t port_state;

    ret = trunk_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    ret = lacp_port_get_port_state(port_index, &port_state);
    if ( 0 != ret || 0 == port_state.agg_id)
    {
        ERR_LOG(ret, port_index, 0, 0);
        return M_LACP_NOT_ENABLE;
    }

    trunk_port_state->valid = port_state.valid;

    lacp_ssp_change_to_slot_port(port_state.port_index, &trunk_port_state->slot, &trunk_port_state->port);

    trunk_port_state->key = port_state.key;
    trunk_port_state->agg_id = port_state.agg_id;
    trunk_port_state->master_port = port_state.master_port;
    trunk_port_state->sel_state = port_state.sel_state;
    trunk_port_state->rx_cnt = port_state.rx_cnt;
    trunk_port_state->tx_cnt = port_state.tx_cnt;
    memcpy(&trunk_port_state->actor, &port_state.actor, sizeof(lacp_port_info_t));
    memcpy(&trunk_port_state->partner, &port_state.partner, sizeof(lacp_port_info_t));

    return 0;

}

uint32_t trunk_port_clear_stat(uint32_t slot, uint32_t port)
{
    uint32_t ret = 0;
    lacp_port_cfg_t port_cfg;
    uint32_t port_index;

    trunk_ssp_get_global_index(slot, port, &port_index);

    memset(&port_cfg, 0, sizeof(port_cfg));
    port_cfg.field_mask = PT_CFG_STAT;
    lacp_bitmap_set_bit(&port_cfg.port_bmp, port_index);
    if ( stub_db_port_lacp_is_enable(slot, port))
    {
        ret = lacp_port_set_cfg(&port_cfg);
        if (ret != 0)
        {
            ERR_LOG(ret, slot, port, port_index);
            return ret;
        }
    }
    return 0;
}


