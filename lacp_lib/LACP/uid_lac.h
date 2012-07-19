/* External management communication API definitions */

#ifndef _UID_LAC_H__
#define _UID_LAC_H__

#define NAME_LEN    20

typedef struct {
    /* service data */
    unsigned long field_mask; /* which fields to change */
    BITMAP_T      port_bmp;

    /* protocol data */
    LAC_KEY key;
    unsigned short port_priority;
    Bool			  lacp_enabled;
    LAC_STATE 	  state;/* readonly */
    int sel_state;  /* selected or standby. readonly */
    int agg_id;
    LAC_PORT_INFO        partner;

} UID_LAC_PORT_CFG_T;
typedef struct {
    int port_index;
    LAC_KEY key;
    int agg_id;
    int master_port;
    int sel_state;  /* selected or standby. readonly */
    int rx_cnt;
    int tx_cnt;
    LAC_PORT_INFO        actor;
    LAC_PORT_INFO        partner;

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
    BITMAP_T		  ports;
    int period;
    /* protocol data */
    SYSTEM_PRIORITY priority;
    SYSTEM_MAC sys_mac;
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

typedef struct
{
    int cnt;

    int ports[8];
} LINK_GROUP_T;
#endif
