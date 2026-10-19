

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0424e0
#define LOG_MODULE    AMOD_VENDOR
#include <string.h>

#include <aic_osa.h>
#include <aic_size.h>
#include <aic_errno.h>
#include <aos_mem.h>

#include <timer_inc.h>
#include <efuse.h>

#include "jc.h"
#include "pthread.h"

#define TRAVERSAL_CHAR_MAX_NUM  6
#define JC_TASK_STACK_SIZE      (SIZE_1K * 14)

static char log_buf[255];
void jc_log(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(log_buf, 255, fmt, args);
    alog_output(LOG_MODULE, ALOG_LV_ERROR, "%s", log_buf);
    va_end(args);
}

struct net_addr *jc_getaddrs(const char *host, unsigned short dft_port,
                             unsigned enables)
{
    char str1[64], str2[8];
    struct addrinfo hints;
    int i, ret, str_len, cnt = 0, has_port = FALSE;
    struct addrinfo *res, *head_res, *next_res;
    struct net_addr *net_addr, *head_net_addr = NULL, *next_net_addr, *pre_net_addr;

    memset(str1, 0, sizeof(str1));
    memset(str2, 0, sizeof(str2));
    str_len = strlen(host);
    if (str_len > (sizeof(str1) - 1)) {
        jc_log("[JC_SOCKET] host is too big %d\r\n", str_len);
        return NULL;
    }

    for (i = 0; i < str_len; i++) {
        if (*(host + i) == ':')
            cnt++;
    }

    if (cnt > 1) {
        for (i = 0; i < (TRAVERSAL_CHAR_MAX_NUM + 1); i++) {
            if (*(host + str_len - 1 - i) == ':' &&
                *(host + str_len - 2 - i) == ']' && i > 0) {
                jc_log("port index %d\r\n", i);
                memcpy(str1, host + 1, str_len - i - 3);
                str1[str_len - i - 3] = '\0';
                memcpy(str2, host + str_len - i, i);
                str2[i] = '\0';
                has_port = TRUE;
                break;
            }
        }
    } else if (cnt == 1) {
        for (i = 0; i < (TRAVERSAL_CHAR_MAX_NUM + 1); i++) {
            if (*(host + str_len - 1 - i) == ':' && i > 0) {
                jc_log("port index %d\r\n", i);
                memcpy(str1, host, str_len - i - 1);
                str1[str_len - i - 1] = '\0';
                memcpy(str2, host + str_len - i, i);
                str2[i] = '\0';
                has_port = TRUE;
                break;
            }
        }
    } else {
        memcpy(str1, host, str_len);
        str1[str_len] = '\0';
        memset(str2, '\0', sizeof(str2));
        snprintf(str2, sizeof(str2), "%d", dft_port);
    }

    memset(&hints, 0, sizeof(hints));
    if (enables == NET_ENABLE_IP4)
        hints.ai_family = AF_INET;
    else if (enables == NET_ENABLE_IP6)
        hints.ai_family = AF_INET6;
    else
        hints.ai_family = AF_UNSPEC;

    jc_log("[JC_SOCK] get addr info str1 %s str2 %s\r\n", str1, str2);
    ret = getaddrinfo(str1, str2, &hints, &res);
    if (ret != NX_SUCCESS || !res) {
        jc_log("[JC_SOCK] get addr info fail %s ret %d\r\n", host, ret);
        return NULL;
    }

    head_res = res;
    while (res) {
        net_addr = aos_zalloc(sizeof(struct net_addr));
        if (!net_addr) {
            jc_log("[JC_SOCK] alloc net addr fail\r\n");
            goto ERROR_OUT;
        }

        net_addr->addrlen = res->ai_addrlen;
        net_addr->addr = aos_zalloc(net_addr->addrlen);
        if (!net_addr->addr) {
            aos_free(net_addr);
            jc_log("[JC_SOCK] alloc sock addr fail\r\n");
            goto ERROR_OUT;
        }

        next_res = res->ai_next;
        memcpy(net_addr->addr, res->ai_addr, net_addr->addrlen);
        if (!head_net_addr) {
            head_net_addr = net_addr;
        } else if (pre_net_addr) {
            pre_net_addr->next = net_addr;
        } else {
            AIC_ASSERT(0);
        }

        pre_net_addr = net_addr;
        res = next_res;
    }

    freeaddrinfo(head_res);
    return head_net_addr;

ERROR_OUT:
    freeaddrinfo(head_res);
    net_addr = head_net_addr;
    while (net_addr) {
        next_net_addr = net_addr->next;
        aos_free(net_addr->addr);
        aos_free(net_addr);
        net_addr = next_net_addr;
    }

    return NULL;
}

