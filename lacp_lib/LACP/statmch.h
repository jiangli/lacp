/************************************************************************
 * RLAC library - Rapid Spanning Tree (802.1t, 802.1w)
 * Copyright (C) 2001-2003 Optical Access
 * Author: Alex Rozin
 *
 * This file is part of RLAC library.
 *
 * RLAC library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; version 2.1
 *
 * RLAC library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RLAC library; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 **********************************************************************/

/* Generic (abstract state machine) state machine : 17.13, 17.14 */

#ifndef _LAC_STATER_H__
#define _LAC_STATER_H__

#define BEGIN  9999 /* distinct from any valid state */

typedef struct state_mach_t {
    struct state_mach_t* next;

    char*         name; /* for debugging */
#ifdef LAC_DBG
    char          debug; /* 0- no dbg, 1 - port, 2 - LACm */
    unsigned int  ignoreHop2State;
#endif

    Bool          changeState;
    unsigned int  State;

    void          (* concreteEnterState) (struct state_mach_t * );
    Bool          (* concreteCheckCondition) (struct state_mach_t * );
    char*         (* concreteGetStatName) (int);
    union {
        struct lac_port_t* port;
        void         * owner;
    } owner;

} LAC_STATE_MACH_T;

#define LAC_STATE_MACH_IN_LIST(WHAT)                              \
  {                                                               \
    LAC_STATE_MACH_T* abstr;                                          \
                                                                  \
    abstr = lac_state_mach_create (lac_##WHAT##_enter_state,      \
                                  lac_##WHAT##_check_conditions,  \
                                  lac_##WHAT##_get_state_name,    \
                                  this,                           \
                                  #WHAT);                         \
    abstr->next = this->machines;                                 \
    this->machines = abstr;                                       \
    this->WHAT = abstr;                       \
  }


LAC_STATE_MACH_T *
lac_state_mach_create (void (* concreteEnterState) (LAC_STATE_MACH_T*),
                       Bool (* concreteCheckCondition) (LAC_STATE_MACH_T*),
                       char * (* concreteGetStatName) (int),
                       void* owner, char* name);

void
lac_state_mach_delete (LAC_STATE_MACH_T* this);

Bool
lac_check_condition (LAC_STATE_MACH_T* this);

Bool
lac_change_state (LAC_STATE_MACH_T* this);

Bool
lac_hop_2_state (LAC_STATE_MACH_T* this, unsigned int new_state);

#endif /* _LAC_STATER_H__ */

