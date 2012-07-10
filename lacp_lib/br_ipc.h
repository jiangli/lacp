/************************************************************************ 
 * RSTP library - Rapid Spanning Tree (802.1t, 802.1w) 
 * Copyright (C) 2001-2003 Optical Access 
 * Author: Alex Rozin 
 * 
 * This file is part of RSTP library. 
 * 
 * RSTP library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation; version 2.1 
 * 
 * RSTP library is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser 
 * General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with RSTP library; see the file COPYING.  If not, write to the Free 
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 * 02111-1307, USA. 
 **********************************************************************/

#include "ipc.h"


typedef enum {
  BR_IPC_CNTRL = 0,
  BR_IPC_BPDU
} BR_IPC_CMD_TYPE_T;

typedef enum {
  BR_IPC_PORT_CONNECT,
  BR_IPC_PORT_DISCONNECT,
  BR_IPC_BRIDGE_SHUTDOWN,
  BR_IPC_BRIDGE_HANDSHAKE,
  BR_IPC_LAST_DUMMY
} BR_IPC_CNTRL_CMD_T;

typedef struct br_ipc_port_control_s {
  BR_IPC_CNTRL_CMD_T cmd;
  unsigned long  param1;  
  unsigned long  param2;  
} BR_IPC_CNTRL_BODY_T;

typedef struct br_ipc_msg_header_s {
  BR_IPC_CMD_TYPE_T    cmd_type;
  long          sender_pid;
  int           destination_port;
  int           source_port;
  size_t        body_len;
} BR_IPC_MSG_HEADER_T;

typedef struct br_ipc_msg_s {
  BR_IPC_MSG_HEADER_T  header;
  union {
    BR_IPC_CNTRL_BODY_T    cntrl;
    char bpdu[500];
  } body;

} BR_IPC_MSG_T;

#define MAX_BR_IPC_MSG_SIZE    sizeof(BR_IPC_MSG_T)

