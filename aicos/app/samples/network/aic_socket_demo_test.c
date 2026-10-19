#include "aic_osa_ex.h"
#include "nx_api.h"
#include "nx_ip.h"
#include "nxd_bsd.h"
#include "aic_net_pdp_mgr.h"
#include <aic_init.h>
#include <aic_size.h>
#include <string.h>
#include <stdlib.h>

#define DEMO_TASK_PRIORITY  20

#define RECV_BUFFER_SIZE  128
#define MESSAGE0 "socket demo recv data from server 47.106.251.151"
#define MESSAGE1 "socket demo recv data from server 47.110.72.141"

static aos_thread_id socket_thread0 = AOS_INVALID_THREAD_ID;
static aos_thread_id socket_thread1 = AOS_INVALID_THREAD_ID;

int udp_sockfd;

void socket_demo_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state);
void socket_demo_thread0_entry(void* thread_input);
void socket_demo_thread1_entry(void* thread_input);
void socket_demo_close(void);

int socket_demo_init(void)
{
    aic_register_pdp_state_change_cb(socket_demo_pdp_state_change_callback);
    return 0;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(socket_demo_init);

void socket_demo_pdp_state_change_callback(u8 pdp_cid, u8 pdp_state)
{
    WAN_NET *net_ptr = NULL;
    struct sockaddr_in srcaddr;
    struct timeval timeout;
    int ret;

    if(pdp_cid != ANET_DEFAULT_PDP_CID || pdp_state != 1) {
        app_logi("[demo]:PDP is not activated\n");
        socket_demo_close();
        return;
    }

    if (socket_thread0 == AOS_INVALID_THREAD_ID) {
        socket_thread0 = aos_create_thread("Socket Demo Thread 0",
                                          socket_demo_thread0_entry,
                                          NULL, SIZE_2K,
                                          0, 0, DEMO_TASK_PRIORITY,
                                          AOS_DONT_START);
        /* Check for errors */
        if (socket_thread0 == AOS_INVALID_THREAD_ID) {
            app_loge("[demo]:create thread error\n");
            return;
        }
    }

    if (socket_thread1 == AOS_INVALID_THREAD_ID) {
        socket_thread1 = aos_create_thread("Socket Demo Thread 1",
                                          socket_demo_thread1_entry,
                                          NULL, SIZE_2K,
                                          0, 0, DEMO_TASK_PRIORITY,
                                          AOS_DONT_START);
        /* Check for errors */
        if (socket_thread1 == AOS_INVALID_THREAD_ID) {
            aos_delete_thread(socket_thread0);
            app_loge("[demo]:create thread error\n");
            return;
        }
    }

    udp_sockfd = anet_socket(AF_INET, SOCK_DGRAM, NX_PROTOCOL_UDP);
    if (udp_sockfd == -1) {
        app_loge("%s:create socket error, errno %d\n", __func__, errno);
        return;
    } else {
        app_logi("%s:create socket success\n", __func__);
    }

    net_ptr = aic_get_wan_net_per_pdp(pdp_cid);
    if (net_ptr == NULL || net_ptr->pdp_activated == 0) {
        aos_delete_thread(socket_thread0);
        aos_delete_thread(socket_thread1);
        socket_thread0 = AOS_INVALID_THREAD_ID;
        socket_thread1 = AOS_INVALID_THREAD_ID;
        return;
    }

    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = htonl(net_ptr->wan_ipv4_address);
    srcaddr.sin_port = NX_ANY_PORT;
    ret = anet_bind(udp_sockfd, (struct sockaddr *)&srcaddr, sizeof(srcaddr));
    if (ret < 0) {
        app_loge("socket bind error, errno %d\n", errno);
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    ret = setsockopt(udp_sockfd, SOL_SOCKET, SO_RCVTIMEO,
                     &timeout, sizeof(timeout));

    aos_resume_thread(socket_thread0);
    aos_sleep(1000);
    aos_resume_thread(socket_thread1);
}

void socket_demo_thread0_entry(void* thread_input)
{
    struct sockaddr_in dstaddr, fromaddr;
    u32 time0;
    int addrlen, ret;
    char buffer[RECV_BUFFER_SIZE] = {'\0'};

    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_addr.s_addr = htonl(IP_ADDRESS(47, 106, 251, 151));
    dstaddr.sin_port = htons(39998);

    ret = anet_sendto(udp_sockfd, MESSAGE0, (INT)(sizeof(MESSAGE0)),
                      MSG_DONTWAIT, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
    if (ret < 0) {
        app_loge("[demo] send data to server 0 error, errno %d\n", errno);
    } else {
        app_logi("[demo] send data to server 0 success\n");
    }
    time0 = aos_get_tickcount();

    addrlen = sizeof(struct sockaddr_in);
    while (1) {
        ret = anet_recvfrom(udp_sockfd, buffer, RECV_BUFFER_SIZE, 0,
                            (struct sockaddr *)&fromaddr, &addrlen);
        if (ret < 0) {
            app_loge("[demo] recv data from server 0 error, errno %d\n", errno);
        } else {
            app_logi("[demo] recv data from server 0, data: %s", buffer);
        }

        if (aos_get_tickcount() < (time0 + 2000)) {
            aos_sleep((time0 + 2000) - aos_get_tickcount());
        }

        ret = anet_sendto(udp_sockfd, MESSAGE0, (INT)(sizeof(MESSAGE1)),
                          MSG_DONTWAIT, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
        if (ret < 0) {
            app_loge("[demo] send data to server 0 error, errno %d\n", errno);
        } else {
            app_logi("[demo] send data to server 0 success\n");
        }
        time0 = aos_get_tickcount();
    }
}

void socket_demo_thread1_entry(void* thread_input)
{
    struct sockaddr_in dstaddr, fromaddr;
    u32 time1;
    int addrlen, ret;
    char buffer[RECV_BUFFER_SIZE] = {'\0'};

    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_addr.s_addr = htonl(IP_ADDRESS(47, 110, 72, 141));
    dstaddr.sin_port = htons(2320);

    ret = anet_sendto(udp_sockfd, MESSAGE1, (INT)(sizeof(MESSAGE1)),
                      MSG_DONTWAIT, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
    if (ret < 0) {
        app_loge("[demo] send data to server 1 error, errno %d\n", errno);
    } else {
        app_logi("[demo] send data to server 1 success\n");
    }
    time1 = aos_get_tickcount();

    addrlen = sizeof(struct sockaddr_in);
    while (1) {
        ret = anet_recvfrom(udp_sockfd, buffer, RECV_BUFFER_SIZE, 0,
                            (struct sockaddr *)&fromaddr, &addrlen);
        if (ret < 0) {
            app_loge("[demo] recv data from server 1 error, errno %d\n", errno);
        } else {
            app_logi("[demo] recv data from server 1, data: %s", buffer);
        }

        if (aos_get_tickcount() < (time1 + 2000)) {
            aos_sleep((time1 + 2000) - aos_get_tickcount());
        }

        ret = anet_sendto(udp_sockfd, MESSAGE1, (INT)(sizeof(MESSAGE1)),
                          MSG_DONTWAIT, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
        if (ret < 0) {
            app_loge("[demo] send data to server 1 error, errno %d\n", errno);
        } else {
            app_logi("[demo] send data to server 1 success\n");
        }
        time1 = aos_get_tickcount();
    }
}

void socket_demo_close(void)
{
    soc_close(udp_sockfd);
    //aos_suspend_thread(socket_thread0);
    //aos_suspend_thread(socket_thread1);
    aos_delete_thread(socket_thread0);
    aos_delete_thread(socket_thread1);
    socket_thread0 = AOS_INVALID_THREAD_ID;
    socket_thread1 = AOS_INVALID_THREAD_ID;
}
