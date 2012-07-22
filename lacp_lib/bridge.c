
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <readline/readline.h>

#include "cli.h"
#include "br_ipc.h"

#include "lacp_base.h"
#include "lacp_port.h"
#include "bitmap.h"
#include "lacp_sys.h"
#include "lacp_api.h"
#include "trunk_api.h"

long my_pid = 0;
lacp_bitmap_t enabled_ports;
IPC_SOCKET_T ipc_socket;
extern void setuptrap();
int
bridge_tx_bpdu (int port_index, unsigned char *bpdu, size_t bpdu_len)
{
    BR_IPC_MSG_T msg;

    msg.header.sender_pid = my_pid;
    msg.header.cmd_type = BR_IPC_BPDU;
    msg.header.source_port = port_index;
    msg.header.body_len = bpdu_len;
    memcpy (&msg.body, bpdu, bpdu_len);
    IPC_SocketSendto (&ipc_socket, &msg, sizeof (BR_IPC_MSG_T));
    return 0;
}

int
bridge_start ()
{
    BR_IPC_MSG_T msg;

    register int iii;

    int number_of_ports = 4;
    lacp_sys_cfg_t uid_cfg;

    rl_callback_handler_install (get_prompt (), rl_read_cli);

    if (0 != IPC_SocketInit (&ipc_socket, IPC_REPL_PATH, IPC_BIND_AS_CLIENT)) {
        printf ("FATAL: can't init the connection\n");
        exit (-3);
    }

    /* send HANDSHAKE */
    msg.header.sender_pid = my_pid;
    msg.header.cmd_type = BR_IPC_CNTRL;
    msg.body.cntrl.cmd = BR_IPC_BRIDGE_HANDSHAKE;
    msg.body.cntrl.param1 = number_of_ports;
    iii = IPC_SocketSendto (&ipc_socket, &msg, sizeof (BR_IPC_MSG_T));
    if (iii < 0) {
        printf ("can't send HANDSHAKE: %s\n", strerror (errno));
        printf ("May be 'mngr' is not alive ? :(\n");
        return (-4);
    }

    lacp_cli_init ();


    /* 初始化lac系统 */
    lacp_sys_init();
    lacp_bitmap_clear (&enabled_ports);
    lacp_bitmap_clear (&uid_cfg.ports);

    /* 协议栈默认没有端口，需要初始化端口 */
    return 0;
}

void
bridge_shutdown (void)
{
    BR_IPC_MSG_T msg;
//    int rc;

    /* send SHUTDOWN */
    msg.header.sender_pid = my_pid;
    msg.header.cmd_type = BR_IPC_CNTRL;
    msg.body.cntrl.cmd = BR_IPC_BRIDGE_SHUTDOWN;
    IPC_SocketSendto (&ipc_socket, &msg, sizeof (BR_IPC_MSG_T));

#if 0
    rc = STP_IN_stpm_delete (0);
    if (0 != rc) {
        printf ("FATAL: can't delete:%s\n", STP_IN_get_error_explanation (rc));
        exit (1);
    }
#endif

}

char *
get_prompt (void)
{
    static char prompt[MAX_CLI_PROMT];
    snprintf (prompt, MAX_CLI_PROMT - 1, "%s B%ld > ", UT_sprint_time_stamp (0),
              my_pid);
    return prompt;
}

int
bridge_control (int port_index, BR_IPC_CNTRL_BODY_T * cntrl)
{
    uint32_t slot;
    uint32_t port;

    switch (cntrl->cmd) {
    case BR_IPC_PORT_CONNECT:
        printf ("connected port p%02d\n", port_index);
        lacp_bitmap_set_bit (&enabled_ports, port_index);
        //lacp_port_link_change (port_index, True);
        lacp_ssp_change_to_slot_port(port_index, &slot, &port);
        trunk_port_link_change(slot,  port, True);
        lacp_ssp_set_port_link_status(port_index, 1);
        break;
    case BR_IPC_PORT_DISCONNECT:
        printf ("disconnected port p%02d\n", port_index);
        lacp_bitmap_clear_bit (&enabled_ports, port_index);
        lacp_ssp_change_to_slot_port(port_index, &slot, &port);
        trunk_port_link_change(slot,  port, False);
        lacp_ssp_set_port_link_status(port_index, 0);
        break;
    case BR_IPC_BRIDGE_SHUTDOWN:
        printf ("shutdown from manager :(\n");
        return 1;
    default:
        printf ("Unknown control command <%d> for port %d\n",
                cntrl->cmd, port_index);
    }
    return 0;
}

int
bridge_rx_bpdu (BR_IPC_MSG_T * msg, size_t msgsize, int number_of_ports)
{
    uint32_t slot, port;

    lacp_ssp_change_to_slot_port(msg->header.destination_port, &slot, &port);
    lacp_ssp_rx_lacpdu (slot, port, (lacp_pdu_t *) (msg->body.bpdu),
                        msg->header.body_len);

    return 0;
}

