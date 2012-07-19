#include "lacp_base.h"
#include "lacp_port.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "lacp_stub.h"

uint32_t
bridge_tx_bpdu (uint32_t port_index, unsigned char *bpdu, size_t bpdu_len);
char *
UT_sprint_time_stamp (char ticks_accuracy);

uint32_t lacp_ssp_change_to_slot_port(uint32_t port_index, uint32_t *slot, uint32_t *port)
{
    *slot = port_index / 8;
    *port = port_index % 8 + 1;
    return 0;
}
uint32_t lacp_ssp_get_global_index( uint32_t slot, uint32_t port, uint32_t *port_index)
{
    if (port == 0)
        return M_LACP_INTERNEL;

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
        sprintf (tmp, "port-%2d/%2d", slot, port);
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

        printf("\r\n attach %d --> %d!!",  port_index, tid);

    }

    else
    {
        printf("\r\n !!!!!!!!! detach  tid:%d, ", tid);
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

uint32_t lacp_set_port_cd(uint32_t port_index, uint32_t state)
{
    port_attr_t attr;
    stub_get_port_attr(port_index, &attr);
    attr.cd = state;
    stub_set_port_attr(port_index, &attr);

    return 0;

}

void memdump(unsigned char *buf, uint32_t len)
{
    uint32_t i = 0;
    printf("\r\n buf:0x%x, len:%d\r\n---------\r\n", (uint32_t)buf, len);
    for (i = 0; i < len; i++)
    {
        if (i%16 == 0)
            printf("\r\n");
        printf(" %02x", *(buf + i));
    }

    printf("\r\n---------\r\n");

}

uint32_t lacp_ssp_tx_pdu (uint32_t port_index, unsigned char *bpdu, size_t bpdu_len)
{
    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
//	memdump(bpdu, bpdu_len);
    bridge_tx_bpdu(port_index, bpdu, bpdu_len);
    return 0;

}

uint32_t lacp_ssp_get_speed_index(uint32_t speed, uint32_t duplex)
{
    uint32_t speed_duplex[] = {1,10,100,1000,10000};

    uint32_t i;
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
    tid = aggregator_get_id(port_index);
    speed = lacp_ssp_get_port_oper_speed(port_index);
    duplex = lacp_ssp_get_port_oper_duplex(port_index);

    return (tid << 8 | lacp_ssp_get_speed_index(speed,duplex));


}

uint32_t lacp_ssp_out_init_sem()
{
    //hMutex = CreateMutex(NULL,FALSE,NULL);

    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
    return 0;

}
uint32_t lacp_ssp_out_sem_take()
{
    //WaitForSingleObject(hMutex,INFINITE);
    printf("\r\n <%s.%d>",  __FUNCTION__, __LINE__);
    return 0;

}

uint32_t lacp_ssp_out_sem_give()
{
    printf("\r\n <%s.%d>",  __FUNCTION__, __LINE__);
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


