#ifndef __LAC_IN_H__
#define __LAC_IN_H__

LAC_SYS_T *lac_get_sys_inst (void);
int lac_port_set_cfg(UID_LAC_PORT_CFG_T * uid_cfg);
int lac_port_get_cfg(int port_index, UID_LAC_PORT_CFG_T * uid_cfg);
int lac_rx_lacpdu(int port_index, LACPDU_T * bpdu, int len);
int lac_port_get_dbg_cfg(int port_index, LAC_PORT_T * port);

void
lac_one_second ();
int lac_in_enable_port(int port_index, Bool enable);
int lac_sys_set_cfg(UID_LAC_CFG_T * uid_cfg);
int lac_sys_get_cfg(UID_LAC_CFG_T * uid_cf);
#endif
