#include "lacp_base.h"
#include "lacp_statmch.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "trunk_ssp.h"

#define STATES {        \
  CHOOSE(TXM_NO_PERIODIC),    \
  CHOOSE(TXM_PERIODIC_TX),    \
  CHOOSE(TXM_IDLE),         \
  CHOOSE(TXM_TX),  \
}

#define GET_STATE_NAME lacp_tx_get_state_name
#include "lacp_choose.h"

static lacp_pdu_t lacpdu_packet;

static uint32_t
tx_lacpdu(lacp_state_mach_t * fsm)
{
    register uint32_t port_index;
    register lacp_port_t *port = fsm->owner.port;
    const unsigned char slow_protocols_address[] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02};

    if (!port->lacp_enabled)
    {
        if (fsm->debug)
            trunk_trace("\r\n port %d 's lacp is not enable, cann't tx lacpdu.", port->port_index);

        return M_LACP_NOT_ENABLE;
    }

    port_index = port->port_index;

    memset(&lacpdu_packet, 0, sizeof(lacp_pdu_t));

    memcpy(lacpdu_packet.slow_protocols_address, slow_protocols_address, 6);
    trunk_ssp_get_mac (lacpdu_packet.src_address);
    lacpdu_packet.ethertype = htons (0x8809);

    lacpdu_packet.protocol_subtype = 1;
    lacpdu_packet.protocol_version = 1;

    /* actor tlv */
    lacpdu_packet.type_actor = 1;
    lacpdu_packet.len_actor = 20;
    lacpdu_packet.actor.system_priority = htons(port->actor.system_priority);
    memcpy(lacpdu_packet.actor.system_mac, port->actor.system_mac, 6);
    lacpdu_packet.actor.key = htons(port->actor.key);
    lacpdu_packet.actor.port_priority = htons(port->actor.port_priority);
    lacpdu_packet.actor.port_no = htons(port->actor.port_no);
    lacpdu_packet.actor.state = port->actor.state;

    /* partner tlv */
    lacpdu_packet.type_partner = 2;
    lacpdu_packet.len_partner = 20;
    lacpdu_packet.partner.system_priority = htons(port->partner.system_priority);
    memcpy(lacpdu_packet.partner.system_mac, port->partner.system_mac, 6);
    lacpdu_packet.partner.key = htons(port->partner.key);
    lacpdu_packet.partner.port_priority = htons(port->partner.port_priority);
    lacpdu_packet.partner.port_no = htons(port->partner.port_no);
    lacpdu_packet.partner.state = port->partner.state;

    lacpdu_packet.type_collector = 3;
    lacpdu_packet.len_collector = 16;
    lacpdu_packet.collector_max_delay = htons(10);

    /* statictic */
    port->tx_lacpdu_cnt++;

    /* tx */
    return trunk_ssp_tx_pdu (port_index, (unsigned char *) &lacpdu_packet, sizeof(lacp_pdu_t));
}


void lacp_tx_enter_state (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    switch (fsm->state) {
    case LACP_BEGIN:
    case TXM_NO_PERIODIC:
        port->periodic_timer = 0;
        port->hold_count = 0;
        break;

    case TXM_PERIODIC_TX:
        port->ntt = True;
        if (LACP_STATE_GET_BIT(port->partner.state, LACP_STATE_TMT) == LACP_LONG_TIMEOUT)
        {
            port->periodic_timer = port->system->slow_periodic_time;
        }
        else
        {
            port->periodic_timer = port->system->fast_periodic_time;
        }
        break;

    case TXM_TX:
        tx_lacpdu (fsm);
        port->hold_count++;
        port->ntt = False;
        break;
    };
}

Bool lacp_tx_check_conditions (lacp_state_mach_t * fsm)
{
    register lacp_port_t *port = fsm->owner.port;

    if (LACP_BEGIN == fsm->state || !port->lacp_enabled || !port->port_enabled
            || ((LACP_STATE_GET_BIT(port->actor.state, LACP_STATE_ACT) == LACP_PASSIVE )
                && (LACP_STATE_GET_BIT(port->partner.state, LACP_STATE_ACT) == LACP_PASSIVE)))
    {
        if (TXM_NO_PERIODIC == fsm->state)
        {
            return False;
        }
        else
        {
            return lacp_hop_2_state (fsm, TXM_NO_PERIODIC);
        }
    }

    switch (fsm->state) {
    case TXM_NO_PERIODIC:
        return lacp_hop_2_state (fsm, TXM_IDLE);

    case TXM_PERIODIC_TX:
        return lacp_hop_2_state (fsm, TXM_IDLE);

    case TXM_IDLE:
        if (!port->periodic_timer)
        {
            return lacp_hop_2_state (fsm, TXM_PERIODIC_TX);
        }

        if (port->ntt && port->hold_count < port->system->tx_hold_count)
        {
            return lacp_hop_2_state (fsm, TXM_TX);
        }
        break;

    case TXM_TX:
        return lacp_hop_2_state (fsm, TXM_IDLE);

    };
    return False;
}


/*
static lacp_pdu_t lacpdu_packet = {
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
