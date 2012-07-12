typedef struct {
    /* service data */
    unsigned long field_mask; /* which fields to change */
    BITMAP_T      port_bmp;

    /* protocol data */
    Key key;
    unsigned short port_priority;
    Bool			  lacp_enabled;
    LAC_STATE 	  state;
        int sel_state;
        
    int agg_id;

} UID_LAC_PORT_CFG_T;

#define BR_CFG_PBMP_ADD         (1L << 0)
#define BR_CFG_PBMP_DEL         (1L << 1)
#define BR_CFG_PRIO         	(1L << 2)




#define PT_CFG_STATE    (1L << 0)
#define PT_CFG_COST     (1L << 1)
#define PT_CFG_PRIO     (1L << 2)
#define PT_CFG_P2P      (1L << 3)
#define PT_CFG_EDGE     (1L << 4)
#define PT_CFG_MCHECK   (1L << 5)
#define PT_CFG_NON_STP  (1L << 6)

typedef struct {
    /* service data */
    unsigned long	  field_mask; /* which fields to change */
    unsigned int			   number_of_ports;
    BITMAP_T		  ports;

    /* protocol data */
    SYSTEM_PRIORITY priority;

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


