
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "cli.h"
#include "bitmap.h"
#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_ssp.h"
#include "lacp_util.h"
#include "lacp_stub.h"
#include "trunk_api.h"

extern  uint32_t stub_init();

extern uint32_t max_port;
int DEV_GetIfPonFromIfnet(char * sIfnet, int * pSlot, int * pPort)
{
    int rc;
    int ulSlot, ulPort;

    if (sIfnet == NULL)
    {
        return -1;
    }

    rc = sscanf(sIfnet, "%u/%u", &ulSlot, &ulPort);
    if (rc!=2)
    {
        return -1;
    }

    if (ulSlot < 0 || ulPort<= 0 || ulPort > 8)
    {
        return -1;
    }

    *pSlot = ulSlot;
    *pPort = ulPort;

    return 0;
}
static uint32_t lacp_port_lacp_enable (uint32_t argc, char** argv)
{
    uint32_t ret = 0;
    uint32_t slot;
    uint32_t port;

    uint32_t agg_id = atoi(argv[2]);

    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    if (!agg_id || agg_id >32)
    {
        ERR_LOG(ret, agg_id, 0, 0);
        return M_LACP_INTERNEL;
    }

    ret = trunk_port_lacp_enable(slot, port, agg_id);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, agg_id);
        return ret;
    }

    return 0;

}
static uint32_t lacp_port_lacp_disable (uint32_t argc, char** argv)
{
    uint32_t ret = 0;
    uint32_t slot;
    uint32_t port;

    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    trunk_port_lacp_disable(slot, port);
    return 0;
}


static uint32_t cli_br_get_cfg (uint32_t argc, char** argv)
{
#if 0
    uint32_t i, j;
    lacp_port_cfg_t uid_cfg;

    printf("\r\n agg_id  ports list");

    for (i = 0; i < 32; i++)
    {
        if (g_link_groups[i].cnt)
        {
            printf("\r\n %4d      ", i+1);

            for (j = 0; j < 8; j++)
            {
                if (g_link_groups[i].ports[j] == 0xffffffff)
                    continue;

                lacp_port_get_cfg(g_link_groups[i].ports[j], &uid_cfg);
                if (uid_cfg.sel_state)
                    printf("%d(*), ", g_link_groups[i].ports[j]);
                else
                    printf("%d, ", g_link_groups[i].ports[j]);
            }

        }

    }

    printf("\r\n");
#endif
    return 0;


}

void print_sep(uint32_t *i)
{
    if ((*i)%3 == 0)
        printf("\r\n");
    else
        printf("\t");
    (*i)++;

}

static uint32_t cli_pr_get_cfg (uint32_t argc, char** argv)
{
    lacp_port_t lacp_port;
    uint32_t i = 0;
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t port_index ;

    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return 0;
    }

    lacp_ssp_get_global_index(slot, port, &port_index);

    ret = lacp_port_get_dbg_cfg(port_index, &lacp_port);
    if (ret != 0)
    {
        printf("can't display port %d.\r\n", port_index);
        return 0;
    }

    print_sep(&i);
    printf(" port            : %d/%d", slot, port);
    print_sep(&i);
    printf(" port_index      : %d", port_index);
    print_sep(&i);
    printf(" lacp_enabled    : %d", lacp_port.lacp_enabled);
    print_sep(&i);
    printf(" port_enabled    : %d", lacp_port.port_enabled);
    print_sep(&i);
    printf(" rx state        : %d", lacp_port.rx->state);
    print_sep(&i);
    printf(" sel state       : %d", lacp_port.sel->state);
    print_sep(&i);
    printf(" mux state       : %d", lacp_port.mux->state);
    print_sep(&i);
    printf(" tx state        : %d", lacp_port.tx->state);
    print_sep(&i);
    printf(" port_moved      : %d", lacp_port.port_moved);
    print_sep(&i);
    printf(" agg id          : %d", lacp_port.agg_id);
    print_sep(&i);
    printf(" selected        : %d", lacp_port.selected);
    print_sep(&i);
    printf(" standby         : %d", lacp_port.standby);
    print_sep(&i);
    printf(" aport           : %d", lacp_port.master_port->port_index);
    print_sep(&i);
    printf(" ntt             : %d", lacp_port.ntt);
    print_sep(&i);
    printf(" hold_count      : %d", lacp_port.hold_count);
    print_sep(&i);
    printf(" ready_n         : %d", lacp_port.ready_n);
    print_sep(&i);
    printf(" speed           : %d", lacp_port.speed);
    print_sep(&i);
    printf(" duplex          : %d", lacp_port.duplex);
    print_sep(&i);
    printf(" rcvLacpdu       : %d", lacp_port.rcvd_lacpdu);
    print_sep(&i);
    printf(" tx_cnt          : %d", lacp_port.tx_lacpdu_cnt);
    print_sep(&i);
    printf(" rx_cnt          : %d", lacp_port.rx_lacpdu_cnt);
    print_sep(&i);
    printf(" periodic_while  : %d", lacp_port.periodic_timer);
    print_sep(&i);
    printf(" current_while   : %d", lacp_port.current_while);
    print_sep(&i);
    printf(" wait_while      : %d", lacp_port.wait_while);
    print_sep(&i);

    printf("\r\n actor:" );
    lacp_print_port_info(&lacp_port.actor);
    printf("\r\n parttor: " );
    lacp_print_port_info(&lacp_port.partner);
