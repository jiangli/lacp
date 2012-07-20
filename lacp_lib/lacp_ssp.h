void lacp_trace (const char *format, ...);

uint32_t lacp_ssp_tx_pdu (uint32_t port_index, unsigned char *bpdu, size_t bpdu_len);
uint32_t lacp_ssp_get_port_cd(uint32_t port_index);
uint32_t lacp_ssp_set_port_cd(uint32_t port_index, uint32_t state);
uint32_t lacp_ssp_get_port_oper_key(uint32_t port_index);
uint32_t lacp_ssp_ssp_attach_port(uint32_t port_index, uint32_t *tid);

uint32_t lacp_ssp_set_port_duplex(uint32_t port_index, uint32_t duplex);
uint32_t lacp_ssp_set_port_speed(uint32_t port_index, uint32_t speed);
uint32_t lacp_ssp_get_port_oper_duplex(uint32_t port_index);
uint32_t lacp_ssp_get_port_oper_speed(uint32_t port_index);

void lacp_ssp_get_port_mac (unsigned char *mac);
const char * lacp_ssp_get_port_name (uint32_t port_index);
