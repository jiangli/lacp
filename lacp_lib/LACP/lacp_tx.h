#ifndef __LACP_TX_H__
#define __LACP_TX_H__

void lacp_tx_enter_state (lacp_state_mach_t * fsm);
Bool lacp_tx_check_conditions (lacp_state_mach_t * fsm);
char* lacp_tx_get_state_name (uint32_t state);

#endif
