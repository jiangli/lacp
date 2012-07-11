#include "lac_base.h"
#include "statmch.h"
#include "lac_sys.h"
#include "lac_pdu.h"

			 /* The Port Information State Machine : 17.21 */
			 
#define STATES { \
		       CHOOSE(RXM_INITIALIZE), \	
			   CHOOSE(RXM_PORT_DISABLED), \
			   CHOOSE(RXM_EXPIRED),	 \
			   CHOOSE(RXM_CURRENT),  \
			   CHOOSE(RXM_LACP_DISABLED),  \
			   CHOOSE(RXM_DEFAULTED), \
			 }
			 
#define GET_STATE_NAME lac_rx_get_state_name
#include "choose.h"
			 

 static void copy_info(LAC_PORT_INFO *from, LAC_PORT_INFO *to)
 {
	to->port_priority		  = from->port_priority;
	to->port_index            = from->port_index;
	to->system_priority 	  = from->system_priority;
	memcpy(to->system_id, from->system_id, 6);
	to->key 				  = from->key;
	to->state                 = from->state;
 }

 static void copy_info_from_net(LAC_PORT_INFO *from, LAC_PORT_INFO *to)
 {
         to->port_priority		  = ntohs(from->port_priority);
         to->port_index            = ntohs(from->port_index);
         to->system_priority 	  = ntohs(from->system_priority);
	memcpy(to->system_id, from->system_id, 6);
	to->key 				  = ntohs(from->key);
	to->state		  = from->state;
 }

 void
 lac_rx_bpdu (LAC_PORT_T * port, LACPDU_T *Lacpdu, int len)
 {
 	/* msg check */
	
 	/* statistic */
	port->rx_lacpdu_cnt++;

	if (port->lacp_enabled == False)
		return;
	
 	port->rcvdLacpdu = True;
          memdump(Lacpdu, len);
    
	copy_info_from_net(&Lacpdu->actor, &port->msg_actor);
	copy_info_from_net(&Lacpdu->partner, &port->msg_partner);
	return 0; 
 }
 static Bool same_port(LAC_PORT_INFO *a, LAC_PORT_INFO *b)
 {
	return (  (a->port_index			== b->port_index)
				&& (!memcmp(a->system_id, b->system_id, 6))
		   );
 }
 static Bool lac_same_partner(LAC_PORT_INFO *a, LAC_PORT_INFO *b)
 {/*
   */
	return (  (a->port_priority 	  == b->port_priority)
		   && (a->port_index == b->port_index)
		   && (a->system_priority	  == b->system_priority)
		   && (!memcmp(a->system_id, b->system_id, 6))
		   && (a->key				  == b->key)
              && (LAC_STATE_GET_BIT(a->state, LAC_STATE_AGG)   == LAC_STATE_GET_BIT(b->state, LAC_STATE_AGG))
		   );
 }
 static void actor_default(LAC_PORT_T *port)
 {
	copy_info(&port->actor_admin, &port->actor);
 }

int record_default(LAC_PORT_T *port)
{
	copy_info(&port->partner_admin, &port->partner);
	LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DEF , True);
}
static void choose_matched(LAC_PORT_T *port)
{
   if (  (  (lac_same_partner(&port->msg_partner, &port->actor)
             && LAC_STATE_GET_BIT(port->msg_partner.state, LAC_STATE_AGG) == LAC_STATE_GET_BIT(port->actor.state, LAC_STATE_AGG))
            || (  !LAC_STATE_GET_BIT(port->msg_actor.state, LAC_STATE_AGG))
            && (  (    LAC_STATE_GET_BIT(port->msg_actor.state, LAC_STATE_ACT)
            )
                  || (   LAC_STATE_GET_BIT(port->actor.state, LAC_STATE_ACT)
                         &&  LAC_STATE_GET_BIT(port->msg_partner.state, LAC_STATE_ACT))
                    )))
      port->matched = True;
   else
   {
           printf("\r\n port:%d partner changed  ! ", port->port_index);
           
                 port->ntt = True;
      port->matched = False;
   }
   
}

int update_selected(LAC_PORT_T *port)
{
	   if (!lac_same_partner(&port->msg_actor, &port->partner))
	   {
		  port->selected					= False;
		  port->reselect					= True;
		  port->standby 					= False;
		  LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
	}
}
static void update_ntt(LAC_PORT_T *port)
{
   if ( !(lac_same_partner(&port->msg_partner, &port->actor))
        || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_ACT))
        || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_AGG)
            || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state, LAC_STATE_SYN))
            || (LAC_STATE_CMP_BIT(port->msg_partner.state, port->actor.state,LAC_STATE_COL)))
      )
      port->ntt = True;
}
int record_pdu(LAC_PORT_T *port)
{
   copy_info(&port->msg_actor, &port->partner);
   LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_DEF, False);
}
int update_default_selected(LAC_PORT_T *port)
{
	   if (!lac_same_partner(&port->partner_admin, &port->partner))
	   {
		  port->selected					= False;
		  port->standby 					= False;
		  LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_SYN, False);
	   } 
}
static int lac_check_moved(LAC_PORT_T *port)
{
	
}
static void start_current_while_timer(LAC_PORT_T *port, Bool timeout)
{
   if (timeout == SHORT_TIMEOUT)
	   port->current_while = port->system->short_timeout_time;
   else
	   port->current_while = port->system->long_timeout_time;
}

