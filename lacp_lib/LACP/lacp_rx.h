#ifndef __LACP_RX_H__
#define __LACP_RX_H__

void lacp_rx_enter_state (lacp_state_mach_t * this);

Bool lacp_rx_check_conditions (lacp_state_mach_t * this);
char* lacp_rx_get_state_name (int state);
int lacp_rxm_rx_lacpdu (lacp_port_t * port, lacp_pdu_t *Lacpdu, int len);


#endif
