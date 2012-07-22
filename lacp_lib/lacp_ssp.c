#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_ssp.h"
#include "lacp_api.h"
#include "lacp_stub.h"
#include "lacp_util.h"

uint32_t
bridge_tx_bpdu (uint32_t port_index, unsigned char *bpdu, size_t bpdu_len);
char *
UT_sprint_time_stamp (char ticks_accuracy);

static uint32_t g_lacp_debug_rx_tx[18][8][2];
void lacp_dbg_pkt(int slot, int port, int direction, int en)
{
    g_lacp_debug_rx_tx[slot][port-1][direction] = en;
}
int lacp_dbg_get_switch(int slot, int port,int direction)
{
    return g_lacp_debug_rx_tx[slot][port-1][direction];
}
uint32_t lacp_ssp_rx_lacpdu(uint32_t slot, uint32_t port, lacp_pdu_t * pdu, uint32_t len)
{
    uint32_t ret = 0;
    uint32_t port_index;
    /* check message */
    if (!stub_db_port_lacp_is_enable(slot, port))
        return 0;

    ret = lacp_ssp_get_global_index(slot, port, &port_index);
    if (ret != 0)
    {
        return ret;
    }

    if (lacp_dbg_get_switch(port_index,1,0))
        lacp_dump_pkt(pdu, len);

    lacp_rx_lacpdu(port_index, pdu, len);
    return 0;
}

uint32_t lacp_ssp_change_to_slot_port(uint32_t port_index, uint32_t *slot, uint32_t *port)
{
    *slot = port_index / 8;
    *port = port_index % 8 + 1;
    return 0;
}
uint32_t lacp_ssp_get_global_index( uint32_t slot, uint32_t port, uint32_t *port_index)
{
    uint32_t ret = 0;
    if (port == 0)
    {
        ERR_LOG(ret, slot, port, 0);
        return M_LACP_INTERNEL;
    }

    *port_index = slot * 8 + (port - 1) ;
    return 0;
}

const char * lacp_ssp_get_port_name (uint32_t port_index)
{
    uint32_t ret = 0;
    uint32_t slot, port;
    static char tmp[4];

    ret = lacp_ssp_change_to_slot_port(port_index, &slot, &port);
    if (ret == 0)
    {
        sprintf (tmp, "port-%2d/%-2d", slot, port);
    }
    else
    {
        sprintf(tmp, "port-unknown(%d)", port_index);
    }

    return tmp;
}

void lacp_ssp_get_mac (unsigned char *mac)
{
    static long pid = -1;
    static unsigned char mac_beg[] = { '\0', '\0', '\0', '\0', '\0', '\0' };

    if (pid < 0) {
        pid = getpid ();
        memcpy (mac_beg + 1, &pid, 4);
    }
    memcpy (mac, mac_beg, 5);

    return ;
}

uint32_t lacp_ssp_get_port_oper_speed(uint32_t port_index)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    return attr.speed;

}
uint32_t lacp_ssp_get_port_oper_duplex(uint32_t port_index)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    return attr.duplex;
}


uint32_t lacp_ssp_set_port_link_status(uint32_t port_index, uint32_t link_status)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    attr.link_status  = link_status;
    stub_set_port_attr(port_index, &attr);
    return 0;
}

uint32_t lacp_ssp_get_port_link_status(uint32_t port_index)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    return attr.link_status;
}

uint32_t lacp_ssp_set_port_speed(uint32_t port_index, uint32_t speed)
{
    lacp_port_cfg_t uid_cfg;
    port_attr_t attr;

    memset(&uid_cfg, 0, sizeof(lacp_port_cfg_t));
    stub_get_port_attr(port_index, &attr);
    attr.speed  = speed;
    stub_set_port_attr(port_index, &attr);

    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);
    uid_cfg.field_mask = PT_CFG_COST;

    lacp_port_set_cfg(&uid_cfg);

    return 0;

}
uint32_t lacp_ssp_set_port_duplex(uint32_t port_index, uint32_t duplex)
{
    lacp_port_cfg_t uid_cfg;
    port_attr_t attr;
    memset(&uid_cfg, 0, sizeof(lacp_port_cfg_t));

    stub_get_port_attr(port_index, &attr);
    attr.duplex = duplex;
    stub_set_port_attr(port_index, &attr);

    lacp_bitmap_set_bit(&uid_cfg.port_bmp, port_index);
    uid_cfg.field_mask = PT_CFG_COST;

    lacp_port_set_cfg(&uid_cfg);
    return 0;

}

uint32_t lacp_ssp_attach_port(uint32_t port_index, Bool attach, uint32_t tid)
{
    if(attach)
    {
        port_attr_t attr;
        stub_get_port_attr(port_index, &attr);
        attr.tid = tid;
        stub_set_port_attr(port_index, &attr);
    }

    else
    {
        port_attr_t attr;
        stub_get_port_attr(port_index, &attr);
        attr.tid = 0;
        stub_set_port_attr(port_index, &attr);
    }

    return 0;
}

uint32_t lacp_ssp_get_port_attach_tid(uint32_t port_index, uint32_t *tid)
{

    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    *tid = attr.tid;
    return 0;

}
uint32_t lacp_ssp_get_port_cd(uint32_t port_index)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    return attr.cd;

}

uint32_t lacp_ssp_set_port_cd(uint32_t port_index, uint32_t state)
{
    port_attr_t attr;

    //TODO:: read rstp port state, &
    stub_get_port_attr(port_index, &attr);
    attr.cd = state;
    stub_set_port_attr(port_index, &attr);

    return 0;

}


uint32_t lacp_ssp_tx_pdu (uint32_t port_index, unsigned char *pdu, size_t len)
{
    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
//	memdump(bpdu, bpdu_len);
    bridge_tx_bpdu(port_index, pdu, len);
    if (lacp_dbg_get_switch(port_index,1,1))
        lacp_dump_pkt(pdu, len);

    return 0;

}

uint32_t lacp_ssp_get_speed_index(uint32_t speed, uint32_t duplex)
{
    if (!duplex)
        return 0;

    switch(speed)
    {
    case 10:
        return 1;
    case 100:
        return 2;
    case 1000:
        return 3;
    case 10000:
        return 4;
    default:
        return 0;
    }
}

//HANDLE hMutex;
uint32_t lacp_ssp_get_port_oper_key(uint32_t port_index)
{
    uint32_t speed, duplex, tid;
    uint32_t slot, port;

    lacp_ssp_change_to_slot_port(port_index, &slot, &port);

    tid = stub_db_agg_get_port_tid(slot, port);
    if (tid == 0)
        return 0;
    speed = lacp_ssp_get_port_oper_speed(port_index);
    duplex = lacp_ssp_get_port_oper_duplex(port_index);

    return (tid << 8 | lacp_ssp_get_speed_index(speed,duplex));


}

uint32_t lacp_ssp_out_init_sem()
{
    //hMutex = CreateMutex(NULL,FALSE,NULL);


    return 0;

}
uint32_t lacp_ssp_out_sem_take()
{
    //WaitForSingleObject(hMutex,INFINITE);

    return 0;

}

uint32_t lacp_ssp_out_sem_give()
{

    return 0;

}

void lacp_trace (const char *format, ...)
{
#define MAX_MSG_LEN  128
    char msg[MAX_MSG_LEN];
    va_list args;

    va_start (args, format);
    vsprintf (msg, format, args);

    printf("\r\n[%s]%s", UT_sprint_time_stamp (0), msg);
    va_end (args);
}


