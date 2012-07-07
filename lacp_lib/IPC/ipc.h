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

#ifndef _IPC_SOCKET_H__ 
#define _IPC_SOCKET_H__

/* Socket API */

#include        <sys/socket.h>  /* basic socket definitions */
#include        <netinet/in.h>
#include        <linux/un.h>              /* for Unix domain sockets */

#define IPC_REPL_PATH   "/tmp/IPC_SocketFile"

typedef struct sockaddr SA;

#define SOCKET_NAME_LENGTH 108
#define SIZE_OF_ADDRESS sizeof(struct sockaddr_un)

typedef enum {
  IPC_BIND_AS_CLIENT,
  IPC_BIND_AS_SERVER
} TYPE_OF_BINDING;


typedef char        IPC_SOCK_ID[SOCKET_NAME_LENGTH];

typedef struct{
  int           sock_fd;
  struct sockaddr_un    clientAddr;
  struct sockaddr_un    serverAddr; // Only for socket of IPC_BIND_AS_CLIENT
  IPC_SOCK_ID       socket_id;
  TYPE_OF_BINDING   binding;
} IPC_SOCKET_T;

#define MESSAGE_SIZE        2048

int IPC_SocketInit(IPC_SOCKET_T* sock,
            IPC_SOCK_ID id,
            TYPE_OF_BINDING binding);

int IPC_SocketRecvfrom (IPC_SOCKET_T* sock,
            void* msg_buffer,
            int buffer_size,
            IPC_SOCKET_T* sock_4_reply);

int IPC_SocketSendto (IPC_SOCKET_T* sock,
            void* msg_buffer,
            int buffer_size);

int IPC_SocketClose(IPC_SOCKET_T* sock);

int IPC_SocketSetReadTimeout (IPC_SOCKET_T* s, int timeout);

int
IPC_SocketCompare (IPC_SOCKET_T* s, IPC_SOCKET_T* t);

#define GET_FILE_DESCRIPTOR(sock) (((IPC_SOCKET_T*)sock)->sock_fd)

#endif /* _IPC_SOCKET_H__ */


