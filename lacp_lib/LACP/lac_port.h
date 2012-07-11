
typedef LAC_KEY  Key;

typedef enum {LACP_ACTIVE = True, LAC_PASSIVE = False} LACP_ACTIVE_E;
typedef enum {SHORT_TIMEOUT = True, LONG_TIMEOUT = False} LACP_TIMEOUT_E;
typedef enum {LACP_ENABLED  = True, LACP_DISABLED= False} LACP_ENABLE_E;
#if 0
typedef struct /* LAC_STATE */
{
    unsigned lacp_activity   : 1;
    unsigned lacp_timeout    : 1;
    unsigned aggregation     : 1;
    unsigned synchronization : 1;
    unsigned collecting      : 1;
    unsigned distributing    : 1;
    unsigned defaulted       : 1;
    unsigned expired         : 1;
} LAC_STATE;
#endif
typedef unsigned char LAC_STATE;

#define LAC_STATE_ACT 0x1
#define LAC_STATE_TMT 0X2
#define LAC_STATE_AGG 0X4
#define LAC_STATE_SYN 0X8
#define LAC_STATE_COL 0X10
#define LAC_STATE_DIS 0X20
#define LAC_STATE_DEF 0X40
#define LAC_STATE_EXP 0X80


#define LAC_STATE_SET_BIT(flag,mask,value) (value) ? (flag |= mask) : (flag &= ~mask)
#define LAC_STATE_GET_BIT(flag,mask) ((flag & mask) ? 1 : 0)
#define LAC_STATE_COPY_BIT(flag_to, flag_from, mask) (flag_from & mask) ?  (flag_to |= mask) : (flag_to &= ~mask)
#define LAC_STATE_CMP_BIT(flag1, flag2, mask) ((flag1 & mask) != (flag2 & mask))

typedef struct /* LAC_PORT_INFO */
{
    SYSTEM_PRIORITY system_priority;

    SYSTEM_MAC       system_id;

    Key             key;

    LAC_PORT_ID         port_priority;

    LAC_PORT_ID         port_index;

    LAC_STATE       state;

} LAC_PORT_INFO;

#define TIMERS_NUMBER   3
#include "statmch.h"
typedef struct lac_port_t /* Lac_port */
{
    struct lac_port_t  *next;
    struct lac_sys_t   *system;
    LAC_PORT_ID     port_index;
    char*			port_name;

    LAC_STATE_MACH_T*	rx;
    LAC_STATE_MACH_T*	sel;
    LAC_STATE_MACH_T*	mux;
    LAC_STATE_MACH_T*	tx;

    LAC_STATE_MACH_T*	machines; /* list of machines */

    Bool         	port_enabled;
    Bool         	port_moved;

    LAC_PORT_INFO        actor;
    LAC_PORT_INFO        partner;
    LAC_PORT_INFO        actor_admin;
    LAC_PORT_INFO        partner_admin;

    LAC_PORT_INFO        msg_actor;
    LAC_PORT_INFO        msg_partner;

    Bool         	lacp_enabled;
//    Bool         	reselect;
    Bool         	selected;
//    Bool         	matched;
//    Bool         	local_churn;
//    Bool         	remote_churn;

    LAC_TIME_T           current_while;
    LAC_TIME_T           periodic_timer;
    LAC_TIME_T           wait_while;

    LAC_TIME_T*	timers[TIMERS_NUMBER]; /*list of timers */

    struct lac_port_t      *aport;
    struct lac_port_t      *alink;

//for selection logic
    int speed;
    int duplex;

    Bool            static_agg;
    int             agg_id;

//    Bool         	attach;
//    Bool         	attached;
    Bool         	standby;
    Bool         	ntt;
    int             hold_count;
    Bool ready_n;

//    Time          tick_timer;
    Bool 			rcvdLacpdu;

    unsigned int tx_lacpdu_cnt;
    unsigned int rx_lacpdu_cnt;
} LAC_PORT_T;
