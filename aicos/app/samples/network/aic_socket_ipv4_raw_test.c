/* This is a small demo of the high-performance BSD socket using the NetX Duo TCP/IP stack.  This 
   demo concentrates on raw packet sending and receiving using a simulated Ethernet driver.  */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07500d

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_bsd.h"
#include "nx_ipv6.h"
#include <string.h>
#include <stdlib.h>
#include "aic_osa.h"
#include "aic_init.h"
#include <aic_size.h>
#include <aic_osa_ex.h>

#define     DEMO_STACK_SIZE          2048
#define     CLIENT0_IP_ADDRESS       IP_ADDRESS(127,0,0,1)
#define     CLIENT1_IP_ADDRESS       IP_ADDRESS(127,0,0,1)
#define     MESSAGE                  "Client 0 BSD Raw Socket testing\n"

/* Define the ThreadX and NetX Duo object control blocks...  */

static aos_thread_id               thread_0;
static aos_thread_id               thread_1;

/* Define thread prototypes.  */

static VOID thread_0_entry(void *thread_input);
static VOID thread_1_entry(void *thread_input);
extern void nx_alte_nic_driver_entry(NX_IP_DRIVER *driver_req_ptr);

/* Define Client 0. */
static void thread_0_entry(void *thread_input)
{
    UCHAR        tos = 32, ttl = 32;
    INT          sock_raw;
    INT          socket_family;
    INT          bytes_sent;
    UINT         status;
    struct       sockaddr_in destAddr;
    struct       sockaddr_in ClientAddr;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow the network driver to initialize NetX Duo. */
    tx_thread_sleep(10 * NX_IP_PERIODIC_RATE);

    /* Set up IPv4 client and peer addressing.  */
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    ClientAddr.sin_family = PF_INET;
    ClientAddr.sin_addr.s_addr = htonl(CLIENT0_IP_ADDRESS);

    /* Assign a known Server port */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_addr.s_addr = htonl(CLIENT1_IP_ADDRESS);
    destAddr.sin_family = PF_INET;

    socket_family =  AF_INET;

    while (1)
    {
         /* Create a BSD raw Client Socket */
         sock_raw = socket(socket_family, SOCK_RAW, 0x99);

         /* Check for any errors */
         if (sock_raw == ERROR)
         {
             app_logi("Error: BSD Client socket create %d\n", errno);
             return;
         }

         status = setsockopt(sock_raw, IPPROTO_IP, IP_TOS, &tos, 1);
         if (status)
             app_logi("[NET] ipv4 raw server set IP TOS fail\n");
         status = setsockopt(sock_raw, IPPROTO_IP, IP_TTL, &ttl, 1);
         if (status)
             app_logi("[NET] ipv4 raw server set IP TTL fail\n");

         /* Client 1 is ready to send a packet. */

         bytes_sent = sendto(sock_raw, MESSAGE, (INT)(sizeof(MESSAGE)), 0,
                             (struct sockaddr *) &destAddr, sizeof(destAddr));

         if (bytes_sent > 0)
         {
             app_logi("Client 0 sent a packet\n");
         }

         app_logi("Close Client0 socket.\n");

         /* Close the Client 0 socket */
         soc_close(sock_raw);

         tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }
}

/* Define Client 1. */
static void thread_1_entry(void *thread_input)
{
    UCHAR        tos = 32, ttl = 32;
    UINT         status;
    INT          addrlen;
    INT          sock_raw;
    CHAR         recv_buffer[64];
    INT          socket_family;
    INT          bytes_received;
    struct       sockaddr_in destAddr;                       /* Destination address */
    struct       sockaddr_in ClientAddr;                     /* Client address */
    struct       sockaddr_in fromAddr;                       /* Client address */

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow the network driver to initialize NetX Duo. */
    tx_thread_sleep(10 * NX_IP_PERIODIC_RATE);

    /* Set up IPv4 client and peer addressing.  */
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    ClientAddr.sin_family = PF_INET;
    ClientAddr.sin_addr.s_addr = htonl(CLIENT1_IP_ADDRESS);

    /* Assign a known Server port */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_addr.s_addr = htonl(CLIENT0_IP_ADDRESS);
    destAddr.sin_family = PF_INET;

    socket_family =  AF_INET;

    while(1)
    {

         /* Create Client 1's raw socket */
         sock_raw = socket(socket_family, SOCK_RAW, 0x99);

         /* Check for any errors */
         if (sock_raw == ERROR)
         {
             app_logi("Error: BSD Client 1 socket create %d\n", errno);
             return;
         }

         status = setsockopt(sock_raw, IPPROTO_IP, IP_TOS, &tos, 1);
         if (status)
             app_logi("[NET] ipv4 raw client set IP TOS fail\n");
         status = setsockopt(sock_raw, IPPROTO_IP, IP_TTL, &ttl, 1);
         if (status)
             app_logi("[NET] ipv4 raw client set IP TTL fail\n");

         /* Client 1 is ready to receive packets. */
         bytes_received = 0;

         /* Try to receive an message from the remote peer. */
         addrlen = sizeof(fromAddr);
         bytes_received = recvfrom(sock_raw, (VOID *)recv_buffer,64, 0,
                                   (struct sockaddr *) &fromAddr, &addrlen);
         if (bytes_received > 0)
         {
             app_logi("Client 1 received (%d bytes); %s \n", bytes_received,
                      (CHAR *)&recv_buffer[0] + 20);
             app_logi("Remote IP address 0x%x\n",
                      (UINT)fromAddr.sin_addr.s_addr);
         }

         /* Clear the message buffer. */
         memset(&recv_buffer[0], 0, 64);

         app_logi("Close Client1 socket.\n");

         /* Close the raw socket */
         soc_close(sock_raw);
     }
}

/* Define what the initial system looks like. */
static int tx_raw_socket_application_define(void)
{

    thread_0 = aos_create_thread("Raw Socket Client 0",
                                 thread_0_entry,
                                 NULL, DEMO_STACK_SIZE, 0, NULL, 30,
                                 AOS_AUTO_START);

    thread_1 = aos_create_thread("Raw Socket Client 1",
                                 thread_1_entry,
                                 NULL, DEMO_STACK_SIZE, 0, NULL, 20,
                                 AOS_AUTO_START);

    return 0;
}
AIC_APP_P2_INIT_EXPORT(tx_raw_socket_application_define);

