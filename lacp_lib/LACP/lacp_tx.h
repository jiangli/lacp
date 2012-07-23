#ifndef __LACP_TX_H__
#define __LACP_TX_H__

void lac_tx_enter_state (lacp_state_mach_t * this);
Bool lac_tx_check_conditions (lacp_state_mach_t * this);
char* lac_tx_get_state_name (int state);

#endif
