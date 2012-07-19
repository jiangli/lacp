
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
#include "lacp_stub.h"

extern  uint32_t aggregator_init();

extern uint32_t max_port;

static uint32_t lacp_port_lacp_enable (uint32_t argc, char** argv)
{
    uint32_t port_loop;

    uint32_t port_start = 0;
    uint32_t port_end = max_port - 1;

    uint32_t port_index = 0xFFFFFFFF;
    uint32_t agg_id = atoi(argv[2]);
    lacp_port_cfg_t uid_cfg;
    lacp_bitmap_t ports;

    memset(&uid_cfg, 0, sizeof(uid_cfg));

    if ('a' != argv[1][0])
    {
        port_index = atoi(argv[1]);
        port_start = port_end = port_index;

    }

    if (!agg_id || agg_id >32)
    {
        printf("aggid error. r\n");
        return -1;
    }

    for (port_loop  = port_start; port_loop <= port_end; port_loop++)
    {
        lacp_bitmap_set_bit(&ports, port_loop);
    }

    lacp_create_ports(&ports);

    for (port_loop  = port_start; port_loop <= port_end; port_loop++)
    {
        aggregator_add_member(agg_id, port_loop);

        uid_cfg.field_mask = PT_CFG_STATE;
        uid_cfg.lacp_enabled = True;
        uid_cfg.agg_id = agg_id;

        lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_loop);

        uid_cfg.field_mask |= PT_CFG_COST;
        uid_cfg.field_mask |= PT_CFG_PRIO;
        uid_cfg.port_priority = 2;
        uid_cfg.field_mask |= PT_CFG_STAT;

        lacp_port_set_cfg(&uid_cfg);

        if (lacp_ssp_get_port_link_status(port_loop))
            lacp_port_link_change(port_index, 1);
        else
            lacp_port_link_change(port_index, 0);

    }


    return 0;

}
static uint32_t _lacp_port_lacp_delete_range(uint32_t port_start, uint32_t port_end)
{
    uint32_t port_loop;
    lacp_port_cfg_t uid_cfg;

    memset(&uid_cfg, 0, sizeof(uid_cfg));

    for (port_loop  = port_start; port_loop <= port_end; port_loop++)
    {
        aggregator_del_member(port_loop);
        lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_loop);
    }
    uid_cfg.field_mask = PT_CFG_STATE;
    uid_cfg.lacp_enabled = False;
    uid_cfg.agg_id = 0;
    lacp_port_set_cfg(&uid_cfg);
    lacp_remove_ports(&uid_cfg.port_bmp);
}
static uint32_t lacp_port_lacp_disable (uint32_t argc, char** argv)
{
    uint32_t port_start = 0;
    uint32_t port_end = max_port - 1;
    uint32_t port_index;

    if ('a' != argv[1][0])
    {
        port_index = atoi(argv[1]);
        port_start = port_end = port_index;
    }
    _lacp_port_lacp_delete_range(port_start, port_end);
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

static void
get_sysid_str (uint32_t prio, unsigned char *addr, unsigned char *str)
{
    sprintf((char *)str, "%u-%02x:%02x:%02x:%02x:%02x:%02x", prio,
            (unsigned char) addr[0],
            (unsigned char) addr[1],
            (unsigned char) addr[2],
            (unsigned char) addr[3],
            (unsigned char) addr[4],
            (unsigned char) addr[5]);
    return;

}
static void
get_mac_str (unsigned char *addr, unsigned char *str)
{
    sprintf((char *)str, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char) addr[0],
            (unsigned char) addr[1],
            (unsigned char) addr[2],
            (unsigned char) addr[3],
            (unsigned char) addr[4],
            (unsigned char) addr[5]);
    return;

}

static void
_lacp_in_display_bit (unsigned char bitmask,
                      char *bit_name, char *bit_fmt, unsigned char flags)
{
    uint32_t the_bit = (flags & bitmask) ? 1 : 0;

    printf ("    ");
    printf (bit_fmt, the_bit);
    printf (" %-20s  %s\n", bit_name, the_bit ? "- yes" : "");
}

