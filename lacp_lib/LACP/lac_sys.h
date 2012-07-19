#ifndef __LAC_SYS_H__
#define __LAC_SYS_H__

#define LAC_SYS_UPDATE_READON_RX 1
#define LAC_SYS_UPDATE_READON_TIMER 2
#define LAC_SYS_UPDATE_READON_PORT_CFG 3
#define LAC_SYS_UPDATE_READON_SYS_CFG 4
#define LAC_SYS_UPDATE_READON_LINK 5
#define LAC_SYS_UPDATE_READON_PORT_CREATE 6
LAC_SYS_T *lac_get_sys_inst (void);

int lac_sys_update(LAC_SYS_T *this, int reason);
LAC_SYS_T *lac_sys_init ();

#endif
