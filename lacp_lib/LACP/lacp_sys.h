#ifndef __LACP_SYS_H__
#define __LACP_SYS_H__

#define LACP_SYS_UPDATE_READON_RX 1
#define LACP_SYS_UPDATE_READON_TIMER 2
#define LACP_SYS_UPDATE_READON_PORT_CFG 3
#define LACP_SYS_UPDATE_READON_SYS_CFG 4
#define LACP_SYS_UPDATE_READON_LINK 5
#define LACP_SYS_UPDATE_READON_PORT_CREATE 6
lacp_sys_t *lacp_get_sys_inst (void);

uint32_t lacp_sys_update(lacp_sys_t *sys, uint32_t reason);
lacp_sys_t *lacp_sys_init ();

#endif
