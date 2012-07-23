
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
#include "trunk_ssp.h"
#include "lacp_util.h"
#include "lacp_stub.h"
#include "trunk_api.h"

extern  uint32_t stub_init();

#define TRUNK_MAC_STR_LEN 20
#define TRUNK_PRIO_STR_LEN 10
#define TRUNK_PORT_STR_LEN 6
#define TRUNK_KEY_STR_LEN 6
#define TRUNK_STATUS_STR_LEN 100

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
static int lacp_port_lacp_enable (int argc, char** argv)
{
    uint32_t ret = 0;
    uint32_t slot;
    uint32_t port;
    uint32_t agg_id = (uint32_t)atoi(argv[2]);

    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int *) &port);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    if (!agg_id || agg_id > TRUNK_ID_MAX)
    {
        ERR_LOG(ret, agg_id, slot, port);
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
static int lacp_port_lacp_disable (int argc, char** argv)
{
    uint32_t ret = 0;
    uint32_t slot;
    uint32_t port;

    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret != 0)
    {
        ERR_LOG(ret, 0, 0, 0);
        return ret;
    }

    ret = trunk_port_lacp_disable(slot, port);
    if (ret != 0)
    {
        ERR_LOG(ret, slot, port, 0);
        return ret;
    }
	
    return 0;
}

void lacp_debug_print_sep(uint32_t *i)
{
    if ((*i)%3 == 0)
        printf("\r\n");
    else
        printf("\t");
    (*i)++;

}

static int cli_pr_get_cfg (int argc, char** argv)
{
    uint32_t ret = 0;
    uint32_t i = 0;
    uint32_t slot, port;
    uint32_t port_index ;
    lacp_port_t lacp_port;
    char show_str[LACP_PORT_INFO_LEN + 1] = {0};

    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return 0;
    }

    trunk_ssp_get_global_index(slot, port, &port_index);

    ret = lacp_port_get_dbg_cfg(port_index, &lacp_port);
    if (ret != 0)
    {
        printf("can't display port %d.\r\n", port_index);
        return 0;
    }

    lacp_debug_print_sep(&i);
    printf(" port            : %d/%d", slot, port);
    lacp_debug_print_sep(&i);
    printf(" port_index      : %d", port_index);
    lacp_debug_print_sep(&i);
    printf(" lacp_enabled    : %d", lacp_port.lacp_enabled);
    lacp_debug_print_sep(&i);
    printf(" port_enabled    : %d", lacp_port.port_enabled);
    lacp_debug_print_sep(&i);
    printf(" rx state        : %d", lacp_port.rx->state);
    lacp_debug_print_sep(&i);
    printf(" sel state       : %d", lacp_port.sel->state);
    lacp_debug_print_sep(&i);
    printf(" mux state       : %d", lacp_port.mux->state);
    lacp_debug_print_sep(&i);
    printf(" tx state        : %d", lacp_port.tx->state);
    lacp_debug_print_sep(&i);
    printf(" port_moved      : %d", lacp_port.port_moved);
    lacp_debug_print_sep(&i);
    printf(" agg id          : %d", lacp_port.agg_id);
    lacp_debug_print_sep(&i);
    printf(" selected        : %d", lacp_port.selected);
    lacp_debug_print_sep(&i);
    printf(" standby         : %d", lacp_port.standby);
    lacp_debug_print_sep(&i);
    printf(" aport           : %d", lacp_port.master_port->port_index);
    lacp_debug_print_sep(&i);
    printf(" ntt             : %d", lacp_port.ntt);
    lacp_debug_print_sep(&i);
    printf(" hold_count      : %d", lacp_port.hold_count);
    lacp_debug_print_sep(&i);
    printf(" ready_n         : %d", lacp_port.ready_n);
    lacp_debug_print_sep(&i);
    printf(" speed           : %d", lacp_port.speed);
    lacp_debug_print_sep(&i);
    printf(" duplex          : %d", lacp_port.duplex);
    lacp_debug_print_sep(&i);
    printf(" rcvLacpdu       : %d", lacp_port.rcvd_lacpdu);
    lacp_debug_print_sep(&i);
    printf(" tx_cnt          : %d", lacp_port.tx_lacpdu_cnt);
    lacp_debug_print_sep(&i);
    printf(" rx_cnt          : %d", lacp_port.rx_lacpdu_cnt);
    lacp_debug_print_sep(&i);
    printf(" periodic_while  : %d", lacp_port.periodic_timer);
    lacp_debug_print_sep(&i);
    printf(" current_while   : %d", lacp_port.current_while);
    lacp_debug_print_sep(&i);
    printf(" wait_while      : %d", lacp_port.wait_while);
    lacp_debug_print_sep(&i);

    printf("\r\n actor:" );
    lacp_print_port_info(show_str, &lacp_port.actor);
    printf("%s", show_str);

    printf("\r\n parttor: " );
    lacp_print_port_info(show_str, &lacp_port.partner);
    printf("%s", show_str);
