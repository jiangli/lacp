
#ifndef __LACP_UTIL_H__
#define __LACP_UTIL_H__


#define LACP_PORT_INFO_LEN 1000

void lacp_get_mac_str (unsigned char *addr, char *str);
void lacp_get_sysid_str (uint32_t prio, unsigned char *addr, unsigned char *str);

int lacp_get_bit_str (char *buf, unsigned char bitmask,                        char *bit_name, char *bit_fmt, unsigned char flags);
void lacp_copy_info(lacp_port_info_t *from, lacp_port_info_t *to);

void lacp_copy_info_from_net(lacp_port_info_t *from, lacp_port_info_t *to);
int lacp_print_port_info(char *buf, lacp_port_info_t *lacp_info);
void lacp_dump_pkt(lacp_pdu_t *pkt, uint32_t len);

void write_log(char *filename, int line, int ret,
               int para1, int para2, int para3);

#define ERR_LOG(ret,para1,para2,para3) \
        write_log(__FILE__, __LINE__, ret, para1, para2, para3);
int cli_pr_get_attr(int argc, char **argv);
#endif
