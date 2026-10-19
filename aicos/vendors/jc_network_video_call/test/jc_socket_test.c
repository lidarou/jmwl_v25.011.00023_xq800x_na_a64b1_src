

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1a2d86
#define LOG_MODULE    AMOD_VENDOR
#include <aic_bitmap.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aipc_api.h>
#include <aic_cmd.h>
#include <aic_osa.h>
#include <aos_mem.h>

#include "jc.h"

#define IPV4_ADDR "192.168.0.17:8080"
#define IPV6_ADDR "[240e:58:c000:1300:116:236:159:8]:8090"
#define DNS_ADDR  "e36d0bcad92b1965.natapp.cc:11784"
#define DNS_ADDR_1  "cn.iot.justalkcloud.com:2198"
#define IPV6_ADDR_1 "[::1]:87"
#define IPV6_ADDR_2 "[::1]:86"

struct net_addr *jc_tcp_net_addr;
struct net_sock *jc_tcp_net_sock;

struct net_addr *jc_net_addr;
struct net_sock *jc_net_sock;
u64 recv_buffer[64];

static int aic_jc_sock_test_cmd(int argc, char **argv)
{
    int ret, num;

    jc_tcp_net_addr = jc_getaddrs(IPV6_ADDR_2, 0, NET_ENABLE_IP6);
    jc_net_addr = jc_getaddrs(IPV6_ADDR_1, 0, NET_ENABLE_IP6);
    CONS_OUT("addr: %s addr: %s\r\n", jc_addr2str(jc_net_addr), jc_addr2str(jc_tcp_net_addr));
    jc_net_sock = jc_dgramsocket(jc_net_addr);
    if (!jc_net_sock)
        CONS_OUT("connect fail\r\n");

    jc_tcp_net_sock = jc_connecthost(jc_tcp_net_addr);
    if (!jc_tcp_net_sock)
        CONS_OUT("tcp connect fail\r\n");

    CONS_OUT("socket %d %d\r\n", (int)jc_net_sock, (int)jc_tcp_net_sock);

    if (jc_net_sock)
        jc_sendto(jc_net_sock, DNS_ADDR_1, sizeof(DNS_ADDR_1), jc_net_addr);

    if (jc_tcp_net_sock)
        jc_sendto(jc_tcp_net_sock, DNS_ADDR_1, sizeof(DNS_ADDR_1), jc_tcp_net_addr);

    aos_sleep(1000);
    ret = jc_timedrecv(1000, jc_tcp_net_sock, jc_net_sock);
    CONS_OUT("select: 0x%x\r\n", ret);
    if (ret) {
       num = jc_recv(jc_net_sock, recv_buffer, 64);
       recv_buffer[num] = '\0';
       CONS_OUT("udp recv: num %d %s\r\n", num, recv_buffer);

        num = jc_recv(jc_tcp_net_sock, recv_buffer, 64);
        recv_buffer[num] = '\0';
        CONS_OUT("tcp recv: num %d %s\r\n", num, recv_buffer);
    }

    jc_closesocket(jc_net_sock);
    jc_closesocket(jc_tcp_net_sock);

    jc_freeaddr(jc_net_addr, 16);
    jc_freeaddr(jc_tcp_net_addr, 16);

    return 0;
}
CONSOLE_EXPORT_CMD(jc_sock_test, aic_jc_sock_test_cmd, jc sock test);

static void jc_demo_task(void *p)
{
    jc_log("jc demo task\r\n");
}

static int aic_jc_sys_test_cmd(int argc, char **argv)
{
    u8 *mem;
    void *task_id;
    CONS_OUT("time: %d\r\n", jc_time());
    mem = jc_malloc(1024);
    if (!mem)
        CONS_OUT("alloc mem fail\r\n");
    else
        jc_free(mem, 0);

    task_id = jc_task_create("JC test task", jc_demo_task, NULL);
    aos_sleep(1000);
    jc_task_join(task_id);

    CONS_OUT("UID:%s\r\n", jc_chip_uid);
    CONS_OUT("time: %d\r\n", jc_time());
}
CONSOLE_EXPORT_CMD(jc_sys_test, aic_jc_sys_test_cmd, jc sys test);