#if 1
    printf("\r\n msg actor: " );
    lacp_print_port_info(show_str, &lacp_port.msg_actor);
    printf("%s", show_str);

    printf("\r\n msg parttor: " );
    lacp_print_port_info(show_str, &lacp_port.msg_partner);
    printf("%s", show_str);
#endif
    return 0;

}

int cli_pr_set_speed(int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t speed = (uint32_t)atoi(argv[2]);
	
    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_set_speed(slot, port, speed);
    return 0;

}

int cli_pr_set_duplex(int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t speed = atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_set_duplex(slot, port, speed);
    return 0;


}


int cli_pr_set_link(int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t link_status = atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_port_link_change(slot, port, link_status);
    return 0;

}

int lacp_agg_delete(int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t agg_id = (uint32_t)atoi(argv[1]);
	
    ret = trunk_agg_delete(agg_id);
    if (ret != 0)
    {
        ERR_LOG(ret, agg_id, 0, 0);
        return 0;
    }

    return 0;
}

int cli_print_sys_header()
{
    uint32_t ret = 0;
    trunk_sys_cfg_t  sys_cfg;
    trunk_sys_state_t  sys_state;
    char sys_mac_str[TRUNK_MAC_STR_LEN + 1] = {0};

    ret = trunk_sys_get_cfg(&sys_cfg);
	if (ret != 0)
	{		
		return ret;
	}
	
    ret = trunk_sys_get_state(&sys_state);
	if (ret != 0)
	{		
		return ret;
	}
	
    lacp_get_mac_str(sys_state.mac, sys_mac_str);

    printf("\r\n  System Priority : %d", sys_cfg.prio);
    printf("\r\n  MAC Address     : %s", sys_mac_str);
    printf("\r\n  Short Period    : %-3ds", sys_cfg.short_period);
    printf("\r\n  Long Period     : %-3ds", sys_cfg.long_period);
    printf("\r\n  Timeout         : %s Timeout", sys_cfg.period ? "Short" : "Long");
	
    return 0;
}

int cli_sys_get_lacp_brief(int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t agg_index;
    uint32_t master_index = TRUNK_UINT_INVALID;
    trunk_agg_state_t agg_state;
    uint32_t port_loop;
    uint32_t selected_cnt = 0 ;
    uint32_t standby_cnt = 0;
    uint32_t master_slot;
	uint32_t master_port;
    char partner_sys_mac_str[TRUNK_MAC_STR_LEN + 1] = {0};
    char partner_sys_prio_str[TRUNK_PRIO_STR_LEN + 1] = {0};
    char master_port_str[TRUNK_PORT_STR_LEN + 1] = {0};
    char agg_oper_key_str[TRUNK_KEY_STR_LEN + 1] = {0};

    cli_print_sys_header();
    memset(&agg_state, 0, sizeof(agg_state));

    printf("\r\n ----------------------------------------------------------------------");
    printf("\r\n Agg  Partner  Parnter            Master   Selected   Standby   Oper ");
    printf("\r\n ID   Pri      MAC                Port     PortNum    PortNum   Key  ");
    printf("\r\n ----------------------------------------------------------------------");
	
    for (agg_index = 0; agg_index < TRUNK_ID_MAX; agg_index++)
    {
		/* ³õÊ¼»¯ */
        selected_cnt = standby_cnt = 0;
        master_index = TRUNK_UINT_INVALID;
        sprintf(master_port_str, "-");
        sprintf(partner_sys_mac_str, "-");
        sprintf(partner_sys_prio_str, "-");
        sprintf(agg_oper_key_str, "-");

        if (stub_db_agg_has_member(agg_index + 1))
        {
            ret = trunk_agg_get_state(agg_index + 1, &agg_state);
			if (ret != 0)
			{		
				continue;
			}

            for (port_loop = 0; port_loop < TRUNK_PORT_NUMBER_MAX; port_loop++)
            {
                if (!agg_state.ports_state[port_loop].valid)
                    continue;

                if (agg_state.ports_state[port_loop].sel_state)
                {
                    selected_cnt++;
                }
                else
                {
                    standby_cnt++;
                }
            }
			
            master_index = agg_state.master_index;

            if (master_index != TRUNK_UINT_INVALID)
            {
                master_slot = agg_state.ports_state[master_index].slot;
                master_port = agg_state.ports_state[master_index].port;
                sprintf(master_port_str, "%d/%d", master_slot, master_port);
                sprintf(partner_sys_prio_str, "%d", agg_state.ports_state[master_index].partner.system_priority);
                lacp_get_mac_str(agg_state.ports_state[master_index].partner.system_mac, partner_sys_mac_str);
                sprintf(agg_oper_key_str, "%d", agg_state.ports_state[master_index].actor.key);
            }

            printf("\r\n %-5d%-9s%-19s%-9s%-11d%-10d%-3s",
                   (agg_index + 1), partner_sys_prio_str, partner_sys_mac_str, master_port_str, selected_cnt, standby_cnt, agg_oper_key_str);
        }

    }
	
    printf("\r\n ----------------------------------------------------------------------\r\n");
    return 0;
}

