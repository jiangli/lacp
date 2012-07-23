/* Generic (abstract state machine) state machine */

#ifndef _LACP_STATER_H__
#define _LACP_STATER_H__

#define LACP_BEGIN  9999 /* distinct from any valid state */

typedef struct lacp_state_mach_s {
    struct lacp_state_mach_s* next;

    char*         name; /* for debugging */

    char          debug; /* 0- no dbg, 1 - port, 2 - LACm */
    uint32_t  ignoreHop2State;

    Bool          change_state;
    uint32_t  state;

    void          (* concreteEnterState) (struct lacp_state_mach_s * );
    Bool          (* concreteCheckCondition) (struct lacp_state_mach_s * );
    char*         (* concreteGetStatName) (uint32_t);
    union {
        struct lacp_port_t* port;
        void         * owner;
    } owner;

} lacp_state_mach_t;

#define LACP_STATE_MACH_IN_LIST(WHAT)                              \
  {                                                               \
    lacp_state_mach_t* abstr;                                          \
                                                                  \
    abstr = lacp_state_mach_create (lac_##WHAT##_enter_state,      \
                                  lac_##WHAT##_check_conditions,  \
                                  lac_##WHAT##_get_state_name,    \
                                  port,                           \
                                  #WHAT);                         \
    abstr->next = port->machines;                                 \
    port->machines = abstr;                                       \
    port->WHAT = abstr;                       \
  }


lacp_state_mach_t *
lacp_state_mach_create (void (* concreteEnterState) (lacp_state_mach_t*),
                       Bool (* concreteCheckCondition) (lacp_state_mach_t*),
                       char * (* concreteGetStatName) (uint32_t),
                       void* owner, char* name);

void
lacp_state_mach_delete (lacp_state_mach_t* fsm);

Bool
lacp_check_condition (lacp_state_mach_t* fsm);

Bool
lacp_change_state (lacp_state_mach_t* fsm);

Bool
lacp_hop_2_state (lacp_state_mach_t* fsm, uint32_t new_state);

#endif /* _LAC_STATER_H__ */

