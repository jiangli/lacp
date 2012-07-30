#ifndef __LACP_TYPE_H__
#define __LACP_TYPE_H__

typedef unsigned short  lacp_port_id_t;
typedef unsigned short  lacp_key_t;
typedef unsigned short  lacp_sys_prio_t;
typedef unsigned short  lacp_port_prio_t;
typedef unsigned short  lacp_timer_t;

#define LACP_MAC_LEN 6
typedef unsigned char lacp_mac_t[LACP_MAC_LEN];

#define LACP_ACTIVE True
#define LACP_PASSIVE False

#define LACP_SHORT_TIMEOUT True
#define LACP_LONG_TIMEOUT  False

#define LACP_ENABLED  True
#define LACP_DISABLED False

#if 0
typedef struct /* lacp_state_t */
{
    unsigned lacp_activity   : 1;
    unsigned lacp_timeout    : 1;
    unsigned aggregation     : 1;
    unsigned synchronization : 1;
    unsigned collecting      : 1;
    unsigned distributing    : 1;
    unsigned defaulted       : 1;
    unsigned expired         : 1;
} lacp_state_t;
#endif

typedef unsigned char lacp_state_t;

#define LACP_STATE_ACT 0x1
#define LACP_STATE_TMT 0X2
#define LACP_STATE_AGG 0X4
#define LACP_STATE_SYN 0X8
#define LACP_STATE_COL 0X10
#define LACP_STATE_DIS 0X20
#define LACP_STATE_DEF 0X40
#define LACP_STATE_EXP 0X80


#define LACP_STATE_SET_BIT(flag,mask,value) ((value) ? (flag |= mask) : (flag &= ~mask))
#define LACP_STATE_GET_BIT(flag,mask) ((flag & mask) ? 1 : 0)
/*#define LAC_STATE_COPY_BIT(flag_to, flag_from, mask) ((flag_from & mask) ?  (flag_to |= mask) :
(flag_to &=
~mask))*/

/* return zero if equal */
#define LACP_STATE_CMP_BIT(flag1, flag2, mask) ((flag1 & mask) != (flag2 & mask))

typedef struct lacp_port_info_s /* lacp_port_info_t */
{
    lacp_sys_prio_t       system_priority;
    lacp_mac_t            system_mac;
    lacp_key_t            key;
    lacp_port_prio_t      port_priority;
    lacp_port_id_t        port_no;
    lacp_state_t          state;
} lacp_port_info_t;

#define TIMERS_NUMBER   3
#include "lacp_statmch.h"

typedef struct lacp_port_s 
{
    struct lacp_port_s  *next;
    struct lacp_sys_s   *system;
    uint32_t            port_index; /* 端口索引,从0开始 */
    char*         	port_name;

    lacp_state_mach_t* rx;
    lacp_state_mach_t* sel;
    lacp_state_mach_t* mux;
    lacp_state_mach_t* tx;

    lacp_state_mach_t* machines; /* list of machines */

    lacp_port_info_t    actor;
    lacp_port_info_t    partner;
    lacp_port_info_t    actor_admin;
    lacp_port_info_t    partner_admin;

    lacp_port_info_t    msg_actor;
    lacp_port_info_t    msg_partner;

    lacp_timer_t    current_while;
    lacp_timer_t    periodic_timer;
    lacp_timer_t    wait_while;

    lacp_timer_t*   timers[TIMERS_NUMBER]; /*list of timers */

    struct lacp_port_s  *master_port;    /* 指向聚合组主端口,无主端口时，指向自己 */

    Bool        	port_enabled;		/* 端口是否up */
    Bool          	port_moved;		/* 端口物理连接是否移动 */
    Bool          	lacp_enabled;		/* 端口lacp是否开启 */
    Bool          	selected;		/* 端口是否已完成聚合选择 */
    Bool            standby;			/* 端口的选中状态是否为备用 */

    uint32_t       	 	speed;			/* 速率,100/1000/10000 */
    uint32_t        	duplex;			/* 双工，1:全双工，0:半双工 */
    uint32_t        	agg_id;			/* 聚合组id,从1开始 */

    Bool        	rcvd_lacpdu;		/* 端口是否接收到协议报文并且未处理 */
    Bool            ntt;				/* need to transmit */

    Bool        	ready_n;			/* 端口是否已准备好加入聚合组 */

    uint32_t			hold_count;		/* 端口1s内已发送的协议报文数 */
    uint32_t        	tx_lacpdu_cnt;	/* 端口开启LACP后的发送协议报文数 */
    uint32_t        	rx_lacpdu_cnt;	/* 端口开启LACP后的接收协议报文数 */
} lacp_port_t;

typedef struct lacpdu_t
{
    unsigned char  slow_protocols_address[6];
    unsigned char  src_address[6];
    unsigned short  ethertype;
    unsigned char protocol_subtype;
    unsigned char protocol_version;


    unsigned char type_actor;
    unsigned char len_actor;
    lacp_port_info_t  actor;
    unsigned char reverved1[3];

    unsigned char type_partner;
    unsigned char len_partner;
    lacp_port_info_t  partner;
    unsigned char reverved2[3];

    unsigned char type_collector;
    unsigned char len_collector;
    unsigned short collector_max_delay;
    unsigned char reverved3[12];

    unsigned char type_terminator;
    unsigned char len_terminator;
    unsigned char reverved4[50];
} lacp_pdu_t;


#define slow_protocols_ethertype (USHORT)0x4242};
#define lacp_subtype  (Octet)1};

typedef struct lacp_sys_s
{
    lacp_port_t * ports;  /* all lacp port link list */
    lacp_bitmap_t * portmap;
    uint32_t number_of_ports;
    uint32_t admin_state;

    lacp_sys_prio_t  priority;
    lacp_mac_t       mac;

    lacp_timer_t fast_periodic_time;
    lacp_timer_t slow_periodic_time;
    lacp_timer_t short_timeout_time;
    lacp_timer_t long_timeout_time;
    lacp_timer_t aggregate_wait_time;

    Bool lacp_timeout;	/* 使用长超时或短超时 */
    uint32_t tx_hold_count;
} lacp_sys_t;


#endif
