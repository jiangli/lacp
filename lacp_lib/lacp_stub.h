
typedef struct {
    uint32_t slot;
    uint32_t port;
    uint32_t speed;
    uint32_t duplex;
    uint32_t cd;
    uint32_t tid;
    uint32_t link_status;
    uint32_t prio;
} port_attr_t;

typedef struct
{
    uint32_t cnt;

    uchar_t ports[19];
} LINK_GROUP_T;

#define BCM_HWW_TRUNK_SET_PBMP(slot, port, pbmpArray) \
        {\
        uint16_t byteIndex = slot;\
        uint16_t bitIndex  = (port - 1);\
        pbmpArray[byteIndex] |= (1 << bitIndex);\
        }

#define BCM_HWW_TRUNK_CLEAR_PBMP(slot, port, pbmpArray) \
        {\
        uint16_t byteIndex = slot;\
        uint16_t bitIndex  = (port - 1);\
        pbmpArray[byteIndex] &= (~(1 << bitIndex));\
        }

#define BCM_HWW_TRUNK_GET_PBMP(slot, port, pbmpArray) \
        ((pbmpArray[slot]) & (1U << ((port - 1))))

uint32_t stub_db_agg_add_member(uint32_t agg_id, uint32_t slot, uint32_t port);
uint32_t stub_db_agg_del_member(uint32_t slot, uint32_t port);
uint32_t stub_db_agg_get_port_tid(uint32_t slot, uint32_t port);