#if 1
    printf("\r\n msg actor: " );
    lacp_print_port_info(&lacp_port.msg_actor);
    printf("\r\n msg parttor: " );
    lacp_print_port_info(&lacp_port.msg_partner);
#endif
    return 0;

}

uint32_t cli_pr_set_speed(uint32_t argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t speed = atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_set_speed(slot, port, speed);
    return 0;

}

uint32_t cli_pr_set_duplex(uint32_t argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t speed = atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_set_duplex(slot, port, speed);
    return 0;


}

uint32_t cli_pr_get_attr(uint32_t argc, char **argv)
{
    uint32_t port_loop;
    uint32_t port_index;

    uint32_t port_start = 0;
    uint32_t port_end = max_port;
    port_attr_t attr;
    uint32_t ret = 0;
    uint32_t slot, port;

    if ('a' != argv[1][0])
    {

        ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
        if (ret !=0 )
        {
            return ret;
        }
        lacp_ssp_get_global_index(slot, port, &port_index);
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

uint32_t cli_pr_set_link(uint32_t argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t link_status = atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_link_change(slot, port, link_status);
    return 0;

}

uint32_t lacp_agg_delete(uint32_t argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t agg_id = atoi(argv[1]);
    ret = trunk_agg_delete(agg_id);
    if (ret != 0)
    {
        ERR_LOG(ret, agg_id, 0, 0);
        return 0;
    }

    return 0;
}

uint32_t cli_sysget_lacp_brief(uint32_t argc, char **argv)
{
    uint32_t i;
    UID_LAC_CFG_T  uid_cfg;
    uint32_t master_index = -1;
    unsigned char actor_sys_id_str[40];
    unsigned char partner_sys_id_str[40];
    unsigned char partner_sys_prio_str[20];
    UID_LAC_PORT_STATE_T uid_port_state[8] = {0};
    unsigned char master_str[10] = {0};
    unsigned char oper_key_str[10] = {0};
    uint32_t m_index = -1;
    uint32_t sel_cnt = 0 ;
    uint32_t standby_cnt = 0;
    uint32_t slot, port;

    lacp_sys_get_cfg(&uid_cfg);

    memset(uid_port_state, 0, sizeof(UID_LAC_PORT_STATE_T)*8);

    lacp_get_mac_str(uid_cfg.sys_mac, actor_sys_id_str);

    printf("\r\n Actor");
    printf("\r\n  Priority     : %-8d    MAC Address : %s", uid_cfg.priority, actor_sys_id_str);
    printf("\r\n  Short Period : %-8d    Long Period : %d", uid_cfg.short_period, uid_cfg.long_period);
    printf("\r\n ----------------------------------------------------------------------");
    printf("\r\n Agg  Partner  Parnter            Master   Selected   Standby   Oper ");
    printf("\r\n ID   Pri      MAC                Port     PortNum    PortNum   Key  ");
    printf("\r\n ----------------------------------------------------------------------");
    for (i = 0; i < 32; i++)
    {
        sel_cnt = standby_cnt = 0;
        master_index = -1;
        sprintf(master_str, "-");
        sprintf(partner_sys_id_str, "-");
        sprintf(partner_sys_prio_str, "-");
        sprintf(oper_key_str, "-");

        if (stub_db_agg_has_member(i+1))
        {
            lacp_agg_get_port_state(i+1, uid_port_state, &master_index);

            for (m_index = 0; m_index < 8; m_index++)
            {
                if (!uid_port_state[m_index].valid)
                    continue;

                if (uid_port_state[m_index].sel_state)
                {
                    sel_cnt++;
                }
                else
                {
                    standby_cnt++;
                }
            }

            if (master_index != -1)
            {
                slot = uid_port_state[master_index].slot;
                port = uid_port_state[master_index].port;
                sprintf(master_str, "%d/%d", slot, port);
                sprintf(partner_sys_prio_str, "%d", uid_port_state[master_index].partner.port_priority);
                lacp_get_mac_str(uid_port_state[master_index].partner.system_mac, partner_sys_id_str);
                sprintf(oper_key_str, "%d", uid_port_state[master_index].actor.key);
            }

            printf("\r\n %-5d%-9s%-19s%-9s%-11d%-10d%-3s",
                   (i+1), partner_sys_prio_str, partner_sys_id_str, master_str, sel_cnt, standby_cnt, oper_key_str);
        }

    }
    printf("\r\n ----------------------------------------------------------------------\r\n");
    return 0;
}

void cli_get_status_str(uchar_t status, uchar_t *str)
{
    uchar_t *col_str[] = {"Non-Collecting", "Collecting"};
    uchar_t *dis_str[] = {"Non-Distributing", "Distributing"};

    sprintf(str, "%s & %s", col_str[LACP_STATE_GET_BIT(status, LACP_STATE_COL)], dis_str[LACP_STATE_GET_BIT(status, LACP_STATE_DIS)]);

    return;

}
void cli_print_port_info(UID_LAC_PORT_STATE_T *uid_port_state)
{
    unsigned char partner_sys_id_str[40] = {0};
    unsigned char status_str[100] = {0};
    unsigned char *role_str[] = {"STANDBY", "SELECTED"};
    uint32_t slot, port;

    sprintf(partner_sys_id_str, "-");
    cli_get_status_str(uid_port_state->actor.state, status_str);

    lacp_get_mac_str(uid_port_state->partner.system_mac, partner_sys_id_str);

    lacp_ssp_change_to_slot_port(uid_port_state->actor.port_no - 1, &slot, &port);

    printf("\r\n Actor    Port            : %-2d/%-5d  Port Role       : %s",
           slot, port, role_str[uid_port_state->sel_state]);
    printf("\r\n          Port Priority   : %-8d  Port Number     : %d",
           uid_port_state->actor.port_priority, uid_port_state->actor.port_no);
    printf("\r\n          Oper-key        : %-8d  Port Status     : %s",
           uid_port_state->actor.key, status_str);

    cli_get_status_str(uid_port_state->partner.state, status_str);

    printf("\r\n Parnter  System Priority : %-8d  MAC Address     : %s",
           uid_port_state->partner.system_priority, partner_sys_id_str);
    printf("\r\n          Port Priority   : %-8d  Port Number     : %d",
           uid_port_state->partner.port_priority, uid_port_state->partner.port_no);
    printf("\r\n          Oper-key        : %-8d  Port Status     : %s",
           uid_port_state->partner.key, status_str);
    printf("\r\n");

    return;
}
uint32_t cli_sysget_lacp_verbose(uint32_t argc, char **argv)
{
    uint32_t i;
    UID_LAC_CFG_T  uid_cfg;
    uint32_t master_index = -1;
    unsigned char actor_sys_id_str[40];
    UID_LAC_PORT_STATE_T uid_port_state[8] = {0};
    unsigned char master_str[10] = {0};
    uint32_t agg_id = atoi(argv[1]);
    uint32_t slot, port;

    //TODO:: agg not exist
    lacp_sys_get_cfg(&uid_cfg);

    memset(uid_port_state, 0xff, sizeof(UID_LAC_PORT_STATE_T)*8);

    lacp_get_mac_str(uid_cfg.sys_mac, actor_sys_id_str);

    printf("\r\n Priority     : %-8d    MAC Address : %s", uid_cfg.priority, actor_sys_id_str);
    printf("\r\n Short Period : %-8d    Long Period : %d", uid_cfg.short_period, uid_cfg.long_period);

    sprintf(master_str, "-");

    if (stub_db_agg_has_member(agg_id))
    {
        lacp_agg_get_port_state(agg_id, uid_port_state, &master_index);

        if (master_index != -1)
        {
                slot = uid_port_state[master_index].slot;
                port = uid_port_state[master_index].port;
            sprintf(master_str, "%d/%d", slot, port);

        }

        printf("\r\n Master Port  : %s", master_str);
        printf("\r\n");

        for (i = 0; i < 8; i++)
        {
            if (!uid_port_state[i].valid)
                continue;

            cli_print_port_info(&uid_port_state[i]);
        }

    }
    return 0;
}
uint32_t cli_sysget_lacp_portinfo(uint32_t argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t i;
    UID_LAC_CFG_T  uid_cfg;
    uint32_t master_index = -1;
    unsigned char actor_sys_id_str[40];
    UID_LAC_PORT_STATE_T uid_port_state;
    unsigned char master_str[10] = {0};
    uint32_t slot, port;
    uint32_t slot_master, port_master;
    uint32_t agg_id;
    uint32_t prio;

    DEV_GetIfPonFromIfnet(argv[1], &slot, &port);

    agg_id = stub_db_agg_get_port_tid(slot, port);

    trunk_port_get_prio(slot, port, &prio);

    printf("\r\n Lacp Enable     : %-8s  Port Priority: %d", (agg_id == -1)? "disable" : "enable", prio);
    memset(&uid_port_state, 0, sizeof(UID_LAC_PORT_STATE_T));
    ret = trunk_port_get_lacp_info(slot, port, &uid_port_state);
    if ( 0 != ret || 0 == uid_port_state.agg_id)
    {
            printf("\r\n port is not in lacp aggregation gruop. ret:%d, agg:%d", ret, uid_port_state.agg_id);
        printf("\r\n");
        return 0;
    }

    lacp_get_mac_str(uid_port_state.actor.system_mac, actor_sys_id_str);
    lacp_ssp_change_to_slot_port(uid_port_state.master_port, &slot_master, &port_master);
    sprintf(master_str, "%d/%d", slot_master, port_master);

    printf("\r\n Selected AggID  : %-8d  Master Port  : %s", uid_port_state.agg_id, master_str);
    printf("\r\n System Priority : %-8d  MAC Address  : %s", uid_port_state.actor.system_priority, actor_sys_id_str);
    printf("\r\n Received LACPDU : %-8d  Sent LACPDU  : %d", uid_port_state.rx_cnt, uid_port_state.tx_cnt);
    printf("\r\n");
    cli_print_port_info(&uid_port_state);
    return 0;
}
uint32_t cli_port_clear_stat(uint32_t argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;

    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_clear_stat(slot, port);
    return 0;
}

uint32_t cli_sys_set_prio(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    return trunk_sys_set_prio(prio);

}

uint32_t cli_sys_set_long_period(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    return trunk_sys_set_long_period(prio);

}
uint32_t cli_sys_set_short_period(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    return trunk_sys_set_short_period(prio);
}
uint32_t cli_sys_set_period(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    return trunk_sys_set_period(prio);
}

uint32_t cli_debug_trace (int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t port_index;

    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    lacp_ssp_get_global_index(slot, port, &port_index);

    lacp_dbg_trace(port_index, argv[2], argv[3][0] != 'n' && argv[3][0] != 'N');
    return 0;
}
uint32_t cli_debug_pkt (int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;

    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    ret = lacp_dbg_pkt(slot, port, argv[2][0] == 't', argv[3][0] != 'n' && argv[3][0] != 'N');
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, 0);
        return ret;
    }
    return 0;
}
uint32_t cli_port_set_prio(uint32_t argc, char **argv)
{

    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t prio = atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], &slot, &port);
    if (ret !=0 )
    {
        return ret;
    }

    ret = trunk_port_set_prio(slot, port, prio);
    if (ret !=0 )
    {
        return ret;
    }

    return 0;

}

