#include <stdio.h>
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



#define LOG_MAX_NUM 200
#define LOG_FILENAME_LEN 15

typedef struct tag_log_record
{
    int tick;
    char filename[LOG_FILENAME_LEN + 1];
    short line_no;
    int ret;
    int para1;
    int para2;
    int para3;
    int count;
} _LOG_RECORD;

_LOG_RECORD g_lastword[LOG_MAX_NUM];

static int cur_i = 0;

void str_trip(char *src, char *dst, int len)
{
    if (strlen(src) <= len)
    {
        strncpy(dst, src, len);
    }
    else
    {
        strncpy(dst, src + (strlen(src) - len), len);
    }
}

int get_log_index(char *filename, int line)
{
    int i;
    for(i=0; i<cur_i; i++)
    {
        if(strncmp(filename,g_lastword[i].filename, LOG_FILENAME_LEN)== 0
                && g_lastword[i].line_no == line)
        {
            return i;
        }
    }
    return -1;
}


void write_log(char *filename, int line, int ret,
               int para1, int para2, int para3)
{
    int index = cur_i;
    char fileshort[LOG_FILENAME_LEN + 1]  = {0};
    str_trip(filename, fileshort, LOG_FILENAME_LEN);

    index = get_log_index(fileshort, line);
    if (index == -1)
    {
        index = cur_i;
        cur_i = (cur_i + 1) % LOG_MAX_NUM;
        g_lastword[index].count = 1;
    }
    else
    {
        g_lastword[index].count++;
    }
    g_lastword[index].tick = 1;//(int)tickGet();
    strncpy(g_lastword[index].filename, fileshort, LOG_FILENAME_LEN);
    g_lastword[index].line_no = line;
    g_lastword[index].ret = ret;
    g_lastword[index].para1 = para1;
    g_lastword[index].para2 = para2;
    g_lastword[index].para3 = para3;
    printf("\r\n error log :<%s.%d>%d,%d,%d,%d\r\n", filename, line, ret, para1, para2, para3);

}


void show_log()
{
    int i;
    printf("tick     file            line      ret     para1     para2     para3 count\r\n");
    for(i=0; i<LOG_MAX_NUM; i++)
    {
        if (g_lastword[i].tick != 0)
            printf("%8u %15s %4d %8x %8x %8x %8x %4u\r\n",
                   g_lastword[i].tick,
                   g_lastword[i].filename,
                   g_lastword[i].line_no,
                   g_lastword[i].ret,
                   g_lastword[i].para1,
                   g_lastword[i].para2,
                   g_lastword[i].para3,
                   g_lastword[i].count);
    }
}

void clear_log()
{
    cur_i = 0;
    memset(g_lastword, 0, sizeof(g_lastword));
}

