/************************************************************************
 * RSTP library - Rapid Spanning Tree (802.1t, 802.1w)
 * Copyright (C) 2001-2003 Optical Access
 * Author: Alex Rozin
 *
 * This file is part of RSTP library.
 *
 * RSTP library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; version 2.1
 *
 * RSTP library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RSTP library; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 **********************************************************************/

/* Generic (abstract) state machine : 17.13, 17.14 */

#include "lac_base.h"
#include "lac_port.h"
#include "statmch.h"
#include "../lac_out.h"
#if LAC_DBG
#  include "lac_base.h"
#endif

LAC_STATE_MACH_T *
lac_state_mach_create (void (*concreteEnterState) (LAC_STATE_MACH_T *),
                       Bool (*concreteCheckCondition) (LAC_STATE_MACH_T *),
                       char *(*concreteGetStatName) (int),
                       void *owner, char *name)
{
    LAC_STATE_MACH_T *this;

    LAC_MALLOC (this, LAC_STATE_MACH_T, "state machine");

    this->State = BEGIN;
    LAC_STRDUP (this->name, name, "stpm name");
    this->changeState = False;
#if LAC_DBG
    this->debug = False;
    this->ignoreHop2State = BEGIN;
#endif
    this->concreteEnterState = concreteEnterState;
    this->concreteCheckCondition = concreteCheckCondition;
    this->concreteGetStatName = concreteGetStatName;
    this->owner.owner = owner;

    return this;
}

void
lac_state_mach_delete (LAC_STATE_MACH_T * this)
{
    LAC_FREE (this->name, "stpm name");
    LAC_FREE (this, "state machine");
}

Bool
lac_check_condition (LAC_STATE_MACH_T * this)
{
    Bool bret;

    bret = (*(this->concreteCheckCondition)) (this);
    if (bret) {
        this->changeState = True;
    }

    return bret;
}

Bool
lac_change_state (LAC_STATE_MACH_T * this)
{
    register int number_of_loops;

    for (number_of_loops = 0;; number_of_loops++) {
        if (!this->changeState)
            return number_of_loops;
        (*(this->concreteEnterState)) (this);
        this->changeState = False;
        lac_check_condition (this);
    }

    return number_of_loops;
}

Bool
lac_hop_2_state (LAC_STATE_MACH_T * this, unsigned int new_state)
{
#ifdef LAC_DBG
    switch (this->debug) {
    case 0:
        break;
    case 1:
        if (new_state == this->State || new_state == this->ignoreHop2State)
            break;
        lac_trace ("%-8s(%s): %s=>%s",
                   this->name, this->owner.port->port_name,
                   (*(this->concreteGetStatName)) (this->State),
                   (*(this->concreteGetStatName)) (new_state));
        break;
    case 2:
        if (new_state == this->State)
            break;
        lac_trace ("%s: %s=>%s",
                   this->name,
                   (*(this->concreteGetStatName)) (this->State),
                   (*(this->concreteGetStatName)) (new_state));
        break;
    }
#endif

    this->State = new_state;
    this->changeState = True;
    return True;
}
