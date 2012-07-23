#ifndef   __LACP_API_H__
#define   __LACP_API_H__


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
    Default_system_priority = 1,
    Default_port_priority   = 1,
    Default_key             = 0,
    Default_lacp_activity   = True,
    Default_lacp_timeout    = LACP_LONG_TIMEOUT,
    Default_aggregation     = True
};


typedef struct {
    /* service data */
    unsigned long field_mask; /* which fields to change */
    lacp_bitmap_t port_bmp;

    /* protocol data */
    Bool 			 lacp_enabled;
    lacp_key_t		 key;
    lacp_port_prio_t port_priority;
    lacp_state_t 	 state;
    int 			 agg_id;
    lacp_port_info_t partner;

} lacp_port_cfg_t;

typedef struct {
	int valid;
    int port_index;
    lacp_key_t key;
    int agg_id;
    int master_port;
    int sel_state;  /* selected or standby. readonly */
    int rx_cnt;
    int tx_cnt;
    lacp_port_info_t        actor;
    lacp_port_info_t        partner;

} UID_LAC_PORT_STATE_T;

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
#define PT_CFG_EDGE     (1L << 4)
#define PT_CFG_MCHECK   (1L << 5)
#define PT_CFG_NON_STP  (1L << 6)

typedef struct {
    /* service data */
    unsigned long	  field_mask; /* which fields to change */
    unsigned int			   number_of_ports;
    lacp_bitmap_t		  ports;
    int period;
    /* protocol data */
    lacp_sys_prio_t priority;
    lacp_mac_t sys_mac;
    unsigned int short_period;
    unsigned int long_period;
} UID_LAC_CFG_T;

#ifdef __LINUX__
#  define LAC_INIT_CRITICAL_PATH_PROTECTIO
#  define LAC_CRITICAL_PATH_START
#  define LAC_CRITICAL_PATH_END
#else
#  define LAC_INIT_CRITICAL_PATH_PROTECTIO  lac_out_init_sem();
#  define LAC_CRITICAL_PATH_START			lac_out_sem_take();
#  define LAC_CRITICAL_PATH_END				lac_out_sem_give();

#endif


typedef enum {
    LAC_DISABLED,
    LAC_ENABLED,
    LAC_EMULATION
} UID_LAC_MODE_T;


typedef enum {
    M_LACP_INTERNEL,
    M_RSTP_PORT_IS_ABSENT,
    M_RSTP_NOT_ENABLE,
} LACP_ERROR_E;

unsigned int lacp_port_get_actor_admin(int port_index, lacp_port_info_t  *admin);
unsigned int lacp_port_get_partner_admin(int port_index, lacp_port_info_t  *admin);
lacp_sys_t *lacp_get_sys_inst (void);
int lac_port_set_cfg(lacp_port_cfg_t * uid_cfg);
int lac_port_get_cfg(int port_index, lacp_port_cfg_t * uid_cfg);
int lac_rx_lacpdu(int port_index, lacp_pdu_t * bpdu, int len);
int lac_port_get_dbg_cfg(int port_index, lacp_port_t * port);

void
lac_one_second ();
int lac_in_enable_port(int port_index, Bool enable);
int lac_sys_set_cfg(UID_LAC_CFG_T * uid_cfg);
int lac_sys_get_cfg(UID_LAC_CFG_T * uid_cf);

#endif 
