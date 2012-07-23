typedef struct {
    uint32_t speed;
    uint32_t duplex;
    uint32_t cd;
    uint32_t tid;
    uint32_t link_status;
} port_attr_t;

typedef struct
{
    uint32_t cnt;

    uint32_t ports[8];
} LINK_GROUP_T;

