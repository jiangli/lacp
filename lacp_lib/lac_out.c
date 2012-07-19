#include "lac_base.h"
#include "stdarg.h"
#include "lac_port.h"
#include "lac_sys.h"
#include "uid_lac.h"
#include "lac_in.h"

port_attr g_port_list[144];
LINK_GROUP_T g_link_groups[32];
int
bridge_tx_bpdu (int port_index, unsigned char *bpdu, size_t bpdu_len);
char *
UT_sprint_time_stamp (char ticks_accuracy);


int aggregator_init()
{
    int i;
    memset(g_link_groups, 0xff, sizeof(g_link_groups));
    for (i = 0; i < 32; i++)
        g_link_groups[i].cnt = 0;

    for (i = 0; i < sizeof(g_port_list)/sizeof(g_port_list[0]); i++)
    {
        g_port_list[i].speed = 1000;
        g_port_list[i].duplex = 1;
        g_port_list[i].cd = 1;
    }

    return 0;

}

int aggregator_add_member(int agg_id, int port_index)
{
    int i;
    for (i=0; i<8; i++)
    {
        if (g_link_groups[agg_id - 1].ports[i] == 0xffffffff)
        {
            g_link_groups[agg_id - 1].ports[i] = port_index;
            g_link_groups[agg_id - 1].cnt ++;
            return 0;

        }

    }
    printf("\r\n agg %d full !", agg_id);
    return -1 ;


}
int aggregator_del_member(int port_index)
{
    int i;
    int j;
    for (j=0; j<32; j++)
    {
        for (i=0; i<8; i++)
        {
            if (g_link_groups[j].ports[i] == port_index)
            {
                g_link_groups[j].ports[i] = 0xffffffff;
                g_link_groups[j].cnt --;
                return 0;

            }

        }
    }
    printf("\r\n not found port:%d in all agg!",  port_index);
    return -1 ;


}
int aggregator_get_id(int port_index)
{
    int i,j;
    for (i=0; i<32; i++)
    {
        for (j=0; j<8; j++)
            if (g_link_groups[i].ports[j] == port_index)
            {
                return i+1;
            }
    }
    return 0;



}

const char * LAC_OUT_get_port_name (int port_index)
{
    static char tmp[4];
    sprintf (tmp, "p%02d", (int) port_index);
    return tmp;
}

void LAC_OUT_get_port_mac (unsigned char *mac)
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
int lac_get_port_oper_speed(int port_index)
{
    return g_port_list[port_index].speed;

}
int lac_get_port_oper_duplex(int port_index)
{
    return g_port_list[port_index].duplex;
}


int lac_set_port_link_status(int port_index, int link_status)
{
    g_port_list[port_index].link_status  = link_status;
}
int lac_get_port_link_status(int port_index)
{
    return g_port_list[port_index].link_status;
}
int lac_set_port_speed(int port_index, int speed)
{
    UID_LAC_PORT_CFG_T uid_cfg;

    g_port_list[port_index].speed = speed;
    BitmapSetBit(&uid_cfg.port_bmp, port_index);
    uid_cfg.field_mask = PT_CFG_COST;

    lac_port_set_cfg(&uid_cfg);

    return 0;

}
int lac_set_port_duplex(int port_index, int duplex)
{
    UID_LAC_PORT_CFG_T uid_cfg;
    g_port_list[port_index].duplex = duplex;
    BitmapSetBit(&uid_cfg.port_bmp, port_index);
    uid_cfg.field_mask = PT_CFG_COST;

    lac_port_set_cfg(&uid_cfg);
    return 0;

}

int lac_set_port_attach_to_tid(int port_index, Bool attach, int tid)
{
    if(attach)
    {
        g_port_list[port_index].tid = tid;
        printf("\r\n attach %d --> %d!!",  port_index, tid);

    }

    else //if (tid != g_port_list[port_index].tid)
    {
//                printf("\r\n !!!!!!!!! detach from error tid:%d, actually in:%d", tid, g_port_list[port_index].tid);

        g_port_list[port_index].tid = 0;
    }


    return 0;
}
int lac_get_port_attach_to_tid(int port_index, int *tid)
{
    *tid = g_port_list[port_index].tid;
    return 0;

}
int lac_get_port_cd(int port_index)
{
    return g_port_list[port_index].cd;

}
int lac_set_port_cd(int port_index, int state)
{
    g_port_list[port_index].cd = state;

    return 0;

}


void memdump(unsigned char *buf, int len)
{
    int i = 0;
    printf("\r\n buf:0x%x, len:%d\r\n---------\r\n", (unsigned int)buf, len);
    for (i = 0; i < len; i++)
    {
        if (i%16 == 0)
            printf("\r\n");
        printf(" %02x", *(buf + i));
    }

    printf("\r\n---------\r\n");

}

int LAC_OUT_tx_bpdu (int port_index, unsigned char *bpdu, size_t bpdu_len)
{
    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
//	memdump(bpdu, bpdu_len);
    bridge_tx_bpdu(port_index, bpdu, bpdu_len);
    return 0;

}

int get_index(int speed, int duplex)
{
    int speed_duplex[] = {1,10,100,1000,10000};

    int i;
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
int lac_get_port_oper_key(int port_index)
{
    int speed, duplex, tid;
    tid = aggregator_get_id(port_index);
    speed = lac_get_port_oper_speed(port_index);
    duplex = lac_get_port_oper_duplex(port_index);

    return (tid << 8 | get_index(speed,duplex));


}

int lac_out_init_sem()
{
    //hMutex = CreateMutex(NULL,FALSE,NULL);

    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
    return 0;

}
int lac_out_sem_take()
{
    //WaitForSingleObject(hMutex,INFINITE);
    printf("\r\n <%s.%d>",  __FUNCTION__, __LINE__);
    return 0;

}

int lac_out_sem_give()
{
    printf("\r\n <%s.%d>",  __FUNCTION__, __LINE__);
    return 0;

}

void lac_trace (const char *format, ...)
{
#define MAX_MSG_LEN  128
    char msg[MAX_MSG_LEN];
    va_list args;

    va_start (args, format);
    vsprintf (msg, format, args);

    printf("\r\n[%s]%s", UT_sprint_time_stamp (0), msg);
    va_end (args);
}


