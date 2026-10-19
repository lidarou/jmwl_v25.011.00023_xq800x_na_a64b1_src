
#ifndef _JC_H_
#define _JC_H_

#include <nxd_bsd.h>

#define JC_TASK_PRIORITY 20

enum {
    NET_ENABLE_IP4 = 1,
    NET_ENABLE_IP6 = 2,
    NET_ENABLE_IP = 3,
    NET_ENABLE_DNS = 4,
};

struct net_addr {
    struct net_addr *next;
    struct sockaddr *addr;
    socklen_t addrlen;
};

struct net_sock {
    int fd;
};

/** 释放网络地址链表
 * @param[in] addr 链表首结点
 * @param[in] num  需释放个数
 * @return 返回剩余的链表, 全部释放则返回NULL
 * 通常如下实现：
 * @code
 *      for (; addr && num > 0; --num) {
 *          struct _net_addr* a = (struct _net_addr*)addr;
 *          addr = addr->next;
 *          free(a);
 *      }
 *      return addr;
 * @endcode
 */
struct net_addr* jc_freeaddr(struct net_addr* addr, unsigned num);

/** 返回首个网络地址的文本格式, 只用于日志打印 */
const char* jc_addr2str(const struct net_addr* addr);

/** DNS 解析
 * @param[in] host 地址的标准文本格式
 * @param[in] dft_port 若host中不带地址时,使用该默认端口
 * @param[in] enables NET_ENABLE_* 的组合,指明解析方式
 * @return 成功返回地址链表
 * @remarks
 *  enables=NET_ENABLE_DNS|NET_ENABLE_IP
 *  则说明可能是域名或ip格式(ip4,ip6)
 */
struct net_addr* jc_getaddrs(const char* host, unsigned short dft_port, unsigned enables);

/** 连接服务器, 返回非阻塞的 TCP socket */
struct net_sock* jc_connecthost(struct net_addr* addr);

//创建非阻塞的 UDP socket
struct net_sock* jc_dgramsocket(struct net_addr* addr);

/** 使用sockfd 发送数据
 * @param[in] addr 若sockfd为TCP连接, 则NULL
 * @return 成功返回实际写入的长度
 */
int jc_sendto(struct net_sock* sockfd, void* data, int len, struct net_addr* addr);

/** 收发都关闭,只将缓冲区中的发送完毕
 * @param[in] addr 若sockfd为TCP连接, 则NULL
 * @return 返回后续将被释放的socket
 */
struct net_sock* jc_shutdown(struct net_sock* sockfd, struct net_addr* addr);

//释放socket
void jc_closesocket(struct net_sock* sockfd);

/** 超时等待可读状态
 * @param[in] ms 超时毫秒数
 * @param[in] tcpfd,udpfd 等待读的sockets, udpfd 允许为NULL
 * @return 失败返回负数, 成功返回值掩码, 从低位开始表示每个fd 的可读状态
 * 例如
 * - tcpfd, udpfd 都可读, return (1<<0) | (1<<1);
 * - 只有udpfd 可读, return (1<<1);
 * - 只有tcpfd 可读, return (1<<0);
 * - 出错返回 return -1;
 */
int jc_timedrecv(unsigned ms, struct net_sock* tcpfd, struct net_sock* udpfd);

/** 读取 sockfd 中的数据
 * 内部收包流程:
 * 1. jc_timedrecv() //检测可读性
 * 2. while( jc_recv() > 0) {...} //尽可能到读取所有
 * 所以要求jc_recv 是非阻塞的。
 * jc_connecthost/jc_dgramsocket 中设置该sockfd为非阻塞后，
 * 通常如下实现:
 * @code 
 *      int ret = socket_recv(sockfd, buf, buflen);
 *      if (ret < 0 && socket_errno(sockfd) == EWOULDBLOCK) 
 *          ret = 0;
 *      return ret;
 * @endcode
 */
int jc_recv(struct net_sock* sockfd, void* buf, int buflen);

//打印日志
//@param[in] fmt 以"STS: ","INF: ","WAN: "或"ERR: "为前缀
void jc_log(const char *fmt, ...);

//返回芯片名称
const char*jc_chip_uid(void);

//返回当前CPU空闲百分比,不支持返回0
unsigned jc_cpu_idle(void);

//返回当前内存剩余KB,不支持返回0
unsigned jc_mem_size(void);

//返回递增的时间截,毫秒值
unsigned jc_time(void);

//动态分配堆地址
void* jc_malloc(unsigned size);

//释放动态堆地址
void jc_free(void* ptr, unsigned size);

/** 开启线程
 * jrtc_open() 内部将使用该函数创建工作线程
 * @note
 *  若不支持线程, 请直接使用 jrtc_alloc(), 例如:
 * @code
 *      jc = jrtc_alloc('uid', NULL, NULL, NULL);
 *      jrtc_activate(jc, JRTC_SPEAKER|JRTC_MICROPHONE);
 *      state = jrtc_state(jc);
 *      while (state != JRTC_CLOSED)
 *          state = jrtc_schedule(jc, jc_time());
 * @endcode
 */
void* jc_task_create(char* name, void (*fn)(void*), void *arg);

extern unsigned JRTC_TASK_STACK_KB; /*< 上述创建的线程栈大小,与平台相关 */
extern unsigned JRTC_TASK_PRIORITY; /*< 上述创建的线程优先级,与平台相关 */

/** 等待线程结束
 * jrtc_close() 内部使用该函数回收线程
 */
void  jc_task_join(void *task);

#endif

