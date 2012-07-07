#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_pdu.h"

#define STATES {        \
  CHOOSE(DETACHED),    \
  CHOOSE(WAITING),    \
  CHOOSE(ATTACHED),         \
  CHOOSE(RX_TX),  \
}

#define GET_STATE_NAME lac_mux_get_state_name
#include "choose.h"

int sys_is_ready()
{
	
}
int disable_collecting_distributing()
{
	
}
int enable_collecting_distributing()
{
	
}

int detach_mux_from_aggregator()
{
}
int attach_mux_to_aggregator()
{
}

void lac_mux_enter_state (LAC_STATE_MACH_T * this)
{
  register LAC_PORT_T *port = this->owner.port;

  switch (this->State) {
    case BEGIN:
    case DETACHED:
		detach_mux_from_aggregator(port);
		port->actor.state.synchronization = False;
		port->actor.state.collecting = False;
		disable_collecting_distributing(port);
		port->actor.state.distributing = False;
		port->ntt = True;
		break;
		
	case WAITING:
	    port->wait_while = port->system->aggregate_wait_time;
		break;
		
	case ATTACHED:
		attach_mux_to_aggregator(port);
		port->actor.state.synchronization = True;
		port->actor.state.collecting = False;
		disable_collecting_distributing(port);
		port->actor.state.distributing = False;
		port->ntt = True;
		break;
		
	case RX_TX:
		port->actor.state.distributing = True;
		enable_collecting_distributing(port);
		port->actor.state.collecting = True;
		port->ntt = True;
		break;

  };
}

Bool lac_mux_check_conditions (LAC_STATE_MACH_T * this)
{
  register LAC_PORT_T *port = this->owner.port;
  
  switch (this->State) {
    case BEGIN:
	case DETACHED:
		if (port->selected == True)
	    	return lac_hop_2_state (this, WAITING);
		break;
	case WAITING:	 
		if (port->selected == True && sys_is_ready())
	    	return lac_hop_2_state (this, ATTACHED);
		
		if (!port->selected)
	    	return lac_hop_2_state (this, DETACHED);
		break;
		
	case ATTACHED:
		if (port->selected && port->partner.state.synchronization)
	    	return lac_hop_2_state (this, RX_TX);
		if (!port->selected)
	    	return lac_hop_2_state (this, DETACHED);
		break;
		
	case RX_TX:
		if (!port->selected || !port->partner.state.synchronization)
	    	return lac_hop_2_state (this, ATTACHED);
		break;
		
	default:
		break;
  };
  return False;
}