static void print_info(lacp_port_info_t *lacp_info)
{
    unsigned char sysid_str[40] = {0};

    get_sysid_str(lacp_info->system_priority, lacp_info->system_mac, sysid_str);

    printf("\r\n System ID:%s", sysid_str);
    printf("\r\n port     :%d", lacp_info->port_no);
    printf("\t priority :%d", lacp_info->port_priority);
    printf("\t key      :%d", lacp_info->key);
    printf("\r\n state    \r\n");
    _lacp_in_display_bit(LACP_STATE_ACT, "LACP_Activity", "%d.......", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_TMT, "LACP_Timeout", ".%d......", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_AGG, "Aggregation", "..%d.....", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_SYN, "Synchronization", "...%d....", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_COL, "Collecting", "....%d...", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_DIS, "Distributing", ".....%d..", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_DEF, "Defaulted", "......%d.", lacp_info->state);
    _lacp_in_display_bit(LACP_STATE_EXP, "Expired", ".......%d", lacp_info->state);

    return ;

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
    uint32_t port_index = atoi(argv[1]);
    lacp_port_t port;
    uint32_t i = 0;
    uint32_t ret = 0;

    ret = lacp_port_get_dbg_cfg(port_index, &port);
    if (ret != 0)
    {
        printf("can't display port %d.\r\n", port_index);
        return 0;
    }
    print_sep(&i);
    printf(" port_index      : %d", port.port_index);
    print_sep(&i);
    printf(" lacp_enabled    : %d", port.lacp_enabled);
    print_sep(&i);
    printf(" port_enabled    : %d", port.port_enabled);
    print_sep(&i);
    printf(" rx state        : %d", port.rx->state);
    print_sep(&i);
    printf(" sel state       : %d", port.sel->state);
    print_sep(&i);
    printf(" mux state       : %d", port.mux->state);
    print_sep(&i);
    printf(" tx state        : %d", port.tx->state);
    print_sep(&i);
    printf(" port_moved      : %d", port.port_moved);
    print_sep(&i);
    printf(" agg id          : %d", port.agg_id);
    print_sep(&i);
    printf(" selected        : %d", port.selected);
    print_sep(&i);
    printf(" standby         : %d", port.standby);
    print_sep(&i);
//    printf(" aport           : %d", port.master_port->port_index);
    print_sep(&i);
    printf(" ntt             : %d", port.ntt);
    print_sep(&i);
    printf(" hold_count      : %d", port.hold_count);
    print_sep(&i);
    printf(" ready_n         : %d", port.ready_n);
    print_sep(&i);
    printf(" speed           : %d", port.speed);
    print_sep(&i);
    printf(" duplex          : %d", port.duplex);
    print_sep(&i);
    printf(" rcvLacpdu       : %d", port.rcvd_lacpdu);
    print_sep(&i);
    printf(" tx_cnt          : %d", port.tx_lacpdu_cnt);
    print_sep(&i);
    printf(" rx_cnt          : %d", port.rx_lacpdu_cnt);
    print_sep(&i);
    printf(" periodic_while  : %d", port.periodic_timer);
    print_sep(&i);
    printf(" current_while   : %d", port.current_while);
    print_sep(&i);
    printf(" wait_while      : %d", port.wait_while);
    print_sep(&i);

    printf("\r\n actor:" );
    print_info(&port.actor);
    printf("\r\n parttor: " );
    print_info(&port.partner);
#if 1
    printf("\r\n msg actor: " );
    print_info(&port.msg_actor);
    printf("\r\n msg parttor: " );
    print_info(&port.msg_partner);
#endif
    return 0;

}

uint32_t cli_pr_set_speed(uint32_t argc, char **argv)
{

    uint32_t port_index = atoi(argv[1]);
    uint32_t speed = atoi(argv[2]);
    lacp_ssp_set_port_speed(port_index, speed);
    return 0;

}

uint32_t cli_pr_set_duplex(uint32_t argc, char **argv)
{

    uint32_t port_index = atoi(argv[1]);
    uint32_t speed = atoi(argv[2]);
    lacp_ssp_set_port_duplex(port_index, speed);
    return 0;


}

