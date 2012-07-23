#include "lacp_base.h"
#include "lacp_stub.h"
#include "lacp_api.h"
uint32_t g_sys_prio = 0xffffffff;
port_attr_t g_port_list[144];
LINK_GROUP_T g_link_groups[32];

uint32_t stub_init()
{
    uint32_t i;
    memset(g_link_groups, 0, sizeof(g_link_groups));
    memset(g_port_list, 0, sizeof(g_port_list));
    for (i = 0; i < sizeof(g_port_list)/sizeof(g_port_list[0]); i++)
    {
        g_port_list[i].slot = i/8;
        g_port_list[i].port = i%8+1;
        g_port_list[i].speed = 1000;
        g_port_list[i].duplex = 1;
        g_port_list[i].cd = 1;
        g_port_list[i].tid = 0;
        g_port_list[i].link_status = 0;
        g_port_list[i].prio = 0xffffffff;
    }

    return 0;

}

Bool stub_db_agg_has_member(uint32_t agg_id)
{
    if (g_link_groups[agg_id - 1].cnt)
        return True;
    else
        return False;
}

uint32_t stub_db_agg_get_member(uint32_t agg_id, uchar_t *pbmp)
{
    memcpy(pbmp, g_link_groups[agg_id - 1].ports, 19);
    return 0;
}

uint32_t stub_db_agg_add_member(uint32_t agg_id, uint32_t slot, uint32_t port)
{
    BCM_HWW_TRUNK_SET_PBMP(slot, port, g_link_groups[agg_id - 1].ports);
    g_link_groups[agg_id - 1].cnt ++;
    return 0;

}
uint32_t stub_db_agg_del_member(uint32_t slot, uint32_t port)
{
    uint32_t j;
    for (j=0; j<32; j++)
    {
        BCM_HWW_TRUNK_CLEAR_PBMP(slot, port, g_link_groups[j].ports);
    }

    return 0;


}
uint32_t stub_db_agg_get_port_tid(uint32_t slot, uint32_t port)
{
    uint32_t i;

    for (i=0; i<32; i++)
    {
        if (BCM_HWW_TRUNK_GET_PBMP(slot, port, g_link_groups[i].ports))
        {
            return i+1;
        }
    }
    return 0;



}
uint32_t stub_db_port_lacp_is_enable(uint32_t slot, uint32_t port)
{
    if (!stub_db_agg_get_port_tid(slot, port))
    {
        return False;
    }
    else
    {
        return True;
    }
}

uint32_t stub_get_port_attr(uint32_t port_index, port_attr_t *attr)
{
    memcpy(attr, &g_port_list[port_index], sizeof(port_attr_t));
    return 0;
}

uint32_t stub_set_port_attr(uint32_t port_index, port_attr_t *attr)
{
    memcpy(&g_port_list[port_index], attr, sizeof(port_attr_t));
    return 0;
}

uint32_t stub_get_port_priority(uint32_t port_index, uint32_t *prio)
{
    uint32_t ret = 0;
    port_attr_t attr;
    ret = stub_get_port_attr(port_index, &attr);
    if (attr.prio == 0xffffffff)
        return M_LACP_NOT_FOUND;
    *prio = attr.prio;
    return 0;
}

uint32_t stub_set_port_priority(uint32_t port_index, uint32_t prio)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    attr.prio = prio;
    stub_set_port_attr(port_index, &attr);
    return 0;
}

uint32_t stub_db_get_sys_prio(uint32_t *prio)
{
    if (g_sys_prio == 0xffffffff)
        return M_LACP_NOT_FOUND;

    *prio = g_sys_prio;
    return 0;
}
uint32_t stub_db_set_sys_prio(uint32_t prio)
{
    g_sys_prio = prio;
    return 0;
}

int cli_pr_get_attr(int argc, char **argv)
{
    uint32_t port_loop;
    uint32_t port_index;

    uint32_t port_start = 0;
    uint32_t port_end = LACP_PORT_MAX;
    port_attr_t attr;
    uint32_t ret = 0;
    uint32_t slot, port;

    if ('a' != argv[1][0])
    {

        ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
        if (ret !=0 )
        {
            return ret;
        }
        trunk_ssp_get_global_index(slot, port, &port_index);
        port_start = port_end = port_index;
    }

    printf("port    speed    duplex    rx&tx    chip_tgid");
    for (port_loop = port_start; port_loop < port_end; port_loop++)
    {
        stub_get_port_attr(port_loop, &attr);
        printf("\r\n%-4d     %-4d     %-4d     %-4d     %-4d", port_loop, attr.speed,
               attr.duplex,  attr.cd, attr.tid);
    }

    printf("\r\n");
    return 0;
}

