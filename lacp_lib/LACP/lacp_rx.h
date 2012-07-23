#ifndef __LACP_RX_H__
#define __LACP_RX_H__

void lacp_rx_enter_state (lacp_state_mach_t * fsm);

Bool lacp_rx_check_conditions (lacp_state_mach_t * fsm);
char* lacp_rx_get_state_name (uint32_t state);
uint32_t lacp_rxm_rx_lacpdu (lacp_port_t * port, lacp_pdu_t *Lacpdu, uint32_t len);


#endif
