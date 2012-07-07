#include <stdlib.h>
#include <signal.h>  
#include <execinfo.h>  
#define SIZE 1000  
void *buffer[SIZE];  
int curLogLen = 0;
char log[1000] = {0};
#if 0
void memdump(char *buf, int len)
{
  int i = 0;
  for (i = 0; i< len; i++)
    {
      if (i%8 == 0)
	curLogLen += sprintf(&log[curLogLen], "\r\n");
      curLogLen += sprintf(&log[curLogLen], "\t%2x", buf[i]);
      if (curLogLen >= 1000)
	curLogLen = 0;
    }

  return;
}
#endif
void output_backtrace()
{
  int i, num;
  char **calls;

  num = backtrace(buffer, SIZE);
  calls = backtrace_symbols(buffer, num);
  for (i = 0; i < num; i++)
    printf("%s\n", calls[i]);

  return;
}
void fault_trap(int n,struct siginfo *siginfo,void *myact)  
{  
  int i, num;  
  char **calls;  
  printf("Fault address:%X\n",siginfo->si_addr);     
  num = backtrace(buffer, SIZE);  
  calls = backtrace_symbols(buffer, num);  
  for (i = 0; i < num; i++)  
    printf("%s\n", calls[i]);  

  printf("\r\nlog:%s \r\n", log);
  exit(1);  
}  
void setuptrap()  
{  
  struct sigaction act;  
  sigemptyset(&act.sa_mask);     
  act.sa_flags=SA_SIGINFO;      
  act.sa_sigaction=fault_trap;  
  sigaction(SIGSEGV,&act,NULL);  
} 
