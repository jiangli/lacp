#ifndef __LACP_PORT_H__
#define __LACP_PORT_H__

uint32_t lacp_port_rx_lacpdu (lacp_port_t * port, lacp_pdu_t * bpdu, size_t len);
lacp_port_t *lacp_port_create (lacp_sys_t * lac_sys, uint32_t port_index);
void lacp_port_init(lacp_port_t * port);
void lacp_port_delete (lacp_port_t * port);
uint32_t lacp_port_set_reselect(lacp_port_t *port);
int lacp_dbg_trace_state_machine (lacp_port_t * port, char *mach_name, int enadis);


#endif
