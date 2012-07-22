#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_util.h"
#include "lacp_stub.h"

void lacp_memdump(unsigned char *buf, uint32_t len)
{
    uint32_t i = 0;
    printf("\r\n buf:0x%x, len:%d\r\n---------\r\n", (uint32_t)buf, len);
    for (i = 0; i < len; i++)
    {
        if (i%16 == 0)
            printf("\r\n");
        printf(" %02x", *(buf + i));
    }

    printf("\r\n---------\r\n");

}

void lacp_get_mac_str (unsigned char *addr, char *str)
{
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
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

int lacp_get_bit_str (char *buf, unsigned char bitmask,
                      char *bit_name, char *bit_fmt, unsigned char flags)
{
    char * p = buf;
    uint32_t the_bit = (flags & bitmask) ? 1 : 0;

    p += sprintf (p, "    ");
    p += sprintf (p, bit_fmt, the_bit);
    p += sprintf (p, " %-20s  %s\n", bit_name, the_bit ? "- yes" : "");

    return (p-buf);
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

int lacp_print_port_info(char *buf, lacp_port_info_t *lacp_info)
{
    char *ptr  = buf;
    unsigned char sysid_str[40] = {0};

    lacp_get_sysid_str(lacp_info->system_priority, lacp_info->system_mac, sysid_str);

    ptr += sprintf(ptr, "\r\n System ID:%s", sysid_str);
    ptr += sprintf(ptr, "\r\n port     :%d", lacp_info->port_no);
    ptr += sprintf(ptr, "\t priority :%d", lacp_info->port_priority);
    ptr += sprintf(ptr, "\t key      :%d", lacp_info->key);
    ptr += sprintf(ptr, "\r\n state    \r\n");
    ptr += lacp_get_bit_str(ptr,LACP_STATE_ACT, "LACP_Activity", "%d.......", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_TMT, "LACP_Timeout", ".%d......", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_AGG, "Aggregation", "..%d.....", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_SYN, "Synchronization", "...%d....", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_COL, "Collecting", "....%d...", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_DIS, "Distributing", ".....%d..", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_DEF, "Defaulted", "......%d.", lacp_info->state);
    ptr += lacp_get_bit_str(ptr,LACP_STATE_EXP, "Expired", ".......%d", lacp_info->state);

    return (buf-ptr);

}


void lacp_dump_pkt(lacp_pdu_t *pkt, uint32_t len)
{
    char show_str[LACP_MAC_PORT_INFO_LEN + 1] = {0};
    lacp_port_info_t actor, partner;

    lacp_memdump((unsigned char *)pkt, len);

    lacp_copy_info_from_net(&pkt->actor, &actor);
    lacp_copy_info_from_net(&pkt->partner, &partner);
    lacp_print_port_info(show_str, &actor);
    printf("%s", show_str);
    lacp_print_port_info(show_str, &partner);
    printf("%s", show_str);
}

