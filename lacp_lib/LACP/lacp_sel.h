#ifndef __LACP_SEL_H__
#define __LACP_SEL_H__

void lacp_sel_enter_state (lacp_state_mach_t * fsm);
Bool lacp_sel_check_conditions (lacp_state_mach_t * fsm);
char* lacp_sel_get_state_name (uint32_t state);



#endif
