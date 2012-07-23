#ifndef __LACP_PORT_H__
#define __LACP_PORT_H__

unsigned int lacp_port_rx_lacpdu (lacp_port_t * this, lacp_pdu_t * bpdu, size_t len);
unsigned int lacp_set_port_reselect(lacp_port_t *port);
lacp_port_t *lacp_port_create (lacp_sys_t * lac_sys, int port_index);
void lacp_port_delete (lacp_port_t * this);


#endif