void lac_rx_enter_state (LAC_STATE_MACH_T * this)
 {
   register LAC_PORT_T *port = this->owner.port;
 
   switch (this->State) {
   	case BEGIN:
	case RXM_INITIALIZE:
		port->selected = False;
		port->standby  = False;
		actor_default(port);
		record_default(port);
		LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP, False);
		port->port_moved = False;
		
	 case RXM_PORT_DISABLED:
		 port->matched	= False;
	 	 LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_SYN, False);
		 port->rcvdLacpdu = False;
		 port->current_while = 0;
		 port->selected = False;
		 break;
		 
	 case RXM_LACP_DISABLED:
		 port->selected 				 = False;
		 port->standby					 = False;
		 record_default(port);
		 LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_AGG, False);
		 port->matched					 = True;
	 	break;
		
	 case RXM_EXPIRED:
		 port->matched					  = False;
		 LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_SYN, False);
		 LAC_STATE_SET_BIT(port->partner.state, LAC_STATE_TMT, SHORT_TIMEOUT);
		 start_current_while_timer(port, SHORT_TIMEOUT);
		 LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP, True);
		 break;
		 
		 
	case RXM_DEFAULTED:
		update_default_selected(port);
		record_default(port);
		port->matched			  = True;
		LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP, False);
			
		break;

	case RXM_CURRENT:

		update_selected(port);
		update_ntt(port);
		record_pdu(port);		  
		choose_matched( port);
		start_current_while_timer(port, LAC_STATE_GET_BIT(port->actor.state, LAC_STATE_TMT));		  
		LAC_STATE_SET_BIT(port->actor.state, LAC_STATE_EXP , False);	  
		port->rcvdLacpdu = False;
		break;
   }
 
 }
 
 Bool lac_rx_check_conditions (LAC_STATE_MACH_T * this)
 {
	register LAC_PORT_T *port = this->owner.port;
 
	if (port->port_enabled == False && port->port_moved == False
		&& BEGIN != this->State) {
/*            if (this->debug)                    printf("\r\n port:%d state:%d, port_enable:%d, lacp_enabled:%d", 
                      port->port_index, this->State, port->port_enabled, port->lacp_enabled);*/

		if (this->State == RXM_PORT_DISABLED)
			return False;
		else
		{
			return lac_hop_2_state (this, RXM_PORT_DISABLED);
		}
	}
 
   switch (this->State) {
   	 case BEGIN:
		 return lac_hop_2_state (this, RXM_INITIALIZE);

	 case RXM_PORT_DISABLED:
		if (port->port_enabled == True
			&& port->lacp_enabled == True) 
		 {
		   return lac_hop_2_state (this, RXM_EXPIRED);
		 }
		 if (port->port_enabled == True
		 	&& port->lacp_enabled == False) 
		 {
		   return lac_hop_2_state (this, RXM_LACP_DISABLED);
		 }
		 break;

	 case RXM_LACP_DISABLED:
		if (port->port_enabled == True
			&& port->lacp_enabled == True) 
		 {
		   return lac_hop_2_state (this, RXM_EXPIRED);
		 }
        break;
        
	 case RXM_EXPIRED:
		 if (!port->current_while) 
		 {
		   return lac_hop_2_state (this, RXM_DEFAULTED);
		 }
		 if (port->rcvdLacpdu)
		 {
		   return lac_hop_2_state (this, RXM_CURRENT);
		 }
		 break;
		 
	 case RXM_CURRENT:
//             printf("\r\n current_while:%d, rcvdLacpdu:%d", port->current_while, port->rcvdLacpdu);
             
		 if (!port->current_while && !port->rcvdLacpdu) {
		   return lac_hop_2_state (this, RXM_EXPIRED);
		 }
		 if (port->rcvdLacpdu) {
		   return lac_hop_2_state (this, RXM_CURRENT);
		 }
		 break;

	case RXM_DEFAULTED:
		if (port->rcvdLacpdu)
		{
		  return lac_hop_2_state (this, RXM_CURRENT);
		}
		break;
		
	default:
		break;
   }
 
   return False;
 }