struct net_addr *jc_freeaddr(struct net_addr *addr, unsigned num)
{
    int i;
    struct net_addr *next_addr = NULL;

    for (i = 0; i < num; i++) {
        if (addr) {
            next_addr = addr->next;
            aos_free(addr->addr);
            aos_free(addr);
            addr = next_addr;
        } else {
            break;
        }
    }

    return next_addr;
}

const char *jc_addr2str(const struct net_addr *addr)
{
    static s8 addr_info[32];

    memset(addr_info, '\0', sizeof(addr_info));
    if (addr) {
        if (addr->addr->sa_family == AF_INET)
            inet_ntop(addr->addr->sa_family,
                      &((struct sockaddr_in *)(addr->addr))->sin_addr, addr_info,
                      sizeof(addr_info));
        else
            inet_ntop(addr->addr->sa_family,
                      &((struct sockaddr_in6 *)(addr->addr))->sin6_addr, addr_info,
                      sizeof(addr_info));
    }

    return addr_info;
}

struct net_sock *jc_connecthost(struct net_addr *addr)
{
    int fd, ret;
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;

    if (!addr)
        return NULL;

    fd = socket(addr->addr->sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        jc_log("[JC_SOCKET] create socket fail\r\n");
        return NULL;
    }

    if (addr->addr->sa_family == AF_INET) {
        aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        remoteaddr.sin_family = AF_INET;
        remoteaddr.sin_port = ((struct sockaddr_in *)addr->addr)->sin_port;
        remoteaddr.sin_addr.s_addr =
            ((struct sockaddr_in *)addr->addr)->sin_addr.s_addr;

        ret = connect(fd, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
    } else if (addr->addr->sa_family == AF_INET6) {
        aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        remoteaddr6.sin6_family = AF_INET6;
        remoteaddr6.sin6_port = ((struct sockaddr_in6 *)addr->addr)->sin6_port;
        aos_memcpy(&remoteaddr6.sin6_addr,
                   &((struct sockaddr_in6 *)addr->addr)->sin6_addr, 16);
        ret = connect(fd, (struct sockaddr *)&remoteaddr6, sizeof(remoteaddr6));
    } else {
        jc_log("[JC_SOCKET] can't support this version %d when connect\r\n",
               addr->addr->sa_family);
        return NULL;
    }

    if (ret != OK) {
        jc_log("[JC_SOCKET] socket connect fail 0x%x!\n", ret);
        soc_close(fd);
        return NULL;
    }

    return (struct net_sock *)fd;
}

struct net_sock *jc_dgramsocket(struct net_addr *addr)
{
    return (struct net_sock *)socket(addr->addr->sa_family, SOCK_DGRAM,
                                     IPPROTO_UDP);
}

struct net_sock* jc_shutdown(struct net_sock* sockfd, struct net_addr* addr)
{
    if (sockfd)
        shutdown((int)sockfd, SHUT_RDWR);

    return sockfd;
}

void jc_closesocket(struct net_sock *sockfd)
{
    soc_close((int)sockfd);
}

int jc_sendto(struct net_sock *sockfd, void *data, int len,
              struct net_addr *addr)
{
    int num;
    struct sockaddr_in remoteaddr;
    struct sockaddr_in6 remoteaddr6;

    if (!addr) {
        num = send((int)sockfd, (const CHAR *)data, len, 0);
    } else if (addr->addr->sa_family == AF_INET) {
        aos_memset(&remoteaddr, 0, sizeof(struct sockaddr_in));
        remoteaddr.sin_family = AF_INET;
        remoteaddr.sin_port = ((struct sockaddr_in *)addr->addr)->sin_port;
        remoteaddr.sin_addr.s_addr =
            ((struct sockaddr_in *)addr->addr)->sin_addr.s_addr;

        num = sendto((int)sockfd, (const CHAR *)data, len, MSG_DONTWAIT,
                     (const struct sockaddr *)&remoteaddr,
                     sizeof(remoteaddr));
        if (num < 0 && errno == ENOBUFS) {
            jc_log("[JC_SOCKET] send len %d fail %d \r\n", len, errno);
            num = len;
        }
    } else if (addr->addr->sa_family == AF_INET6) {
        aos_memset(&remoteaddr6, 0, sizeof(struct sockaddr_in6));
        remoteaddr6.sin6_family = AF_INET6;
        remoteaddr6.sin6_port = ((struct sockaddr_in6 *)addr->addr)->sin6_port;
        aos_memcpy(&remoteaddr6.sin6_addr,
                   &((struct sockaddr_in6 *)addr->addr)->sin6_addr, 16);
        num = sendto((int)sockfd, (const CHAR *)data, len, MSG_DONTWAIT,
                     (const struct sockaddr *)&remoteaddr6,
                     sizeof(remoteaddr6));
        if (num < 0 && errno == ENOBUFS) {
            jc_log("[JC_SOCKET] send len %d fail %d \r\n", len, errno);
            num = len;
        }
    } else {
        jc_log("[JC_SOCKET] can't support this version %d when send\r\n",
                  addr->addr->sa_family);
        num = -1;
    }

