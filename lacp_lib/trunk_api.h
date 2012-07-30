#ifndef __LACP_TRUNK_H__
#define __LACP_TRUNK_H__

#define TRUNK_ID_MAX 32
#define TRUNK_UINT_INVALID 0xffffffff
#define TRUNK_PORT_NUMBER_MAX 8
typedef struct trunk_sys_cfg_s
{
    uint32_t prio;
    uint32_t long_period;
    uint32_t short_period;
    uint32_t period;

} trunk_sys_cfg_t;
typedef struct trunk_sys_state_s
{
    lacp_mac_t mac;
} trunk_sys_state_t;

typedef struct {
    uint32_t valid;
    uint32_t slot;
    uint32_t port;
    lacp_key_t key;
    uint32_t agg_id;
    uint32_t master_port;
    uint32_t sel_state;  /* selected or standby. readonly */
    uint32_t rx_cnt;
    uint32_t tx_cnt;
    lacp_port_info_t        actor;
    lacp_port_info_t        partner;

} trunk_port_state_t;

typedef struct {
    uint32_t master_index;
    trunk_port_state_t ports_state[8];
} trunk_agg_state_t;

uint32_t trunk_sys_set_prio(uint32_t prio);
uint32_t trunk_sys_get_cfg(trunk_sys_cfg_t *cfg);
uint32_t trunk_port_lacp_enable(uint32_t slot, uint32_t port, uint32_t tid);
uint32_t trunk_port_lacp_disable(uint32_t slot, uint32_t port);
uint32_t trunk_port_get_prio(uint32_t slot, uint32_t port, uint32_t *prio);
uint32_t trunk_port_set_speed(uint32_t slot, uint32_t port, uint32_t speed);
uint32_t trunk_port_set_duplex(uint32_t slot, uint32_t port, uint32_t duplex);
uint32_t trunk_port_link_change(uint32_t slot, uint32_t port, uint32_t link_status);
uint32_t trunk_port_get_lacp_info(uint32_t slot, uint32_t port, trunk_port_state_t *uid_port_state);
uint32_t trunk_port_clear_stat(uint32_t slot, uint32_t port);

uint32_t trunk_sys_set_long_period(uint32_t period);
uint32_t trunk_sys_set_short_period(uint32_t period);
uint32_t trunk_sys_set_period(uint32_t is_short);
uint32_t trunk_port_set_prio(uint32_t slot, uint32_t port, uint32_t prio);
uint32_t trunk_agg_delete(uint32_t agg_i);
uint32_t trunk_sys_set_cfg(lacp_sys_cfg_t * uid_cfg);

uint32_t trunk_sys_get_state(trunk_sys_state_t *cfg);
uint32_t trunk_port_state_sort(trunk_port_state_t *uid_port_state);
uint32_t trunk_agg_get_state(int agg_id, trunk_agg_state_t *agg_state);

#endif