uint32_t cli_pr_get_attr(uint32_t argc, char **argv)
{
    uint32_t port_loop;
    uint32_t port_index;

    uint32_t port_start = 0;
    uint32_t port_end = max_port;
    port_attr_t attr;
    if ('a' != argv[1][0])
    {
        port_index = atoi(argv[1]);
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
    uint32_t port_index = atoi(argv[1]);
    uint32_t link_status = atoi(argv[2]);
    lacp_port_link_change(port_index, link_status);
    return 0;

}

uint32_t lacp_agg_delete(uint32_t argc, char **argv)
{
    uint32_t agg_id = atoi(argv[1]);
    _lacp_port_lacp_delete_range(0, max_port - 1);
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

    lacp_sys_get_cfg(&uid_cfg);

    memset(uid_port_state, 0, sizeof(UID_LAC_PORT_STATE_T)*8);

    get_mac_str(uid_cfg.sys_mac, actor_sys_id_str);

    printf("\r\n Actor");
    printf("\r\n  Priority     : %d    MAC Address : %s", uid_cfg.priority, actor_sys_id_str);
    printf("\r\n  Short Period : %d    Long Period : %d", uid_cfg.short_period, uid_cfg.long_period);
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

        if (aggregator_has_member(i+1))
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
                sprintf(master_str, "%d", uid_port_state[master_index].port_index);
                sprintf(partner_sys_prio_str, "%d", uid_port_state[master_index].partner.port_priority);
                get_mac_str(uid_port_state[master_index].partner.system_mac, partner_sys_id_str);
                sprintf(oper_key_str, "%d", uid_port_state[master_index].actor.key);
            }

            printf("\r\n %-5d%-9s%-19s%-9s%-11d%-10d%-3s",
                   (i+1), partner_sys_prio_str, partner_sys_id_str, master_str, sel_cnt, standby_cnt, oper_key_str);
        }

    }
    printf("\r\n ----------------------------------------------------------------------\r\n");
    return 0;
}

