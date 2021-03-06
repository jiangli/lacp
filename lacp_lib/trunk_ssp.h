void trunk_trace (const char *format, ...);

uint32_t trunk_ssp_tx_pdu (uint32_t port_index, unsigned char *bpdu, size_t bpdu_len);
uint32_t trunk_ssp_get_port_cd(uint32_t port_index);
uint32_t trunk_ssp_set_port_cd(uint32_t port_index, uint32_t state);
uint32_t trunk_ssp_get_port_oper_key(uint32_t port_index);

uint32_t trunk_ssp_attach_port(uint32_t port_index, Bool attach, uint32_t tid);

uint32_t trunk_ssp_get_global_index( uint32_t slot, uint32_t port, uint32_t *port_index);
uint32_t lacp_ssp_change_to_slot_port(uint32_t port_index, uint32_t *slot, uint32_t *port);
uint32_t trunk_ssp_get_port_link_status(uint32_t port_index);

uint32_t trunk_ssp_set_port_duplex(uint32_t port_index, uint32_t duplex);
uint32_t trunk_ssp_set_port_speed(uint32_t port_index, uint32_t speed);
uint32_t trunk_ssp_get_port_oper_duplex(uint32_t port_index);
uint32_t trunk_ssp_get_port_oper_speed(uint32_t port_index);

void trunk_ssp_get_mac (unsigned char *mac);
const char * trunk_ssp_get_port_name (uint32_t port_index);
uint32_t trunk_ssp_rx_lacpdu(uint32_t slot, uint32_t port, lacp_pdu_t * bpdu, uint32_t len);
