#include "lac_base.h"
#include "stdarg.h"
//#include "stdafx.h"

const char * LAC_OUT_get_port_name (int port_index)
{
  static char tmp[4];
  sprintf (tmp, "p%02d", (int) port_index);
  return tmp;
}

void LAC_OUT_get_port_mac (unsigned char *mac)
{
	mac[5] = 0xfe;
	return ;
}

void memdump(char *buf, int len)
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
	printf("\r\n %s.%d",  __FUNCTION__, __LINE__);
	memdump(bpdu, bpdu_len);
}

//HANDLE hMutex;

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


