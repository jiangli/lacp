
/* Generic (abstract) state machine */

#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_statmch.h"
#include "../lacp_ssp.h"

lacp_state_mach_t *
lacp_state_mach_create (void (*concreteEnterState) (lacp_state_mach_t *),
                        Bool (*concreteCheckCondition) (lacp_state_mach_t *),
                        char *(*concreteGetStatName) (uint32_t),
                        void *owner, char *name)
{
    lacp_state_mach_t *fsm;

    LACP_MALLOC (fsm, lacp_state_mach_t, "state machine");

    fsm->state = LACP_BEGIN;
    LACP_STRDUP (fsm->name, name, "stpm name");
    fsm->change_state = False;
#if LAC_DBG
    fsm->debug = False;
    fsm->ignoreHop2State = LACP_BEGIN;
#endif
    fsm->concreteEnterState = concreteEnterState;
    fsm->concreteCheckCondition = concreteCheckCondition;
    fsm->concreteGetStatName = concreteGetStatName;
    fsm->owner.owner = owner;

    return fsm;
}

void
lacp_state_mach_delete (lacp_state_mach_t * fsm)
{
    LACP_FREE (fsm->name, "stpm name");
    LACP_FREE (fsm, "state machine");
}

Bool
lacp_check_condition (lacp_state_mach_t * fsm)
{
    Bool bret;

    bret = (*(fsm->concreteCheckCondition)) (fsm);
    if (bret) {
        fsm->change_state = True;
    }

    return bret;
}

Bool
lacp_change_state (lacp_state_mach_t * fsm)
{
    register uint32_t number_of_loops;

    for (number_of_loops = 0;; number_of_loops++) {
        if (!fsm->change_state)
            return number_of_loops;
        (*(fsm->concreteEnterState)) (fsm);
        fsm->change_state = False;
        lacp_check_condition (fsm);
    }

    return number_of_loops;
}

Bool
lacp_hop_2_state (lacp_state_mach_t * fsm, uint32_t new_state)
{
#ifdef LAC_DBG
    switch (fsm->debug) {
    case 0:
        break;
    case 1:
        if (new_state == fsm->state || new_state == fsm->ignoreHop2State)
            break;
        lacp_trace ("%-8s(%s): %s=>%s",
                    fsm->name, fsm->owner.port->port_name,
                    (*(fsm->concreteGetStatName)) (fsm->state),
                    (*(fsm->concreteGetStatName)) (new_state));
        break;
    case 2:
        if (new_state == fsm->state)
            break;
        lacp_trace ("%s: %s=>%s",
                    fsm->name,
                    (*(fsm->concreteGetStatName)) (fsm->state),
                    (*(fsm->concreteGetStatName)) (new_state));
        break;
    }
#endif

    fsm->state = new_state;
    fsm->change_state = True;
    return True;
}
