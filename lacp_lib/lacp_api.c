#include "lacp_base.h"
/*****************************************************************************
 �� �� ��  : lacp_port_get_actor_admin
 ��������  : ��ȡ�˿ڵ�actor�������ݣ����û�У���ʹ��Ĭ��ֵ
 �������  : int port_index                  
             lacp_port_info_t  *actor_admin  
 �������  : ��
 �� �� ֵ  : unsigned
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2012��7��23��
    ��    ��   : jiangli
    �޸�����   : �����ɺ���

*****************************************************************************/
unsigned int lacp_port_get_actor_admin(int port_index, lacp_port_info_t  *admin)
{
	lacp_mac_t sys_mac;
	
	/* try to get data from db */

	/* if not exist, use default */
    admin->port_priority		   = Default_port_priority;
    admin->port_no		   = port_index + 1;
    admin->system_priority	   = Default_system_priority;
    memcpy(admin->system_mac, sys_mac, 6);
    admin->key				   = Default_key;

    LACP_STATE_SET_BIT(admin->state, LACP_STATE_ACT, Default_lacp_activity);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_TMT, Default_lacp_timeout);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_AGG, Default_aggregation);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_SYN, False);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_DEF, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_EXP, False);

	return 0;
}



/*****************************************************************************
 �� �� ��  : lacp_port_get_partner_admin
 ��������  : ��ȡ�˿ڵ�partner�������ݣ����û�У���ʹ��Ĭ��ֵ
 �������  : int port_index            
             lacp_port_info_t  *admin  
 �������  : ��
 �� �� ֵ  : unsigned
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2012��7��23��
    ��    ��   : jiangli
    �޸�����   : �����ɺ���

*****************************************************************************/
unsigned int lacp_port_get_partner_admin(int port_index, lacp_port_info_t  *admin)
{
	lacp_mac_t sys_mac;
	
	/* try to get data from db */

	/* if not exist, use default */
    admin->port_priority		 = Default_port_priority;
    admin->port_no				 = port_index;/* TODO:: */
    admin->system_priority		 = Default_system_priority;
    memset(admin->system_mac, 0, 6);
    admin->key					 = port_index;

    LACP_STATE_SET_BIT(admin->state, LACP_STATE_ACT, False); /* Passive	  */
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_TMT, False); /* Long timeout */
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_AGG, False); /* Individual   */
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_SYN, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_COL, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_DIS, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_DEF, True);
    LACP_STATE_SET_BIT(admin->state, LACP_STATE_EXP, False);

	return 0;
}