void cli_get_status_str(uchar_t status, char *str)
{
    char *col_str[] = {"Non-Collecting", "Collecting"};
    char *dis_str[] = {"Non-Distributing", "Distributing"};

    sprintf(str, "%s & %s", col_str[LACP_STATE_GET_BIT(status, LACP_STATE_COL)], dis_str[LACP_STATE_GET_BIT(status, LACP_STATE_DIS)]);

    return;

}
void cli_print_port_info(trunk_port_state_t *port_state)
{
    char partner_sys_mac_str[TRUNK_MAC_STR_LEN + 1] = {0};
    char status_str[TRUNK_STATUS_STR_LEN + 1] = {0};
    char *role_str[] = {"STANDBY", "SELECTED"};
    uint32_t slot, port;

    sprintf(partner_sys_mac_str, "-");
    cli_get_status_str(port_state->actor.state, status_str);

    lacp_get_mac_str(port_state->partner.system_mac, partner_sys_mac_str);

    lacp_ssp_change_to_slot_port(port_state->actor.port_no - 1, &slot, &port);

    printf("\r\n Actor    Port            : %-2d/%-5d  Port Role       : %s",
           slot, port, role_str[port_state->sel_state]);
    printf("\r\n          Port Priority   : %-8d  Port Number     : %d",
           port_state->actor.port_priority, port_state->actor.port_no);
    printf("\r\n          Oper-key        : %-8d  Port Status     : %s",
           port_state->actor.key, status_str);

    cli_get_status_str(port_state->partner.state, status_str);

    printf("\r\n Parnter  System Priority : %-8d  MAC Address     : %s",
           port_state->partner.system_priority, partner_sys_mac_str);
    printf("\r\n          Port Priority   : %-8d  Port Number     : %d",
           port_state->partner.port_priority, port_state->partner.port_no);
    printf("\r\n          Oper-key        : %-8d  Port Status     : %s",
           port_state->partner.key, status_str);
    printf("\r\n");

    return;
}
int cli_sys_get_lacp_verbose(int argc, char **argv)
{
    uint32_t ret = 0;
	uint32_t port_loop;
    uint32_t master_index = TRUNK_UINT_INVALID;
    trunk_agg_state_t agg_state;
    uint32_t agg_id = (uint32_t)atoi(argv[1]);
    char master_str[TRUNK_PORT_STR_LEN + 1] = {0};

    //TODO:: agg not exist

    memset(&agg_state, 0, sizeof(agg_state));

    cli_print_sys_header();

    sprintf(master_str, "-");

    if (stub_db_agg_has_member(agg_id))
    {
        ret = trunk_agg_get_state(agg_id, &agg_state);
		if (ret != 0)
		{
			return ret;
		}
		
        master_index = agg_state.master_index;
        if (master_index != TRUNK_UINT_INVALID)
        {
            sprintf(master_str, "%d/%d", agg_state.ports_state[master_index].slot, agg_state.ports_state[master_index].port);
        }

        printf("\r\n  Agg Master Port : %s", master_str);
        printf("\r\n");
        trunk_port_state_sort(agg_state.ports_state);
        for (port_loop = 0; port_loop < TRUNK_PORT_NUMBER_MAX; port_loop++)
        {
            if (!agg_state.ports_state[port_loop].valid)
                continue;

            cli_print_port_info(&agg_state.ports_state[port_loop]);
        }

    }
    return 0;
}