char
read_ipc (IPC_SOCKET_T * ipc_sock, int number_of_ports)
{
    char buff[MAX_BR_IPC_MSG_SIZE];
    BR_IPC_MSG_T *msg;
    size_t msgsize;
    int rc;

    msgsize = IPC_SocketRecvfrom (ipc_sock, buff, MAX_BR_IPC_MSG_SIZE, 0);
    if (msgsize <= 0) {
        printf ("Something wrong in UIF ?\n");
        return 0;
    }

    msg = (BR_IPC_MSG_T *) buff;
    switch (msg->header.cmd_type) {
    case BR_IPC_CNTRL:
        rc = bridge_control (msg->header.destination_port, &msg->body.cntrl);
        break;
    case BR_IPC_BPDU:
        rc = bridge_rx_bpdu (msg, msgsize, number_of_ports);
        break;
    default:
        printf ("Unknown message type %d\n", (int) msg->header.cmd_type);
        rc = 0;
    }

    return rc;
}

char shutdown_flag = 0;

int
main_loop ()
{
    fd_set readfds;
    struct timeval tv;
    struct timeval now, earliest;
    int rc, numfds, sock, kkk;


    sock = GET_FILE_DESCRIPTOR (&ipc_socket);

    gettimeofday (&earliest, NULL);
    earliest.tv_sec++;

    do {
        numfds = -1;
        FD_ZERO (&readfds);

        kkk = 0;			/* stdin for commands */
        FD_SET (kkk, &readfds);
        if (kkk > numfds)
            numfds = kkk;

        FD_SET (sock, &readfds);
        if (sock > numfds)
            numfds = sock;

        if (numfds < 0)
            numfds = 0;
        else
            numfds++;

        gettimeofday (&now, 0);
        tv.tv_usec = 0;
        tv.tv_sec = 0;

        if (now.tv_sec < earliest.tv_sec) {	/* we must wait more than 1 sec. */
            tv.tv_sec = 1;
            tv.tv_usec = 0;
        } else if (now.tv_sec == earliest.tv_sec) {
            if (now.tv_usec < earliest.tv_usec) {
                if (earliest.tv_usec < now.tv_usec)
                    tv.tv_usec = 0;
                else
                    tv.tv_usec = earliest.tv_usec - now.tv_usec;
            }
        }
        //printf ("wait %ld-%ld\n", (long) tv.tv_sec, (long) tv.tv_usec);
        rc = select (numfds, &readfds, NULL, NULL, &tv);
        if (rc < 0) {		// Error
            if (EINTR == errno)
                continue;		// don't break
            printf ("FATAL_MODE:select failed: %s\n", strerror (errno));
            return -2;
        }

        if (!rc) {			// Timeout expired
            lacp_one_second ();
            gettimeofday (&earliest, NULL);

            earliest.tv_sec++;
            continue;
        }

        if (FD_ISSET (0, &readfds)) {
            rl_callback_read_char ();
        }

        if (FD_ISSET (sock, &readfds)) {
            shutdown_flag |= read_ipc (&ipc_socket, 144);
        }

    } while (!shutdown_flag);
    return 0;
}

int
main (int argc, char **argv)
{
    int number_of_ports = 4;
    rl_init ();
    setuptrap();
    if (argc > 1) {
        number_of_ports = atoi (argv[1]);
        if (number_of_ports < 1 || number_of_ports > MAX_NUMBER_OF_PORTS) {
            number_of_ports = 4;
            printf ("Invalid number of ports %s, %d assumed\n",
                    argv[1], number_of_ports);
        }
    }

    my_pid = getpid ();
    printf ("my pid: %ld\n", my_pid);

    if (0 == bridge_start (number_of_ports)) {
        main_loop ();
    }

    bridge_shutdown ();

    rl_shutdown ();

    return 0;
}

#if 0
int lacp_start()
{
    lacp_sys_cfg_t uid_cfg;
    lacp_bitmap_t Ports;
    int max_valid_port = 144;
    int i = 0;

    /* 初始化lac系统 */
    lacp_sys_init();

    /* 协议栈默认没有端口，需要初始化端口 */
    uid_cfg.field_mask = BR_CFG_PBMP_ADD;
    uid_cfg.number_of_ports = max_valid_port;
    for (i=0; i<max_valid_port; i++)
    {
        /* 此处随意添加一些端口进行测试 */
        if (i % 20 == 0)
            lacp_bitmap_set_bit(&uid_cfg.ports, i);
    }

    lacp_sys_set_cfg(&uid_cfg);
}
int main()
{
    rl_init ();

    my_pid = getpid();
    printf ("my pid: %ld\n", my_pid);

    if (0 == lacp_start ()) {
//    main_loop ();
    }

    //bridge_shutdown ();

//  rl_shutdown ();




    printf("\r\n DONE \r\n");
    getchar();
}


#endif
