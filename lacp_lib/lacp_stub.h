typedef struct {
    int speed;
    int duplex;
    int cd;
    int tid;
    int link_status;
} port_attr_t;

typedef struct
{
    int cnt;

    int ports[8];
} LINK_GROUP_T;