int cli_sys_get_lacp_portinfo(int argc, char **argv)
{
    uint32_t ret = 0;
    char actor_sys_mac_str[TRUNK_MAC_STR_LEN + 1] = {0};
    trunk_port_state_t port_state;
    char master_str[TRUNK_PORT_STR_LEN + 1] = {0};
    uint32_t slot, port;
    uint32_t slot_master, port_master;
    uint32_t agg_id;
    uint32_t prio;

    ret = DEV_GetIfPonFromIfnet(argv[1],(int *) &slot,(int *) &port);
	if (ret != 0)
	{
		return ret;
	}
	
    agg_id = stub_db_agg_get_port_tid(slot, port);

    ret = trunk_port_get_prio(slot, port, &prio);
	if (ret != 0)
	{
		return ret;
	}
	
    printf("\r\n Lacp Enable     : %-8s  Port Priority: %d", (agg_id == 0)? "disable" : "enable", prio);
    if (agg_id == 0)
            return;
	
    memset(&port_state, 0, sizeof(trunk_port_state_t));
	
    ret = trunk_port_get_lacp_info(slot, port, &port_state);
    if ( 0 != ret || 0 == port_state.agg_id)
    {
        printf("\r\n port is not in lacp aggregation gruop. ret:%d, agg:%d", ret, port_state.agg_id);
        printf("\r\n");
        return 0;
    }

    lacp_get_mac_str(port_state.actor.system_mac, actor_sys_mac_str);
    lacp_ssp_change_to_slot_port(port_state.master_port, &slot_master, &port_master);
    sprintf(master_str, "%d/%d", slot_master, port_master);

    printf("\r\n Selected AggID  : %-8d  Master Port  : %s", port_state.agg_id, master_str);
    printf("\r\n System Priority : %-8d  MAC Address  : %s", port_state.actor.system_priority, actor_sys_mac_str);
    printf("\r\n Received LACPDU : %-8d  Sent LACPDU  : %d", port_state.rx_cnt, port_state.tx_cnt);
    printf("\r\n");
    cli_print_port_info(&port_state);
    return 0;
}

int cli_port_clear_stat(int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;

    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return ret;
    }

    ret = trunk_port_clear_stat(slot, port);
	if (ret != 0)
	{
		return ret;
	}
    return 0;
}

int cli_sys_set_prio(int argc, char **argv)
{
    uint32_t prio = (uint32_t)atoi(argv[1]);
    return trunk_sys_set_prio(prio);

}

int cli_sys_set_long_period(int argc, char **argv)
{
    uint32_t long_period = (uint32_t)atoi(argv[1]);
    return trunk_sys_set_long_period(long_period);

}
int cli_sys_set_short_period(int argc, char **argv)
{
    uint32_t short_period = (uint32_t)atoi(argv[1]);
    return trunk_sys_set_short_period(short_period);
}

int cli_sys_set_period(int argc, char **argv)
{
    uint32_t period = (uint32_t)atoi(argv[1]);
    return trunk_sys_set_period(period);
}

int cli_debug_trace (int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t port_index;

    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_ssp_get_global_index(slot, port, &port_index);

    lacp_dbg_trace(port_index, argv[2], argv[3][0] != 'n' && argv[3][0] != 'N');
    return 0;
}

int cli_debug_pkt (int argc, char **argv)
{
    uint32_t ret = 0;
    uint32_t slot, port;

    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
    if (ret !=0 )
    {
        return ret;
    }

    trunk_dbg_pkt(slot, port, argv[2][0] == 't', argv[3][0] != 'n' && argv[3][0] != 'N');

    return 0;
}

int cli_port_set_prio(int argc, char **argv)
{

    uint32_t ret = 0;
    uint32_t slot, port;
    uint32_t prio = (uint32_t)atoi(argv[2]);
    ret = DEV_GetIfPonFromIfnet(argv[1], (int *)&slot,(int*) &port);
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


    THE_COMMAND("show lacp link-group brief", "get bridge config")
    THE_FUNC(cli_sys_get_lacp_brief)

    THE_COMMAND("show lacp link-group verbose", "get bridge config")
    PARAM_NUMBER("agg group on bridge", 1, 32, "1")
    THE_FUNC(cli_sys_get_lacp_verbose)

    THE_COMMAND("show lacp port-info", "add port to static lacp")
    PARAM_STRING("slot/port", "0/1")
    THE_FUNC(cli_sys_get_lacp_portinfo)

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

int lacp_cli_init (void)
{
    cli_register_language (lang);
    stub_init();

    return 0;
}


