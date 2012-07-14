#ifndef __LAC_PORT_H_
#define __LAC_PORT_H_


unsigned int lac_port_rx_lacpdu (LAC_PORT_T * this, LACPDU_T * bpdu, size_t len);
int lac_set_port_reselect(LAC_PORT_T *port);
LAC_PORT_T *lac_port_create (LAC_SYS_T * lac_sys, int port_index);
void
lac_port_delete (LAC_PORT_T * this);


#endif
