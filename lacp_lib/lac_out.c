#include "lac_base.h"
#include "stdarg.h"
//#include "stdafx.h"

port_attr g_port_list[100];

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
int lac_set_port_speed(int port_index, int speed)
{
        g_port_list[port_index].speed = speed;
        return 0;
        
}
int lac_set_port_duplex(int port_index, int duplex)
{
        g_port_list[port_index].duplex = duplex;
        return 0;
        
}

int lac_set_port_attach_to_tid(int port_index, int attach, int tid)
{
        if(attach)
                g_port_list[port_index].tid = tid;
        else if (tid != g_port_list[port_index].tid)
        {
                printf("\r\n !!!!!!!!! detach from error tid:%d, actually in:%d", tid, g_port_list[port_index].tid);
                return -1;
                
        }
        else
                g_port_list[port_index].tid = 0;

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

}

//HANDLE hMutex;
int lac_get_port_oper_key(int port_index)
{
        int speed, duplex, tid;
        
        lac_get_port_attach_to_tid(port_index, &tid);
        speed = lac_get_port_oper_speed(port_index);
        duplex = lac_get_port_oper_duplex(port_index);
        
        return (tid+speed+duplex);
        

}

int lac_out_init_sem()
{
    //hMutex = CreateMutex(NULL,FALSE,NULL);

    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}
int lac_out_sem_take()
{
    //WaitForSingleObject(hMutex,INFINITE);
    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}

int lac_out_sem_give()
{
    //ReleaseMutex(hMutex);
    //printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
}

void lac_trace (const char *format, ...)
{
#define MAX_MSG_LEN  128
    char msg[MAX_MSG_LEN];
    va_list args;

    va_start (args, format);
    vsprintf (msg, format, args);

    printf("\r\n %s", msg);
    va_end (args);
}


