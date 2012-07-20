#ifndef __LACP_TRUNK_H__
#define __LACP_TRUNK_H__


uint32_t trunk_port_lacp_enable(uint32_t slot, uint32_t port, uint32_t tid);
uint32_t trunk_port_lacp_disable(uint32_t slot, uint32_t port);
uint32_t trunk_port_get_prio(uint32_t slot, uint32_t port, uint32_t *prio);
uint32_t trunk_port_set_speed(uint32_t slot, uint32_t port, uint32_t speed);
uint32_t trunk_port_set_duplex(uint32_t slot, uint32_t port, uint32_t duplex);
uint32_t trunk_port_link_change(uint32_t slot, uint32_t port, uint32_t link_status);
uint32_t trunk_port_get_lacp_info(uint32_t slot, uint32_t port, UID_LAC_PORT_STATE_T *uid_port_state);
uint32_t trunk_port_clear_stat(uint32_t slot, uint32_t port);
uint32_t trunk_sys_set_prio(uint32_t prio);
uint32_t trunk_sys_set_long_period(uint32_t period);
uint32_t trunk_sys_set_short_period(uint32_t period);
uint32_t trunk_sys_set_period(uint32_t is_short);
uint32_t trunk_port_set_prio(uint32_t slot, uint32_t port, uint32_t prio);
#endif
