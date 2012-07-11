/*        lac_defaults.h      1.00.000 28FEB99 21:03 */
#ifndef   lac_defaults_h__
#define   lac_defaults_h__


/******************************************************************************
 * LAC : LINK AGGREGATION CONTROL PROTOCOL : DEFAULTS
 ******************************************************************************
 */

enum {/*
       * Lac_ticks is the number of internal timer ticks per LAC tick.
       * The finer internal granularity allows extension of the model to
       * random or uncertain transmission delays.
       */
    Lac_ticks = 10
};

enum {/*
       * This example implementation takes a simple approach to transmit
       * scheduling. The tx_scheduler timer checks ntt frequently, and
       * transmits as needed subject to the maximum transmission rate
       * limitations imposed by the tx_machine.
       */
    Lac_tx_scheduling_ticks = 1
};

enum {/*
       * All the following LAC timers operate in terms of LAC ticks. If
       * a dedicated timer approach was to be used for LAC timers, rather
       * than a period timer tick per port with the LAC component counting
       * down individual timers then the following would need to be multiplied
       * by Lac_ticks (above).
       */
    Slow_periodic_ticks   =   4,/* should be 30, faster for simulation */
    Fast_periodic_ticks   =   1,
    Long_timeout_ticks    =   8,/* should be 90, faster for simulation */
    Short_timeout_ticks   =   3,
    Churn_detection_ticks =  60,
    Aggregate_wait_ticks  =   2,
};

enum {/*
       */
    Max_tx_per_interval  =   3
};

enum {/*
       * Default settings for system characteristics.
       */
    Default_system_priority = 1,
    Default_port_priority   = 1,
    Default_key             = 1,
    Default_lacp_activity   = True,
    Default_lacp_timeout    = LONG_TIMEOUT,
    Default_aggregation     = True
};

#endif /* lac_h__ */

