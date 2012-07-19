
/************************************************************************
 * RSTP library - Rapid Spanning Tree (802.1t, 802.1w)
 * Copyright (C) 2001-2003 Optical Access
 * Author: Alex Rozin
 *
 * This file is part of RSTP library.
 *
 * RSTP library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; version 2.1
 *
 * RSTP library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RSTP library; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "cli.h"
#include "lac_cli.h"
#include "bitmap.h"
#include "lac_base.h"
#include "lac_port.h"
#include "lac_sys.h"
#include "uid_lac.h"
#include "lac_in.h"
#include "lac_out.h"

extern  int aggregator_init();

extern LINK_GROUP_T g_link_groups[32];
extern int max_port;

static int lac_port_lacp_enable (int argc, char** argv)
{
    int port_loop;

    int port_start = 0;
    int port_end = max_port - 1;

    int port_index = 0xFFFFFFFF;
    int agg_id = atoi(argv[2]);
    UID_LAC_PORT_CFG_T uid_cfg;
    BITMAP_T ports;

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
        BitmapSetBit(&ports, port_loop);
    }

    lac_in_create_port(&ports);

    for (port_loop  = port_start; port_loop <= port_end; port_loop++)
    {
        aggregator_add_member(agg_id, port_loop);

        uid_cfg.field_mask = PT_CFG_STATE;
        uid_cfg.lacp_enabled = True;
        uid_cfg.agg_id = agg_id;

        BitmapSetBit(&uid_cfg.port_bmp, port_loop);

        uid_cfg.field_mask |= PT_CFG_COST;
        uid_cfg.field_mask |= PT_CFG_PRIO;
        uid_cfg.port_priority = 2;
        uid_cfg.field_mask |= PT_CFG_STAT;

        lac_port_set_cfg(&uid_cfg);

        if (lac_get_port_link_status(port_loop))
            lac_port_link_change(port_index, 1);
        else
            lac_port_link_change(port_index, 0);

    }


    return 0;

}
static int _lac_port_lacp_delete_range(int port_start, int port_end)
{
    int port_loop;
    UID_LAC_PORT_CFG_T uid_cfg;

    memset(&uid_cfg, 0, sizeof(uid_cfg));

    for (port_loop  = port_start; port_loop <= port_end; port_loop++)
    {
        aggregator_del_member(port_loop);
        BitmapSetBit(&uid_cfg.port_bmp, port_loop);
    }
    uid_cfg.field_mask = PT_CFG_STATE;
    uid_cfg.lacp_enabled = False;
    uid_cfg.agg_id = 0;
    lac_port_set_cfg(&uid_cfg);
    lac_in_remove_port(&uid_cfg.port_bmp);
}
static int lac_port_lacp_disable (int argc, char** argv)
{
    int port_start = 0;
    int port_end = max_port - 1;
    int port_index;

    if ('a' != argv[1][0])
    {
        port_index = atoi(argv[1]);
        port_start = port_end = port_index;
    }
    _lac_port_lacp_delete_range(port_start, port_end);
    return 0;
}


static int cli_br_get_cfg (int argc, char** argv)
{
    int i, j;
    UID_LAC_PORT_CFG_T uid_cfg;

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

                lac_port_get_cfg(g_link_groups[i].ports[j], &uid_cfg);
                if (uid_cfg.sel_state)
                    printf("%d(*), ", g_link_groups[i].ports[j]);
                else
                    printf("%d, ", g_link_groups[i].ports[j]);
            }

        }

    }

    printf("\r\n");
    return 0;


}

static void
get_sysid_str (int prio, unsigned char *addr, unsigned char *str)
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
_lac_in_display_bit (unsigned char bitmask,
                     char *bit_name, char *bit_fmt, unsigned char flags)
{
    int the_bit = (flags & bitmask) ? 1 : 0;

    printf ("    ");
    printf (bit_fmt, the_bit);
    printf (" %-20s  %s\n", bit_name, the_bit ? "- yes" : "");
}

static void print_info(LAC_PORT_INFO *lac_info)
{
    unsigned char sysid_str[40] = {0};

    get_sysid_str(lac_info->system_priority, lac_info->system_mac, sysid_str);

    printf("\r\n System ID:%s", sysid_str);
    printf("\r\n port     :%d", lac_info->port_index);
    printf("\t priority :%d", lac_info->port_priority);
    printf("\t key      :%d", lac_info->key);
    printf("\r\n state    \r\n");
    _lac_in_display_bit(LAC_STATE_ACT, "LACP_Activity", "%d.......", lac_info->state);
    _lac_in_display_bit(LAC_STATE_TMT, "LACP_Timeout", ".%d......", lac_info->state);
    _lac_in_display_bit(LAC_STATE_AGG, "Aggregation", "..%d.....", lac_info->state);
    _lac_in_display_bit(LAC_STATE_SYN, "Synchronization", "...%d....", lac_info->state);
    _lac_in_display_bit(LAC_STATE_COL, "Collecting", "....%d...", lac_info->state);
    _lac_in_display_bit(LAC_STATE_DIS, "Distributing", ".....%d..", lac_info->state);
    _lac_in_display_bit(LAC_STATE_DEF, "Defaulted", "......%d.", lac_info->state);
    _lac_in_display_bit(LAC_STATE_EXP, "Expired", ".......%d", lac_info->state);

    return ;

}

void print_sep(int *i)
{
    if ((*i)%3 == 0)
        printf("\r\n");
    else
        printf("\t");
    (*i)++;

}

static int cli_pr_get_cfg (int argc, char** argv)
{
    int port_index = atoi(argv[1]);
    LAC_PORT_T port;
    int i = 0;

    lac_port_get_dbg_cfg(port_index, &port);

    print_sep(&i);
    printf(" port_index      : %d", port.port_index);
    print_sep(&i);
    printf(" lacp_enabled    : %d", port.lacp_enabled);
    print_sep(&i);
    printf(" port_enabled    : %d", port.port_enabled);
    print_sep(&i);
    printf(" rx state        : %d", port.rx->State);
    print_sep(&i);
    printf(" sel state       : %d", port.sel->State);
    print_sep(&i);
    printf(" mux state       : %d", port.mux->State);
    print_sep(&i);
    printf(" tx state        : %d", port.tx->State);
    print_sep(&i);
    printf(" port_moved      : %d", port.port_moved);
    print_sep(&i);
    printf(" static_agg      : %d", port.static_agg);
    print_sep(&i);
    printf(" agg id          : %d", port.agg_id);
    print_sep(&i);
    printf(" selected        : %d", port.selected);
    print_sep(&i);
    printf(" standby         : %d", port.standby);
    print_sep(&i);
    printf(" aport           : %d", port.aport->port_index);
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
    printf(" rcvLacpdu       : %d", port.rcvdLacpdu);
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

int cli_pr_set_speed(int argc, char **argv)
{

    int port_index = atoi(argv[1]);
    int speed = atoi(argv[2]);
    lac_set_port_speed(port_index, speed);
    return 0;

}

int cli_pr_set_duplex(int argc, char **argv)
{

    int port_index = atoi(argv[1]);
    int speed = atoi(argv[2]);
    lac_set_port_duplex(port_index, speed);
    return 0;


}
extern port_attr g_port_list[100];

int cli_pr_get_attr(int argc, char **argv)
{
    int port_loop;
    int port_index;

    int port_start = 0;
    int port_end = max_port;
    if ('a' != argv[1][0])
    {
        port_index = atoi(argv[1]);
        port_start = port_end = port_index;

    }

    printf("port    speed    duplex    rx&tx    chip_tgid");
    for (port_loop = port_start; port_loop < port_end; port_loop++)
    {
        printf("\r\n%-4d     %-4d     %-4d     %-4d     %-4d", port_loop, g_port_list[port_loop].speed, g_port_list[port_loop].duplex,  g_port_list[port_loop].cd, g_port_list[port_loop].tid);
    }

    printf("\r\n");
    return 0;
}

int cli_pr_set_link(int argc, char **argv)
{
    int port_index = atoi(argv[1]);
    int link_status = atoi(argv[2]);
    lac_port_link_change(port_index, link_status);
    return 0;

}

int lac_agg_delete(int argc, char **argv)
{
    int agg_id = atoi(argv[1]);
    _lac_port_lacp_delete_range(0, max_port - 1);
    return 0;
}

int cli_sysget_lac_brief(int argc, char **argv)
{
    int i;
    UID_LAC_CFG_T  uid_cfg;
    int master_index = -1;
    unsigned char actor_sys_id_str[40];
    unsigned char partner_sys_id_str[40];
    unsigned char partner_sys_prio_str[20];
    UID_LAC_PORT_STATE_T uid_port_state[8] = {0};
    unsigned char master_str[10] = {0};
    unsigned char oper_key_str[10] = {0};
    int m_index = -1;
    int sel_cnt = 0 ;
    int standby_cnt = 0;

    lac_sys_get_cfg(&uid_cfg);

    memset(uid_port_state, 0xff, sizeof(UID_LAC_PORT_STATE_T)*8);

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

        if (g_link_groups[i].cnt)
        {
            lac_agg_get_port_state(i+1, uid_port_state, &master_index);

            for (m_index = 0; m_index < g_link_groups[i].cnt; m_index++)
            {
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
           uid_port_state->actor.port_index);
    printf("\r\n          Oper-key        : %-4d(0x%-4x)  Port Priority: %-4d",
           uid_port_state->actor.key,uid_port_state->actor.key, uid_port_state->actor.port_priority);
    printf("\r\n          Port Role       : %-8s      Port Status  : %d",
           role_str[uid_port_state->sel_state], uid_port_state->actor.state);

    printf("\r\n Parnter  System Priority : %-8d      MAC Address  : %s",
           uid_port_state->partner.system_priority, partner_sys_id_str);
    printf("\r\n          Port Priority   : %-8d      Port Number  : %d",
           uid_port_state->partner.port_priority, uid_port_state->partner.port_index);
    printf("\r\n          Oper-key        : %-8d      Port Status  : %d",
           uid_port_state->partner.key, uid_port_state->partner.state);
    printf("\r\n");

    return;
}
int cli_sysget_lac_verbose(int argc, char **argv)
{
    int i;
    UID_LAC_CFG_T  uid_cfg;
    int master_index = -1;
    unsigned char actor_sys_id_str[40];
    UID_LAC_PORT_STATE_T uid_port_state[8] = {0};
    unsigned char master_str[10] = {0};
    int agg_id = atoi(argv[1]);
    lac_sys_get_cfg(&uid_cfg);

    memset(uid_port_state, 0xff, sizeof(UID_LAC_PORT_STATE_T)*8);

    get_mac_str(uid_cfg.sys_mac, actor_sys_id_str);

    printf("\r\n Priority     : %d    MAC Address : %s", uid_cfg.priority, actor_sys_id_str);
    printf("\r\n Short Period : %d    Long Period : %d", uid_cfg.short_period, uid_cfg.long_period);

    sprintf(master_str, "-");

    if (g_link_groups[agg_id-1].cnt)
    {
        lac_agg_get_port_state(agg_id, uid_port_state, &master_index);

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
int cli_sysget_lac_portinfo(int argc, char **argv)
{
    int ret = 0;
    int i;
    UID_LAC_CFG_T  uid_cfg;
    int master_index = -1;
    unsigned char actor_sys_id_str[40];
    UID_LAC_PORT_STATE_T uid_port_state;
    unsigned char master_str[10] = {0};
    int port_index = atoi(argv[1]);

    memset(&uid_port_state, 0, sizeof(UID_LAC_PORT_STATE_T));
    ret = lac_port_get_port_state(port_index, &uid_port_state);
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
int cli_port_clear_stat(int argc, char **argv)
{
    UID_LAC_PORT_CFG_T uid_cfg;
    int port_index = atoi(argv[1]);
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = PT_CFG_STAT;
    BitmapSetBit(&uid_cfg.port_bmp, port_index);
    lac_port_set_cfg(&uid_cfg);
    return 0;
}
int cli_sys_set_prio(int argc, char **argv)
{
    int prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_PRIO;
    uid_cfg.priority = prio;
    lac_sys_set_cfg(&uid_cfg);
}
int cli_sys_set_long_period(int argc, char **argv)
{
    int prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_LONG_PERIOD;
    uid_cfg.long_period = prio;
    lac_sys_set_cfg(&uid_cfg);
    return 0;
}
int cli_sys_set_short_period(int argc, char **argv)
{
    int prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_SHORT_PERIOD;
    uid_cfg.short_period = prio;
    lac_sys_set_cfg(&uid_cfg);
    return 0;
}
int cli_sys_set_period(int argc, char **argv)
{
    int prio = atoi(argv[1]);
    UID_LAC_CFG_T uid_cfg;
    memset(&uid_cfg, 0, sizeof(uid_cfg));
    uid_cfg.field_mask = BR_CFG_PERIOD;
    uid_cfg.period = prio;
    lac_sys_set_cfg(&uid_cfg);
    return 0;
}

static CMD_DSCR_T lang[] = {

    THE_COMMAND("no link-group", "delete the link group")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(lac_agg_delete)

    THE_COMMAND("port link-group", "add port to static lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(lac_port_lacp_enable)

    THE_COMMAND("no port link-group", "disable lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    THE_FUNC(lac_port_lacp_disable)

    THE_COMMAND("show link-group all", "get bridge config")
    THE_FUNC(cli_br_get_cfg)


    THE_COMMAND("show lacp link-group brief", "get bridge config")
    THE_FUNC(cli_sysget_lac_brief)

    THE_COMMAND("show lacp link-group verbose", "get bridge config")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(cli_sysget_lac_verbose)

    THE_COMMAND("show lacp port-info", "add port to static lacp")
    PARAM_NUMBER("port number on bridge", 1, 4, "all")
    THE_FUNC(cli_sysget_lac_portinfo)

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

int stp_cli_init (void)
{
    cli_register_language (lang);
    aggregator_init();

    return 0;
}


