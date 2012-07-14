#ifndef __LAC_SEL_H__
#define __LAC_SEL_H__

void lac_sel_enter_state (LAC_STATE_MACH_T * this);
Bool lac_sel_check_conditions (LAC_STATE_MACH_T * this);
char* lac_sel_get_state_name (int state);



#endif
