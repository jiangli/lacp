#include "lacp_stub.h"
port_attr_t g_port_list[144];
LINK_GROUP_T g_link_groups[32];

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

Bool aggregator_has_member(int agg_id)
{
	if (g_link_groups[agg_id - 1].cnt)
		return True;
	else
		return False;
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

int stub_get_port_attr(int port_index, port_attr_t *attr)
{
	memcpy(attr, g_port_list[port_index], sizeof(port_attr_t));
	return 0;
}

int stub_set_port_attr(int port_index, port_attr_t *attr)
{
	memcpy(attr, g_port_list[port_index], sizeof(port_attr_t));
	return 0;
}
	
