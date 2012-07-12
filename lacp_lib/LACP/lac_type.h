typedef unsigned short	LAC_PORT_ID;
typedef unsigned short	LAC_KEY;
typedef int Time;
#define LAC_TIME_T unsigned short
typedef int      SYSTEM_PRIORITY;

#define LAC_MAC_LEN 6

typedef unsigned char	SYSTEM_MAC[LAC_MAC_LEN];

typedef struct {
        int speed;
        int duplex;
        int cd;
        int tid;
        
}port_attr;