void cli_print_port_info(UID_LAC_PORT_STATE_T *uid_port_state)
{
    unsigned char partner_sys_id_str[40];
    unsigned char *role_str[] = {"STANDBY", "SELECTED"};

    sprintf(partner_sys_id_str, "-");

    get_mac_str(uid_port_state->partner.system_mac, partner_sys_id_str);

    printf("\r\n Actor    Port Number     : %d",
           uid_port_state->actor.port_no);
    printf("\r\n          Oper-key        : %-4d(0x%-4x)  Port Priority: %-4d",
           uid_port_state->actor.key,uid_port_state->actor.key, uid_port_state->actor.port_priority);
    printf("\r\n          Port Role       : %-8s      Port Status  : %d",
           role_str[uid_port_state->sel_state], uid_port_state->actor.state);

    printf("\r\n Parnter  System Priority : %-8d      MAC Address  : %s",
           uid_port_state->partner.system_priority, partner_sys_id_str);
    printf("\r\n          Port Priority   : %-8d      Port Number  : %d",
           uid_port_state->partner.port_priority, uid_port_state->partner.port_no);
    printf("\r\n          Oper-key        : %-8d      Port Status  : %d",
           uid_port_state->partner.key, uid_port_state->partner.state);
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
    lacp_sys_get_cfg(&uid_cfg);

    memset(uid_port_state, 0xff, sizeof(UID_LAC_PORT_STATE_T)*8);

    get_mac_str(uid_cfg.sys_mac, actor_sys_id_str);

    printf("\r\n Priority     : %d    MAC Address : %s", uid_cfg.priority, actor_sys_id_str);
    printf("\r\n Short Period : %d    Long Period : %d", uid_cfg.short_period, uid_cfg.long_period);

    sprintf(master_str, "-");

    if (aggregator_has_member(agg_id))
    {
        lacp_agg_get_port_state(agg_id, uid_port_state, &master_index);

        if (master_index != -1)
        {
            sprintf(master_str, "%d", uid_port_state[master_index].port_index);
        }

        printf("\r\n Master Port  : %s", master_str);
        printf("\r\n");

        for (i = 0; i < 8; i++)
        {
            if (uid_port_state[i].port_index == 0xffffffff)
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
    uint32_t port_index = atoi(argv[1]);

    memset(&uid_port_state, 0, sizeof(UID_LAC_PORT_STATE_T));
    ret = lacp_port_get_port_state(port_index, &uid_port_state);
    if ( 0 != ret || 0 == uid_port_state.agg_id)
    {
        printf("\r\n port is not in lacp aggregation gruop.");
        return 0;
    }

    get_mac_str(uid_port_state.actor.system_mac, actor_sys_id_str);

    sprintf(master_str, "%d", uid_port_state.master_port);

    printf("\r\n Selected AggID  : %d    Master Port : %s", uid_port_state.agg_id, master_str);
    printf("\r\n System Priority : %d    MAC Address : %s", uid_port_state.actor.system_priority, actor_sys_id_str);
    printf("\r\n Received LACPDU : %d    Sent LACPDU : %d", uid_port_state.rx_cnt, uid_port_state.tx_cnt);
    printf("\r\n");
    cli_print_port_info(&uid_port_state);
    return 0;
}
uint32_t cli_port_clear_stat(uint32_t argc, char **argv)
{
    lacp_port_cfg_t uid_cfg;
    uint32_t port_index = atoi(argv[1]);
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = PT_CFG_STAT;
    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);
    lacp_port_set_cfg(&uid_cfg);
    return 0;
}
uint32_t cli_sys_set_prio(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_PRIO;
    uid_cfg.priority = prio;
    lacp_sys_set_cfg(&uid_cfg);
}
uint32_t cli_sys_set_long_period(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_LONG_PERIOD;
    uid_cfg.long_period = prio;
    lacp_sys_set_cfg(&uid_cfg);
    return 0;
}
uint32_t cli_sys_set_short_period(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_SHORT_PERIOD;
    uid_cfg.short_period = prio;
    lacp_sys_set_cfg(&uid_cfg);
    return 0;
}
uint32_t cli_sys_set_period(uint32_t argc, char **argv)
{
    uint32_t prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_PERIOD;
    uid_cfg.period = prio;
    lacp_sys_set_cfg(&uid_cfg);
    return 0;
}

static CMD_DSCR_T lang[] = {

    THE_COMMAND("no link-group", "delete the link group")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(lacp_agg_delete)

    THE_COMMAND("port link-group", "add port to static lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(lacp_port_lacp_enable)

    THE_COMMAND("no port link-group", "disable lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    THE_FUNC(lacp_port_lacp_disable)

    THE_COMMAND("show link-group all", "get bridge config")
    THE_FUNC(cli_br_get_cfg)


    THE_COMMAND("show lacp link-group brief", "get bridge config")
    THE_FUNC(cli_sysget_lacp_brief)

    THE_COMMAND("show lacp link-group verbose", "get bridge config")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(cli_sysget_lacp_verbose)

    THE_COMMAND("show lacp port-info", "add port to static lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
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







    THE_COMMAND("show port", "get port config")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    THE_FUNC(cli_pr_get_cfg)

    THE_COMMAND("set speed ", "set port speed")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    PARAM_NUMBER("speed", 1, 10000, "all")
    THE_FUNC(cli_pr_set_speed)


    THE_COMMAND("set duplex ", "set port speed")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    PARAM_NUMBER("duplex", 0, 10000, "all")
    THE_FUNC(cli_pr_set_duplex)

    THE_COMMAND("set link ", "set port speed")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    PARAM_NUMBER("status", 0, 1, "1")
    THE_FUNC(cli_pr_set_link)


    THE_COMMAND("show attr ", "show port speed")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    THE_FUNC(cli_pr_get_attr)

    END_OF_LANG
}
;

uint32_t lacp_cli_init (void)
{
    cli_register_language (lang);
    aggregator_init();

    return 0;
}


