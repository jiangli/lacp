#ifndef __LAC_RX_H__
#define __LAC_RX_H__

void lac_rx_enter_state (LAC_STATE_MACH_T * this);

Bool
STP_transmit_check_conditions (LAC_STATE_MACH_T* s);

Bool lac_rx_check_conditions (LAC_STATE_MACH_T * this);
char* lac_rx_get_state_name (int state);
int lac_rxm_rx_lacpdu (LAC_PORT_T * port, LACPDU_T *Lacpdu, int len);


#endif
