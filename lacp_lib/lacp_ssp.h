void lacp_trace (const char *format, ...);
int aggregator_add_member(int agg_id, int port_index);
int aggregator_del_member(int port_index);
int aggregator_get_id(int port_index);

int lacp_ssp_tx_pdu (int port_index, unsigned char *bpdu, size_t bpdu_len);
int lac_get_port_cd(int port_index);
int lac_set_port_cd(int port_index, int state);
int lac_get_port_oper_key(int port_index);
int lac_get_port_attach_to_tid(int port_index, int *tid);
int lac_set_port_attach_to_tid(int port_index, Bool attach, int tid);
int lac_set_port_duplex(int port_index, int duplex);
int lac_set_port_speed(int port_index, int speed);
int lac_get_port_oper_duplex(int port_index);
int lac_get_port_oper_speed(int port_index);

void lacp_ssp_get_port_mac (unsigned char *mac);
const char * lacp_ssp_get_port_name (int port_index);
