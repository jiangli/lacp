#ifndef   lac_defaults_h__
#define   lac_defaults_h__


/******************************************************************************
 * LAC : LINK AGGREGATION CONTROL PROTOCOL : DEFAULTS
 ******************************************************************************
 */

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
    Default_lacp_timeout    = LONG_TIMEOUT,
    Default_aggregation     = True
};

#endif /* lac_h__ */