static CMD_DSCR_T lang[] = {

    THE_COMMAND("no link-group", "delete the link group")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(lacp_agg_delete)

    THE_COMMAND("port link-group", "add port to static lacp")
    PARAM_STRING("slot/port", "0/1")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(lacp_port_lacp_enable)

    THE_COMMAND("no port link-group", "disable lacp")
    PARAM_STRING("slot/port", "0/1")
    THE_FUNC(lacp_port_lacp_disable)

    THE_COMMAND("show link-group all", "get bridge config")
    THE_FUNC(cli_br_get_cfg)


    THE_COMMAND("show lacp link-group brief", "get bridge config")
    THE_FUNC(cli_sysget_lacp_brief)

    THE_COMMAND("show lacp link-group verbose", "get bridge config")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(cli_sysget_lacp_verbose)

    THE_COMMAND("show lacp port-info", "add port to static lacp")
    PARAM_STRING("slot/port", "0/1")
    THE_FUNC(cli_sysget_lacp_portinfo)

    THE_COMMAND("clear lacp stat", "add port to static lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    THE_FUNC(cli_port_clear_stat)

    THE_COMMAND("lacp priority", "add port to static lacp")
    PARAM_NUMBER("priority value", 1, 4, "all")
    THE_FUNC(cli_sys_set_prio)

    THE_COMMAND("lacp long-period", "add port to static lacp")
    PARAM_NUMBER("priority value", 20, 40, "30")
    THE_FUNC(cli_sys_set_long_period)

    THE_COMMAND("lacp short-period", "add port to static lacp")
    PARAM_NUMBER("priority value", 1, 10, "30")
    THE_FUNC(cli_sys_set_short_period)

    THE_COMMAND("lacp period", "add port to static lacp")
    PARAM_NUMBER("priority value", 0,1, "30")
    THE_FUNC(cli_sys_set_period)


    THE_COMMAND("lacp debug-fsm", "add port to static lacp")
    PARAM_STRING("slot/port", "0/1")


    PARAM_ENUM ("stame machine")
    PARAM_ENUM_SEL ("rx", "rx")
    PARAM_ENUM_SEL ("tx", "tx")
    PARAM_ENUM_SEL ("sel", "sel")
    PARAM_ENUM_SEL ("mux", "mux")
    PARAM_ENUM_DEFAULT ("all")
    PARAM_BOOL ("on/off", "trace it", "don't trace it", "n")
    THE_FUNC(cli_debug_trace)

    THE_COMMAND("lacp debug-pkt", "add port to static lacp")
    PARAM_STRING("slot/port", "0/1")

    PARAM_ENUM ("stame machine")
    PARAM_ENUM_SEL ("rx", "rx")
    PARAM_ENUM_SEL ("tx", "tx")
    PARAM_ENUM_DEFAULT ("all")
    PARAM_BOOL ("on/off", "trace it", "don't trace it", "n")
    THE_FUNC(cli_debug_pkt)

    THE_COMMAND("port lacp priority", "add port to static lacp")
    PARAM_STRING("slot/port", "0/1") 
    PARAM_NUMBER("priority value", 1, 4, "all")
    THE_FUNC(cli_port_set_prio)




    THE_COMMAND("show port", "get port config")
    PARAM_STRING("slot/port", "0/1")
    THE_FUNC(cli_pr_get_cfg)

    THE_COMMAND("set speed ", "set port speed")
    PARAM_STRING("slot/port", "0/1")
    PARAM_NUMBER("speed", 1, 10000, "all")
    THE_FUNC(cli_pr_set_speed)


    THE_COMMAND("set duplex ", "set port speed")
    PARAM_STRING("slot/port", "0/1")
    PARAM_NUMBER("duplex", 0, 10000, "all")
    THE_FUNC(cli_pr_set_duplex)

    THE_COMMAND("set link ", "set port speed")
    PARAM_STRING("slot/port", "0/1")
    PARAM_NUMBER("status", 0, 1, "1")
    THE_FUNC(cli_pr_set_link)


    THE_COMMAND("show attr ", "show port speed")
    PARAM_STRING("slot/port", "all")
    THE_FUNC(cli_pr_get_attr)

    END_OF_LANG
}
;

uint32_t lacp_cli_init (void)
{
    cli_register_language (lang);
    stub_init();

    return 0;
}


