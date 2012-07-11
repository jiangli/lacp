#include "lac_port.h"

enum    {Slow_protocols_ethertype = (USHORT)0x4242};
enum    {Lacp_subtype             = (Octet)1};


enum    {Null_system = 0};




typedef struct lac_sys_t /* Lac_system */
{
    LAC_PORT_T * ports;
    BITMAP_T * portmap;
    int number_of_ports;
    int admin_state;

    SYSTEM_PRIORITY priority;
    SYSTEM_MAC       id;

    LAC_TIME_T fast_periodic_time;
    LAC_TIME_T slow_periodic_time;
    LAC_TIME_T short_timeout_time;
    LAC_TIME_T long_timeout_time;
    LAC_TIME_T aggregate_wait_time;

    int tx_hold_count;
} LAC_SYS_T;


#define LAC_SYS_UPDATE_READON_RX 1
#define LAC_SYS_UPDATE_READON_TIMER 2
#define LAC_SYS_UPDATE_READON_PORT_CFG 3
#define LAC_SYS_UPDATE_READON_SYS_CFG 4
#define LAC_SYS_UPDATE_READON_LINK 5


