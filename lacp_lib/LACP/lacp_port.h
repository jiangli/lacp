#ifndef __LACP_PORT_H__
#define __LACP_PORT_H__

uint32_t lacp_port_rx_lacpdu (lacp_port_t * port, lacp_pdu_t * bpdu, size_t len);
uint32_t lacp_set_port_reselect(lacp_port_t *port);
lacp_port_t *lacp_port_create (lacp_sys_t * lac_sys, uint32_t port_index);
void lacp_port_delete (lacp_port_t * port);


#endif