    return num;
}

int jc_timedrecv(unsigned ms, struct net_sock *tcpfd, struct net_sock *udpfd)
{
    int nfds = 0, status = 0;
    fd_set read_ready, exceptfds;
    struct timeval timeout;

    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms % 1000) * 1000;

    FD_ZERO(&read_ready);
    FD_ZERO(&exceptfds);
    if (tcpfd) {
        FD_SET((int)tcpfd, &read_ready);
        FD_SET((int)tcpfd, &exceptfds);
        if ((int)tcpfd > nfds)
            nfds = (int)tcpfd;
    }

    if (udpfd) {
        FD_SET((int)udpfd, &read_ready);
        FD_SET((int)udpfd, &exceptfds);
        if ((int)udpfd > nfds)
            nfds = (int)udpfd;
    }

    if (!select(nfds + 1, &read_ready, NULL, &exceptfds, &timeout)) {
        jc_log("[JC_SOCKET] no read and except event\r\n");
        return 0;
    }


    if (tcpfd && FD_ISSET((int)tcpfd, &exceptfds)) {
        jc_log("[JC_SOCKET] get tcp except\r\n");
        return -1;
    }

    if (udpfd && FD_ISSET((int)udpfd, &exceptfds)) {
        jc_log("[JC_SOCKET] get udp except\r\n");
        return -1;
    }

    if (tcpfd && FD_ISSET((int)tcpfd, &read_ready))
        status |= BIT(0);

    if (udpfd && FD_ISSET((int)udpfd, &read_ready))
        status |= BIT(1);

    jc_log("[JC_SOCKET] jc_timedrecv status=%d\r\n", status);

    return status;
}


int jc_recv(struct net_sock *sockfd, void *buf, int buflen)
{
    int ret;

    ret = recv((int)sockfd, buf, buflen, MSG_DONTWAIT);
    if (ret == -1) {
        if (errno == EAGAIN)
            return 0;
    }

    return ret;
}

unsigned jc_cpu_idle(void)
{
    return 0;
}

unsigned jc_mem_size(void)
{
    return 0;
}

unsigned jc_time(void)
{
    return aic_get_tick();
}

void *jc_malloc(unsigned size)
{
    return aos_malloc(size);
}

void jc_free(void *ptr, unsigned size)
{
    aos_free(ptr);
}


void *jc_task_create(char *name, void (*fn)(void *), void *arg)
{
    pthread_t       *pthread;
    pthread_attr_t  *ptattr;
    void *stack_ptr;
    struct sched_param param;

    pthread = aos_malloc(sizeof(pthread_t) + sizeof(pthread_attr_t) + JC_TASK_STACK_SIZE);
    if (!pthread) {
        jc_log("ERROR: no enough memory\n");
        return NULL;
    }
    ptattr = (pthread_attr_t *)((char *)pthread + sizeof(*pthread));
    stack_ptr = (void *)((char *)ptattr + sizeof(*ptattr));
    pthread_attr_init(ptattr);

    memset(&param, 0, sizeof(param));
    param.sched_priority = JC_TASK_PRIORITY;
    pthread_attr_setschedparam(ptattr, &param);
    pthread_attr_setstackaddr(ptattr, stack_ptr);
    pthread_attr_setstacksize(ptattr, JC_TASK_STACK_SIZE);
    pthread_attr_setcanceltype(ptattr, PTHREAD_CANCEL_ASYNCHRONOUS);
    if (pthread_create(pthread, ptattr, (void *(*)(void*))fn, arg)) {
        jc_log("jc thread create failed\n");
        return NULL;
    }

    jc_log("jc thread=%p, thread_id=%p\n", pthread, *pthread);

    return (void *)(pthread);
}

void jc_task_join(void *task)
{
    jc_log("jc thread join, %p\n", task);
    if (task) {
        if((pthread_join_base(*((pthread_t *)task)) == AIC_SUCCESS))
            aos_free(task);
        jc_log("jc pthread base exit\n");
    }
}

const char *jc_chip_uid(void)
{
    int len;
    static char uid[64] = {'\0'};

    memset(uid, '\0', sizeof(uid));
    aic_get_uid(uid, &len);

    return uid;
}

