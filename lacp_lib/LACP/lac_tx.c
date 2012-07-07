#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_pdu.h"

#define STATES {        \
  CHOOSE(NO_PERIODIC),    \
  CHOOSE(PERIODIC_TX),    \
  CHOOSE(IDLE),         \
  CHOOSE(TX),  \
}

#define GET_STATE_NAME lac_tx_get_state_name
#include "choose.h"

#define MIN_FRAME_LENGTH    64


static LACPDU_T lacpdu_packet;

static int
tx_lacpdu(LAC_STATE_MACH_T * this)
{				/* 17.19.15 (page 67) & 9.3.1 (page 23) */
	register size_t pkt_len;
	register int port_index;
	register LAC_PORT_T *port = this->owner.port;
	const unsigned char slow_protocols_address[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02};  

	if (!port->lacp_enabled)
	{
		printf("\r\n not enable tx");
		return 1;
	}

	port = this->owner.port;
	port_index = port->port_index;

	memset(&lacpdu_packet, 0, sizeof(LACPDU_T));  

	memcpy(lacpdu_packet.slow_protocols_address, slow_protocols_address, 6);
	LAC_OUT_get_port_mac (lacpdu_packet.src_address);
	lacpdu_packet.ethertype = 0x8809;
	lacpdu_packet.protocol_subtype = 1;
	lacpdu_packet.protocol_version = 1;

	lacpdu_packet.type_actor = 1;
	lacpdu_packet.len_actor = 20;
	lacpdu_packet.actor.system_priority = port->actor.system_priority;
	memcpy(lacpdu_packet.actor.system_id, port->actor.system_id, 6);
	lacpdu_packet.actor.key = port->actor.key;
	lacpdu_packet.actor.port_priority = port->actor.port_priority;
	lacpdu_packet.actor.port_index = port->actor.port_index;
	lacpdu_packet.actor.state.lacp_activity = port->actor.state.lacp_activity;
	lacpdu_packet.actor.state.lacp_timeout = port->actor.state.lacp_timeout;
	lacpdu_packet.actor.state.aggregation = port->actor.state.aggregation;
	lacpdu_packet.actor.state.collecting = port->actor.state.collecting;
	lacpdu_packet.actor.state.distributing = port->actor.state.distributing;
	lacpdu_packet.actor.state.defaulted = port->actor.state.defaulted;
	lacpdu_packet.actor.state.expired = port->actor.state.expired;


	lacpdu_packet.type_partner = 2;
	lacpdu_packet.len_partner = 20;
	lacpdu_packet.partner.system_priority = port->partner.system_priority;
	memcpy(lacpdu_packet.partner.system_id, port->partner.system_id, 6);
	lacpdu_packet.partner.key = port->partner.key;
	lacpdu_packet.partner.port_priority = port->partner.port_priority;
	lacpdu_packet.partner.port_index = port->partner.port_index;
	lacpdu_packet.partner.state.lacp_activity = port->partner.state.lacp_activity;
	lacpdu_packet.partner.state.lacp_timeout = port->partner.state.lacp_timeout;
	lacpdu_packet.partner.state.aggregation = port->partner.state.aggregation;
	lacpdu_packet.partner.state.collecting = port->partner.state.collecting;
	lacpdu_packet.partner.state.distributing = port->partner.state.distributing;
	lacpdu_packet.partner.state.defaulted = port->partner.state.defaulted;
	lacpdu_packet.partner.state.expired = port->partner.state.expired;

	lacpdu_packet.type_collector = 3;
	lacpdu_packet.len_collector = 16;
	lacpdu_packet.collector_max_delay = 10;

	/* statictic */
	port->tx_lacpdu_cnt++;

	/* tx */
	return LAC_OUT_tx_bpdu (port_index, (unsigned char *) &lacpdu_packet, sizeof(LACPDU_T));
}


void lac_tx_enter_state (LAC_STATE_MACH_T * this)
{
  register LAC_PORT_T *port = this->owner.port;

  switch (this->State) {
    case BEGIN:
    case NO_PERIODIC:
	    port->periodic_timer = 0;
		port->hold_count = 0;
	    break;
		
	case PERIODIC_TX:
		port->hold_count = 0;
		port->ntt = True;

		if (port->partner.state.lacp_timeout == LONG_TIMEOUT)
		{
		    port->periodic_timer = port->system->fast_periodic_time;
		}
		else
		{
		    port->periodic_timer = port->system->slow_periodic_time;
		}
		break;
		
	case TX:	
		tx_lacpdu (this);
    	port->hold_count++;
    	port->ntt = False;
	    break;
  };
}

Bool lac_tx_check_conditions (LAC_STATE_MACH_T * this)
{
  register LAC_PORT_T *port = this->owner.port;
  
  if (BEGIN == this->State || !port->lacp_enabled || !port->port_enabled 
	|| (port->actor.state.lacp_activity == LAC_PASSIVE 
		&& port->partner.state.lacp_activity == LAC_PASSIVE))
  {  	
	/*printf("\r\n port:%d state:%d, port_enable:%d, lacp_enabled:%d", 
	port->port_index, this->State, port->port_enabled, port->lacp_enabled);*/
	if (NO_PERIODIC == this->State)
	{
		return False;
	}
	else
	{		
	    return lac_hop_2_state (this, NO_PERIODIC);
	}
  }

  switch (this->State) {
    case NO_PERIODIC:
    	return lac_hop_2_state (this, IDLE);
		
	case PERIODIC_TX:	 
		return lac_hop_2_state (this, IDLE);
				
	case IDLE:
		if (!port->periodic_timer)
		{
			return lac_hop_2_state (this, PERIODIC_TX);		
		}

		if (port->ntt && port->hold_count < port->system->tx_hold_count)
		{
			return lac_hop_2_state (this, TX);			
		}
		break;
	case TX:
		return lac_hop_2_state (this, IDLE);

  };
  return False;
}
#if 0
int
LAC_transmit_dbg_sttx (int bpdu_type, STATE_MACH_T * this)
{
  register PORT_T *port = this->owner.port;

  if (!port->adminEnable) {
    stp_trace ("stt on disbaled port %s", port->port_name);
    return -1;
  }
  switch (bpdu_type) {
    default:
    case 0:
    return txRstp (this);
    case 1:
    return txTcn (this);
    case 2:
    return txConfig (this);
  }
}
#endif


/*
static LACPDU_T lacpdu_packet = {
   {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00},	
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	
   {0x88,0x09},
   {0x01},{0x01},
   {0x01},{0x14},
   {{0x00, 0x00},
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00},
   {0x00, 0x00, 0x00},
   {0x01},{0x01},
   {0x01},{0x14},
   {0x00, 0x00},
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00}},
   {0x00, 0x00, 0x00},   	
   {0x02},{0x14},
   {{0x00, 0x00},
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00},
   {0x00, 0x00, 0x00},
   {0x01},{0x01},
   {0x01},{0x14},
   {0x00, 0x00},
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00, 0x00},
   {0x00}},
   {0x00, 0x00, 0x00},
   	
   {0x03},{0x10},
   {0x00, 0x00},
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};


*/
