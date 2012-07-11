#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_in.h"
#include "lac_default.h"
//#include "statmch.h"
#include "lac_rx.h"
#include "lac_tx.h"
#include "lac_mux.h"
#include "lac_sel.h"
#include "lac_pdu.h"

LAC_PORT_T *lac_port_create (LAC_SYS_T * lac_sys, int port_index)
{
	LAC_PORT_T *this;
	register unsigned int iii;
	unsigned short port_prio;
    printf("\r\n create portIndex:%d", port_index);
    
	/* check, if the port has just been added */
	for (this = lac_sys->ports; this; this = this->next) {
		if (this->port_index == port_index) {
			return NULL;
		}
	}

	/* add port to system */

	LAC_NEW_IN_LIST (this, LAC_PORT_T, lac_sys->ports, "port create");
	this->system = lac_sys;
	this->port_index = port_index;
	LAC_STRDUP (this->port_name, LAC_OUT_get_port_name (port_index), "port_name");
	this->machines = NULL;

	this->actor_admin.port_priority		   = Default_port_priority;
	this->actor_admin.port_index			   = port_index;
	this->actor_admin.system_priority	   = Default_system_priority;
	memcpy(this->actor_admin.system_id, this->system->id, 6);
	this->actor_admin.key				   = Default_key;
	
    LAC_STATE_SET_BIT(this->actor_admin.state, LAC_STATE_ACT, Default_lacp_activity);
    
	LAC_STATE_SET_BIT(this->actor_admin.state, LAC_STATE_TMT, Default_lacp_timeout);
	LAC_STATE_SET_BIT(this->actor_admin.state, LAC_STATE_AGG, Default_aggregation);
	LAC_STATE_SET_BIT(this->actor_admin.state, LAC_STATE_SYN, False);
	LAC_STATE_SET_BIT(this->actor_admin.state, LAC_STATE_DEF, True);
	LAC_STATE_SET_BIT(this->actor_admin.state, LAC_STATE_EXP, False);
  
	this->partner_admin.port_priority		 = Default_port_priority;
	this->partner_admin.port_index				 = port_index;/* TODO:: */
	this->partner_admin.system_priority		 = Default_system_priority;
	memset(this->partner_admin.system_id, 0, 6);
	this->partner_admin.key					 = this->port_index;
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_ACT, False); /* Passive	  */
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_TMT, False); /* Long timeout */
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_AGG, False); /* Individual   */
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_SYN,  True);
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_COL, True);
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_DIS, True);
    LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_DEF,	 True);
	LAC_STATE_SET_BIT(this->partner_admin.state, LAC_STATE_EXP, False);
    this->aport = this;
    
	
	iii = 0;
	this->timers[iii++] = &this->current_while;
	this->timers[iii++] = &this->periodic_timer;
	this->timers[iii++] = &this->wait_while;
  
	/* create and bind port state machines */
	LAC_STATE_MACH_IN_LIST (tx);
	LAC_STATE_MACH_IN_LIST (mux);
	LAC_STATE_MACH_IN_LIST (sel);
	//LAC_STATE_MACH_IN_LIST (periodic);
	LAC_STATE_MACH_IN_LIST (rx);

	this->mux->debug = 1;
	this->sel->debug = 1;
	this->rx->debug = 1;
	this->tx->debug = 1;
	
	/* reset timers */
	this->current_while =
	this->periodic_timer =
	this->wait_while = 0;
	
	this->port_moved = False;
	
    this->selected = False;
	
	return this;
}

void
lac_port_init (LAC_PORT_T * this, LAC_SYS_T * LACm)
{
}

void
lac_port_delete (LAC_PORT_T * this)
{
  LAC_SYS_T *lac_sys;
  register LAC_PORT_T *prev;
  register LAC_PORT_T *tmp;
  register LAC_STATE_MACH_T *stater;
  register void *pv;

  lac_sys = this->system;

  LAC_FREE (this->port_name, "port name");
  for (stater = this->machines; stater;) {
    pv = (void *) stater->next;
    lac_state_mach_delete (stater);
    stater = (LAC_STATE_MACH_T *) pv;
  }

  prev = NULL;
  for (tmp = lac_sys->ports; tmp; tmp = tmp->next) {
    if (tmp->port_index == this->port_index) {
      if (prev) {
	prev->next = this->next;
      } else {
	lac_sys->ports = this->next;
      }
      LAC_FREE (this, "LAC instance");
      break;
    }
    prev = tmp;
  }
}

unsigned int lac_port_rx (LAC_PORT_T * this, LACPDU_T * bpdu, size_t len)
{
	lac_rx_bpdu (this, bpdu, len);

	return 0;
}

int lac_set_port_reselect(LAC_PORT_T *port)
{
        LAC_PORT_T  *p, *p0;
	LAC_SYS_T *lac_sys;
	
	if (!port)
	{
		lac_sys = lac_get_sys_inst();
		p = p0 = lac_sys->ports;	
		
		while (p = p->next)
			p->reselect = True;

		return 0;
	}
	else
	{
		p = p0 = port->system->ports;
	
		if (port->static_agg)
		{
			while (p = p->next)
			{
				if (p->agg_id == port->agg_id)
				{
					p->reselect = True;
					//p->selected = False;
				} 
			}
		}
		else
		{
			//TODO::
		}
	}
}


#if 0
#ifdef LAC_DBG
unsigned int
LAC_LAC_PORT_Trace_state_machine (LAC_PORT_T * this, char *mach_name, unsigned int enadis,
			      unsigned int id)
{
  register struct state_mach_t *stater;

  for (stater = this->machines; stater; stater = stater->next) {
    if (!strcmp (mach_name, "all") || !strcmp (mach_name, stater->name)) {
      /* if (stater->debug != enadis) */
      {
	lac_trace ("port %s on %s trace %-8s (was %s) now %s",
		   this->port_name, this->owner->name,
		   stater->name,
		   stater->debug ? " enabled" : "disabled",
		   enadis ? " enabled" : "disabled");
      }
      stater->debug = enadis;
    }
  }

  return 0;
}
#endif
#endif

