
#ifndef __LACP_UTIL_H__
#define __LACP_UTIL_H__



void lacp_get_mac_str (unsigned char *addr, unsigned char *str);
void lacp_get_sysid_str (uint32_t prio, unsigned char *addr, unsigned char *str);

void _lacp_display_bit (unsigned char bitmask,                      char *bit_name, char *bit_fmt, unsigned char flags);
void lacp_copy_info(lacp_port_info_t *from, lacp_port_info_t *to);

void lacp_copy_info_from_net(lacp_port_info_t *from, lacp_port_info_t *to);
void lacp_print_port_info(lacp_port_info_t *lacp_info);
void lacp_dump_pkt(lacp_pdu_t *pkt, int len);

#define ERR_LOG(ret,para1,para2,para3) \
        write_log(__FILE__, __LINE__, ret, para1, para2, para3);

#endif
