#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_util.h"
#include "lacp_stub.h"

void lacp_get_mac_str (unsigned char *addr, unsigned char *str)
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

void lacp_get_sysid_str (uint32_t prio, unsigned char *addr, unsigned char *str)
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

void _lacp_display_bit (unsigned char bitmask,
                      char *bit_name, char *bit_fmt, unsigned char flags)
{
    uint32_t the_bit = (flags & bitmask) ? 1 : 0;

    printf ("    ");
    printf (bit_fmt, the_bit);
    printf (" %-20s  %s\n", bit_name, the_bit ? "- yes" : "");
}

void lacp_copy_info(lacp_port_info_t *from, lacp_port_info_t *to)
{
    to->port_priority		  = from->port_priority;
    to->port_no            = from->port_no;
    to->system_priority 	  = from->system_priority;
    memcpy(to->system_mac, from->system_mac, 6);
    to->key 				  = from->key;
    to->state                 = from->state;
}

void lacp_copy_info_from_net(lacp_port_info_t *from, lacp_port_info_t *to)
{
    to->port_priority		  = ntohs(from->port_priority);
    to->port_no            = ntohs(from->port_no);
    to->system_priority 	  = ntohs(from->system_priority);
    memcpy(to->system_mac, from->system_mac, 6);
    to->key 				  = ntohs(from->key);
    to->state		          = from->state;
}

void lacp_print_port_info(lacp_port_info_t *lacp_info)
{
    unsigned char sysid_str[40] = {0};

    lacp_get_sysid_str(lacp_info->system_priority, lacp_info->system_mac, sysid_str);

    printf("\r\n System ID:%s", sysid_str);
    printf("\r\n port     :%d", lacp_info->port_no);
    printf("\t priority :%d", lacp_info->port_priority);
    printf("\t key      :%d", lacp_info->key);
    printf("\r\n state    \r\n");
    _lacp_display_bit(LACP_STATE_ACT, "LACP_Activity", "%d.......", lacp_info->state);
    _lacp_display_bit(LACP_STATE_TMT, "LACP_Timeout", ".%d......", lacp_info->state);
    _lacp_display_bit(LACP_STATE_AGG, "Aggregation", "..%d.....", lacp_info->state);
    _lacp_display_bit(LACP_STATE_SYN, "Synchronization", "...%d....", lacp_info->state);
    _lacp_display_bit(LACP_STATE_COL, "Collecting", "....%d...", lacp_info->state);
    _lacp_display_bit(LACP_STATE_DIS, "Distributing", ".....%d..", lacp_info->state);
    _lacp_display_bit(LACP_STATE_DEF, "Defaulted", "......%d.", lacp_info->state);
    _lacp_display_bit(LACP_STATE_EXP, "Expired", ".......%d", lacp_info->state);

    return ;

}

void lacp_dump_pkt(lacp_pdu_t *pkt, int len)
{
        lacp_port_info_t actor, partner;

        memdump(pkt, len);
        
        lacp_copy_info_from_net(&pkt->actor, &actor);
        lacp_copy_info_from_net(&pkt->partner, &partner);
        lacp_print_port_info(&actor);
        lacp_print_port_info(&partner);
}

