#include "bitmap.h"
#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_ssp.h"
#include "lacp_util.h"
#include "lacp_stub.h"

uint32_t trunk_port_lacp_enable(uint32 slot, uint32 port, uint32 agg_id)
{
        uint32_t ret = 0;
    uint32_t slot;
    uint32_t port;
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

        lacp_bitmap_set_bit(&ports, port_loop);


    lacp_create_ports(&ports);

        aggregator_add_member(agg_id, port_index);

        uid_cfg.field_mask = PT_CFG_STATE;
        uid_cfg.lacp_enabled = True;
        uid_cfg.agg_id = agg_id;

        lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);

        uid_cfg.field_mask |= PT_CFG_COST;
        uid_cfg.field_mask |= PT_CFG_PRIO;
        uid_cfg.port_priority = 2;
        uid_cfg.field_mask |= PT_CFG_STAT;

        lacp_port_set_cfg(&uid_cfg);

        if (lacp_ssp_get_port_link_status(port_index))
            lacp_port_link_change(port_index, 1);
        else
            lacp_port_link_change(port_index, 0);

}
uint32_t trunk_port_lacp_disable_bmp(lacp_bitmap_t *ports)
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
uint32_t trunk_agg_delete(uint32 agg_id)
{
        uint32_t ret = 0;
        lacp_bitmap_t ports;
        uint32_t port_index;
        uchar_t pbmp[19];

        aggregator_get_member(agg_id, pbmp);

        for (slot = 0; slot <= 18; slot++)
        {
                for (port_loop  = 1; port_loop <= 8; port_loop++)
                {
                        if (BCM_HWW_TRUNK_GET_PBMP(slot, port, pbmp))
                        {
                                ret = lacp_ssp_get_global_index(slot, port, &port_index);
                                if (ret != 0)
                                {
                                        continue;
//                                        return ret;
                                }
                                
                                aggregator_del_member(port_index);
                                lacp_bitmap_set_bit(&ports, port_index);
                        }
                }
    }
        ret = trunk_port_lacp_disable_bmp(&ports);
        if (ret != 0)
        {
                ERR_LOG(ret, slot, port, port_index);
                return ret;
        }

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
        
        aggregator_del_member(port_index);

        lacp_bitmap_set_bit(&ports, port_index);
        ret = trunk_port_lacp_disable_bmp(&ports);
        if (ret != 0)
        {
                ERR_LOG(ret, slot, port, port_index);
                return ret;
        }

        return 0;
}
