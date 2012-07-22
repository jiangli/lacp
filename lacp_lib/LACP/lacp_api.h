#ifndef   __LACP_API_H__
#define   __LACP_API_H__

#define LACP_UINT_INVALID 0xffffffff

enum {
    Slow_periodic_ticks   =   30,/* should be 30, faster for simulation */
    Fast_periodic_ticks   =   1,
    Long_timeout_ticks    =   90,/* should be 90, faster for simulation */
    Short_timeout_ticks   =   3,
    Churn_detection_ticks =  60,
    Aggregate_wait_ticks  =   2,
};

enum {
    Max_tx_per_interval  =   3
};

enum {/*
       * Default settings for system characteristics.
       */
    Default_system_priority = 32768,
    Default_port_priority   = 16384,
    Default_key             = 0,
    Default_lacp_activity   = True,
    Default_lacp_timeout    = LACP_LONG_TIMEOUT,
    Default_aggregation     = False
};


typedef struct {
    /* service data */
    uint32_t field_mask; /* which fields to change */
    lacp_bitmap_t port_bmp;

    /* protocol data */
    Bool 			 lacp_enabled;
    lacp_key_t		 key;
    lacp_port_prio_t port_priority;
    lacp_state_t 	 state;
    uint32_t		 agg_id;
    lacp_port_info_t partner;

} lacp_port_cfg_t;


typedef struct {
    uint32_t valid;
    uint32_t port_index;
    lacp_key_t key;
    uint32_t agg_id;
    uint32_t master_port;
    uint32_t sel_state;  /* selected or standby. readonly */
    uint32_t rx_cnt;
    uint32_t tx_cnt;
    lacp_port_info_t        actor;
    lacp_port_info_t        partner;

} lacp_port_state_t;

#define BR_CFG_PBMP_ADD         (1L << 0)
#define BR_CFG_PBMP_DEL         (1L << 1)
#define BR_CFG_PRIO         	(1L << 2)
#define BR_CFG_LONG_PERIOD         	(1L << 3)
#define BR_CFG_SHORT_PERIOD         	(1L << 4)
#define BR_CFG_PERIOD         	(1L << 5)


#define PT_CFG_STATE    (1L << 0)
#define PT_CFG_COST     (1L << 1)
#define PT_CFG_PRIO     (1L << 2)
#define PT_CFG_STAT     (1L << 3)

typedef struct {
    /* service data */
    uint32_t	field_mask; /* which fields to change */
    uint32_t	number_of_ports;
    lacp_bitmap_t	ports;
    uint32_t        period;
    /* protocol data */
    //lacp_sys_prio_t priority;
    uint32_t priority;
    lacp_mac_t 		sys_mac;
    uint32_t 	short_period;
    uint32_t 	long_period;
} lacp_sys_cfg_t;

typedef struct {
    lacp_mac_t mac;

} lacp_sys_state_t;

#ifdef __LINUX__
#  define LACP_INIT_CRITICAL_PATH_PROTECTIO
#  define LACP_CRITICAL_PATH_START
#  define LACP_CRITICAL_PATH_END
#else
#  define LACP_INIT_CRITICAL_PATH_PROTECTIO  lacp_out_init_sem();
#  define LACP_CRITICAL_PATH_START			lacp_out_sem_take();
#  define LACP_CRITICAL_PATH_END			lacp_out_sem_give();
#endif


typedef enum {
    M_LACP_INTERNEL = 1,
    M_LACP_NOT_ENABLE,
    M_LACP_NOT_FOUND,
    M_LACP_NOT_CREATED,
    M_RSTP_PORT_IS_ABSENT,
    M_RSTP_NOT_ENABLE,
} LACP_ERROR_E;

uint32_t lacp_port_get_actor_init(uint32_t port_index, lacp_port_info_t  *admin);
uint32_t lacp_port_get_partner_init(uint32_t port_index, lacp_port_info_t  *admin);
lacp_sys_t *lacp_get_sys_inst (void);
uint32_t lacp_port_set_cfg(lacp_port_cfg_t * uid_cfg);
uint32_t lacp_port_get_cfg(uint32_t port_index, lacp_port_cfg_t * uid_cfg);
uint32_t lacp_rx_lacpdu(uint32_t port_index, lacp_pdu_t * bpdu, uint32_t len);
uint32_t lacp_port_get_dbg_cfg(uint32_t port_index, lacp_port_t * port);

void
lacp_one_second ();
uint32_t lacp_in_enable_port(uint32_t port_index, Bool enable);
uint32_t lacp_sys_set_cfg(lacp_sys_cfg_t * uid_cfg);
uint32_t lacp_sys_get_cfg(lacp_sys_cfg_t * uid_cfg);
uint32_t lacp_port_get_dbg_cfg(uint32_t port_index, lacp_port_t * port);
uint32_t lacp_port_get_cfg(uint32_t port_index, lacp_port_cfg_t * uid_cfg);
uint32_t lacp_port_set_cfg(lacp_port_cfg_t * uid_cfg);
uint32_t lacp_create_ports(lacp_bitmap_t *ports);
uint32_t lacp_remove_ports(lacp_bitmap_t *ports);
uint32_t lacp_port_link_change(uint32_t port_index, uint32_t link_status);
uint32_t lacp_agg_get_port_state(uint32_t agg_id, lacp_port_state_t * uid_cfg, uint32_t *master_index);
uint32_t lacp_sys_get_state(lacp_sys_state_t * uid_cfg);
uint32_t lacp_dbg_trace(uint32_t port_index, char *state_name, Bool on);
uint32_t lacp_port_get_port_state(uint32_t port_index, lacp_port_state_t * state_para);
#endif
