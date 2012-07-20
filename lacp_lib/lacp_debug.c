#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_stub.h"
static uint32_t g_lacp_debug_rx_tx[18][8][2];

void lacp_dbg_pkt(int slot, int port, int direction, int en)
{        
        g_lacp_debug_rx_tx[slot][port-1][direction] = en;
}
int lacp_dbg_get_switch(int slot, int port,int direction)
{
        return g_lacp_debug_rx_tx[slot][port-1][direction];
}

void
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

void
_lacp_in_display_bit (unsigned char bitmask,
                      char *bit_name, char *bit_fmt, unsigned char flags)
{
    uint32_t the_bit = (flags & bitmask) ? 1 : 0;

    printf ("    ");
    printf (bit_fmt, the_bit);
    printf (" %-20s  %s\n", bit_name, the_bit ? "- yes" : "");
}

void print_info(lacp_port_info_t *lacp_info)
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

void dump_pkt(lacp_pdu_t *pkt, int len)
{
        memdump(pkt, len);
        print_info(&pkt->actor);
        print_info(&pkt->partner);
}
uint32_t lacp_dbg_trace(uint32_t port_index, char *state_name, Bool on)
{
        lacp_port_t *port;
        port = _lacp_port_find(port_index);
        if (!port)
                return 0;
        lacp_dbg_trace_state_machine(port, state_name, on);
}
