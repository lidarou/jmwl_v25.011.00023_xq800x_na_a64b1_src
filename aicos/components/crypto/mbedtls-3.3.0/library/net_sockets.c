/*
 *  TCP/IP or UDP/IP networking functions
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/* Enable definition of getaddrinfo() even when compiling with -std=c99. Must
 * be set before mbedtls_config.h, which pulls in glibc's features.h indirectly.
 * Harmless on other platforms. */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02e030
#define LOG_MODULE    AMOD_SYS

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600 /* sockaddr_storage */
#endif

#include "common.h"

#if defined(MBEDTLS_NET_C)

#include "nxd_bsd.h"
#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_udp.h"
#include "aic_net_api.h"

#include "mbedtls/platform.h"

#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"

#include <string.h>

#define read(fd,buf,len)        recv( fd, (char*)( buf ), (int)( len ), 0 )
#define write(fd,buf,len)       send( fd, (char*)( buf ), (int)( len ), 0 )
#define close(fd)               soc_close(fd)

#define mbedtls_read(fd,buf,len,timeout)    aic_ssl_recv(fd,buf,len,timeout)
#define mbedtls_write(fd,buf,len)           aic_ssl_send(fd,buf,len)

#define IS_EINTR(ret) ((ret) == EINTR)

#include <stdio.h>

#if defined(MBEDTLS_HAVE_TIME)
#include <time.h>
#endif

#include <stdint.h>

#define TLS_SOCKET_MAX 10
static mbedtls_net_cfg mbedtls_net_list[TLS_SOCKET_MAX] = {0};

/*
 * Prepare for using the sockets interface
 */
static int net_prepare(void)
{
    if (!nx_bsd_default_ip)
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;

    return 0;
}

/*
 * Return 0 if the file descriptor is valid, an error otherwise.
 * If for_select != 0, check whether the file descriptor is within the range
 * allowed for fd_set used for the FD_xxx macros and the select() function.
 */
static int check_fd(int fd, int for_select)
{
    if (fd < 0)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    /* A limitation of select() is that it only works with file descriptors
     * that are strictly less than FD_SETSIZE. This is a limitation of the
     * fd_set type. Error out early, because attempting to call FD_SET on a
     * large file descriptor is a buffer overflow on typical platforms. */
    if (for_select && fd >= FD_SETSIZE)
        return MBEDTLS_ERR_NET_POLL_FAILED;

    return 0;
}

/*
 * Initialize a context
 */
void mbedtls_net_init(mbedtls_net_context *ctx)
{
    ctx->fd = -1;
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect(mbedtls_net_context *ctx, const char *host,
                        const char *port, int proto)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    struct addrinfo hints, *addr_list, *cur;

    if ((ret = net_prepare()) != 0)
        return ret;

    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if (getaddrinfo( host, port, &hints, &addr_list) != 0)
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;

    /* Try the sockaddrs until a connection succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        ctx->fd = (int)socket(cur->ai_family, cur->ai_socktype,
                              cur->ai_protocol);
        if (ctx->fd < 0) {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        if (!connect(ctx->fd, cur->ai_addr, cur->ai_addrlen)) {
            ret = 0;
            break;
        }

        close(ctx->fd);
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo( addr_list );

    return ret;
}

/*
 * Create a listening socket on bind_ip:port
 */
int mbedtls_net_bind(mbedtls_net_context *ctx, const char *bind_ip,
                     const char *port, int proto )
{
    int n, ret;
    struct addrinfo hints, *addr_list, *cur;

    if ((ret = net_prepare()) != 0)
        return ret;

    /* Bind to IPv6 and/or IPv4, but only in the desired protocol */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;
    if (!bind_ip)
        hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(bind_ip, port, &hints, &addr_list) != 0 )
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;

    /* Try the sockaddrs until a binding succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        ctx->fd = (int)socket(cur->ai_family, cur->ai_socktype,
                              cur->ai_protocol );
        if (ctx->fd < 0) {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        n = 1;
        if (setsockopt(ctx->fd, SOL_SOCKET, SO_REUSEADDR,
                       (const char *)&n, sizeof(n)) != 0) {
            close(ctx->fd);
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        if (bind(ctx->fd, cur->ai_addr, cur->ai_addrlen) != 0) {
            close( ctx->fd );
            ret = MBEDTLS_ERR_NET_BIND_FAILED;
            continue;
        }

        /* Listen only makes sense for TCP */
        if (proto == MBEDTLS_NET_PROTO_TCP) {
            if (listen(ctx->fd, MBEDTLS_NET_LISTEN_BACKLOG) != 0) {
                close(ctx->fd);
                ret = MBEDTLS_ERR_NET_LISTEN_FAILED;
                continue;
            }
        }

        /* Bind was successful */
        ret = 0;
        break;
    }

    freeaddrinfo(addr_list);

    return( ret );

}

/*
 * Check if the requested operation would be blocking on a non-blocking socket
 * and thus 'failed' with a negative return value.
 *
 * Note: on a blocking socket this function always returns 0!
 */
static int net_would_block(const mbedtls_net_context *ctx)
{
    int err = errno;

    /*
     * Never return 'WOULD BLOCK' on a blocking socket
     */
    if ((anet_fcntl(ctx->fd, F_GETFL, O_NONBLOCK) != O_NONBLOCK)) {
        set_errno(err);
        return 0;
    }

    switch(errno) {
#if defined EAGAIN
    case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
        return 1;
    }

    return 0;
}

/*
 * Accept a connection from a remote client
 */
int mbedtls_net_accept(mbedtls_net_context *bind_ctx,
                       mbedtls_net_context *client_ctx,
                       void *client_ip, size_t buf_size, size_t *ip_len )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int type;

    struct sockaddr_storage client_addr;

#if defined(__socklen_t_defined) || defined(_SOCKLEN_T) ||  \
    defined(_SOCKLEN_T_DECLARED) || defined(__DEFINED_socklen_t) || \
    defined(socklen_t) || (defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L)
    socklen_t n = (socklen_t)sizeof(client_addr);
    socklen_t type_len = (socklen_t)sizeof(type);
#else
    int n = (int)sizeof(client_addr);
    int type_len = (int)sizeof(type);
#endif

    /* Is this a TCP or UDP socket? */
    if (getsockopt(bind_ctx->fd, SOL_SOCKET, SO_TYPE,
                   (void *)&type, &type_len ) != 0 ||
        (type != SOCK_STREAM && type != SOCK_DGRAM))
        return MBEDTLS_ERR_NET_ACCEPT_FAILED;

    if (type == SOCK_STREAM) {
        /* TCP: actual accept() */
        ret = client_ctx->fd = (int)accept(bind_ctx->fd,
                                           (struct sockaddr *)&client_addr, &n);
    } else {
        /* UDP: wait for a message, but keep it in the queue */
        char buf[1] = {0};

        ret = (int)recvfrom(bind_ctx->fd, buf, sizeof( buf ), MSG_PEEK,
                            (struct sockaddr *) &client_addr, &n );
    }

    if (ret < 0) {
        if (net_would_block( bind_ctx ) != 0)
            return MBEDTLS_ERR_SSL_WANT_READ;

        return MBEDTLS_ERR_NET_ACCEPT_FAILED;
    }

    /* UDP: hijack the listening socket to communicate with the client,
     * then bind a new socket to accept new connections */
    if (type != SOCK_STREAM) {
        struct sockaddr_storage local_addr;
        int one = 1;

        if (connect(bind_ctx->fd, (struct sockaddr *)&client_addr, n) != 0)
            return MBEDTLS_ERR_NET_ACCEPT_FAILED;

        client_ctx->fd = bind_ctx->fd;
        bind_ctx->fd = -1; /* In case we exit early */

        n = sizeof(struct sockaddr_storage);
        if (getsockname(client_ctx->fd,
                        (struct sockaddr *) &local_addr, &n) != 0 ||
            (bind_ctx->fd = (int)socket(local_addr.ss_family,
                                        SOCK_DGRAM, IPPROTO_UDP)) < 0 ||
            setsockopt(bind_ctx->fd, SOL_SOCKET, SO_REUSEADDR,
                       (const char *)&one, sizeof(one)) != 0) {
            return MBEDTLS_ERR_NET_SOCKET_FAILED;
        }

        if (bind( bind_ctx->fd, (struct sockaddr *) &local_addr, n) != 0)
            return MBEDTLS_ERR_NET_BIND_FAILED;
    }

    if (client_ip != NULL) {
        if (client_addr.ss_family == AF_INET) {
            struct sockaddr_in *addr4 = (struct sockaddr_in *)&client_addr;
            *ip_len = sizeof(addr4->sin_addr.s_addr);

            if (buf_size < *ip_len)
                return MBEDTLS_ERR_NET_BUFFER_TOO_SMALL;

            memcpy(client_ip, &addr4->sin_addr.s_addr, *ip_len);
        } else {
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&client_addr;
            *ip_len = sizeof(addr6->sin6_addr.s6_addr);

            if (buf_size < *ip_len)
                return MBEDTLS_ERR_NET_BUFFER_TOO_SMALL;

            memcpy(client_ip, addr6->sin6_addr.s6_addr, *ip_len);
        }
    }

    return 0;
}

/*
 * Set the socket blocking or non-blocking
 */
int mbedtls_net_set_block(mbedtls_net_context *ctx)
{
    return anet_fcntl(ctx->fd, F_SETFL, anet_fcntl( ctx->fd, F_GETFL, 0) & ~O_NONBLOCK);
}

int mbedtls_net_set_nonblock( mbedtls_net_context *ctx)
{
    return anet_fcntl(ctx->fd, F_SETFL, anet_fcntl(ctx->fd, F_GETFL, 0) | O_NONBLOCK);
}

/*
 * Check if data is available on the socket
 */

int mbedtls_net_poll(mbedtls_net_context *ctx, uint32_t rw, uint32_t timeout)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    struct timeval tv;

    fd_set read_fds;
    fd_set write_fds;

    int fd = ctx->fd;

    ret = check_fd(fd, 1);
    if (ret != 0)
        return ret;

    FD_ZERO(&read_fds);
    if (rw & MBEDTLS_NET_POLL_READ) {
        rw &= ~MBEDTLS_NET_POLL_READ;
        FD_SET(fd, &read_fds);
    }

    FD_ZERO(&write_fds);
    if (rw & MBEDTLS_NET_POLL_WRITE) {
        rw &= ~MBEDTLS_NET_POLL_WRITE;
        FD_SET(fd, &write_fds);
    }

    if (rw != 0)
        return MBEDTLS_ERR_NET_BAD_INPUT_DATA;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    do {
        ret = select(fd + 1, &read_fds, &write_fds, NULL,
                     timeout == (uint32_t)-1 ? NULL : &tv);
    } while (IS_EINTR(ret));

    if (ret < 0)
        return MBEDTLS_ERR_NET_POLL_FAILED;

    ret = 0;
    if (FD_ISSET(fd, &read_fds))
        ret |= MBEDTLS_NET_POLL_READ;
    if (FD_ISSET( fd, &write_fds ))
        ret |= MBEDTLS_NET_POLL_WRITE;

    return ret;
}

/*
 * Portable usleep helper
 */
void mbedtls_net_usleep(unsigned long usec)
{
    aos_sleep((usec + 999) / 1000);
}

/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *)ctx)->fd;

    ret = check_fd(fd, 0);
    if (ret != 0)
        return ret;

    ret = (int)read(fd, buf, len);
    if (ret < 0) {
        if (net_would_block( ctx ) != 0)
            return MBEDTLS_ERR_SSL_WANT_READ;

        if (errno == EPIPE || errno == ECONNRESET)
            return MBEDTLS_ERR_NET_CONN_RESET;

        if (errno == EINTR)
            return MBEDTLS_ERR_SSL_WANT_READ;

        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return ret;
}

int mbedtls_net_recv_ext( void *ctx, unsigned char *buf, size_t len)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if (fd == 0 || fd == -1)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    ret = (int) mbedtls_read(fd, buf, len, NX_NO_WAIT);

    if(ret < 0) {
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return ret;
}


/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout(void *ctx, unsigned char *buf,
                             size_t len, uint32_t timeout)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    struct timeval tv;
    fd_set read_fds;
    int fd = ((mbedtls_net_context *)ctx)->fd;

    ret = check_fd(fd, 1);
    if (ret != 0)
        return ret;

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ret = select(fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv);

    /* Zero fds ready means we timed out */
    if (ret == 0)
        return MBEDTLS_ERR_SSL_TIMEOUT;

    if (ret < 0) {
        if (errno == EINTR)
            return MBEDTLS_ERR_SSL_WANT_READ;

        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    /* This call will not block */
    return mbedtls_net_recv(ctx, buf, len);
}

int mbedtls_net_recv_timeout_ext( void *ctx, unsigned char *buf,
                              size_t len, uint32_t timeout )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *)ctx)->fd;

    if (fd == 0 || fd == -1)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    /* This call will not block */
    ret = mbedtls_read(fd, buf, len, timeout);
    if(ret < 0) {
        if (ret == MBEDTLS_ERR_SSL_WANT_READ)
            return ret;

        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return ret;
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *)ctx)->fd;

    ret = check_fd(fd, 0);
    if (ret != 0)
        return ret;

    ret = (int)write(fd, buf, len);

    if (ret < 0) {
        if (net_would_block( ctx ) != 0)
            return MBEDTLS_ERR_SSL_WANT_WRITE;

        if (errno == EPIPE || errno == ECONNRESET)
            return MBEDTLS_ERR_NET_CONN_RESET;

        if (errno == EINTR)
            return MBEDTLS_ERR_SSL_WANT_WRITE;

        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    return ret;
}

int mbedtls_net_send_ext( void *ctx, const unsigned char *buf, size_t len)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *) ctx)->fd;
    NX_TCP_SOCKET *tcp_socket;
    NX_UDP_SOCKET *udp_socket;
    NX_PACKET *packet_ptr;
    int is_dtls = NX_FALSE;
    struct aic_network_ip_instance *ip_instance;

    if (fd == 0 || fd == -1)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    tcp_socket = (NX_TCP_SOCKET *)fd;
    if (tcp_socket->nx_tcp_socket_id != NX_TCP_ID) {
        udp_socket = (NX_UDP_SOCKET *)fd;
        if (udp_socket->nx_udp_socket_id == NX_UDP_ID)
            is_dtls = NX_TRUE;
    }
    ip_instance = aic_get_default_ip_instance();
    if (!ip_instance || !ip_instance->ip_ptr)
        return MBEDTLS_ERR_NET_SEND_FAILED;

    if (tcp_socket->nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        ret = nx_packet_allocate(ip_instance->packet_pool, &packet_ptr,
                                 NX_IPv4_PACKET, NX_IP_PERIODIC_RATE * 10);
    else
        ret = nx_packet_allocate(ip_instance->packet_pool, &packet_ptr,
                                 NX_IPv6_PACKET, NX_IP_PERIODIC_RATE * 10);
    if (ret != NX_SUCCESS) {
        aicx_info(0x1d1db0, "x", "[mbedtls] alloc packet fail ret 0x%x\n", ret);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    ret = nx_packet_data_append(packet_ptr, (VOID *)buf, len,
                                ip_instance->packet_pool,
                                NX_IP_PERIODIC_RATE * 10);
    if (ret != NX_SUCCESS)
    {
        nx_packet_release(packet_ptr);
        aicx_info(0x0b15eb, "x", "[mbedtls] packet append fail ret 0x%x\n", ret);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    ret = (int) mbedtls_write(fd, packet_ptr, len );

    if(ret != 0) {
        aicx_info(0x0a00e8, "x", "[mbedtls] mbedtls_write fail ret 0x%x\n", ret);
        nx_packet_release(packet_ptr);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    return len;
}

/*
 * Close the connection
 */
void mbedtls_net_close(mbedtls_net_context *ctx)
{
    if (ctx->fd == -1)
        return;

    close(ctx->fd);

    ctx->fd = -1;
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free(mbedtls_net_context *ctx)
{
    if (ctx->fd == -1)
        return;

    //shutdown( ctx->fd, 2 );
    close(ctx->fd);

    ctx->fd = -1;
}

int mbedtls_ssl_recv(mbedtls_ssl_context *ssl, NX_PACKET **packet_ptr_ptr)
{
    int len, actual_len, ret;
    NX_PACKET *packet_ptr, *current_packet, *new_packet;
    struct aic_network_ip_instance *ip_instance;

    *packet_ptr_ptr = NULL;
    ip_instance = aic_get_default_ip_instance();
    if (!ip_instance || !ip_instance->ip_ptr)
        return MBEDTLS_ERR_NET_RECV_FAILED;

    ret = nx_packet_allocate(ip_instance->packet_pool, &packet_ptr,
                             0, NX_IP_PERIODIC_RATE * 10);
    if (ret != NX_SUCCESS) {
        aicx_info(0x1d1db0, "x", "[mbedtls] alloc packet fail ret 0x%x\n", ret);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    len = mbedtls_ssl_read(ssl, packet_ptr->nx_packet_prepend_ptr,
        packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_prepend_ptr);
    if (len <= 0) {
        if (len == MBEDTLS_ERR_SSL_WANT_READ) {
            nx_packet_release(packet_ptr);
            return NX_NO_PACKET;
        }

        if (len == MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE) {
            nx_packet_release(packet_ptr);
            return MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE;
        }
        aicx_info(0x1c035c, "", "[mbedtls] recv NULL\n");
        nx_packet_release(packet_ptr);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    packet_ptr->nx_packet_length = len;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr + len;

    current_packet = packet_ptr;
    len = mbedtls_ssl_get_bytes_avail(ssl);
    while (len) {
        ret = nx_packet_allocate(ip_instance->packet_pool, &new_packet,
                                 0, NX_IP_PERIODIC_RATE * 10);
        if (ret != NX_SUCCESS) {
            aicx_info(0x1090ba, "x", "[mbedtls] loop alloc packet fail ret 0x%x\n", ret);
            break;
        }

        actual_len = mbedtls_ssl_read(ssl, new_packet->nx_packet_prepend_ptr,
                                      new_packet->nx_packet_data_end -
                                      new_packet->nx_packet_prepend_ptr);
        if (actual_len <= 0) {
            aicx_info(0x0c2f01, "", "[mbedtls] loop recv NULL\n");
            nx_packet_release(new_packet);
            break;
        }

        //new_packet->nx_packet_length = actual_len;
        new_packet->nx_packet_append_ptr =
            new_packet->nx_packet_prepend_ptr + actual_len;

        current_packet->nx_packet_next = new_packet;
        packet_ptr->nx_packet_last = new_packet;
        packet_ptr->nx_packet_length += actual_len;

        current_packet = new_packet;
        len -= actual_len;
    }
    *packet_ptr_ptr = packet_ptr;

    return NX_SUCCESS;
}


int mbedtls_ssl_send(mbedtls_ssl_context *ssl, NX_PACKET *packet_ptr)
{
    char *buf;
    int sendlen = 0, len, ret = 0;
    NX_PACKET *packet;

    packet = packet_ptr;
    do {
        buf = packet->nx_packet_prepend_ptr;
        len = packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr;

        ret = mbedtls_ssl_write(ssl, buf, len);
        if (ret >= 0)
            sendlen += ret;

        packet = packet->nx_packet_next;
    } while (packet);

    nx_packet_release(packet_ptr);
    return sendlen;
}

void aic_ssl_set_send_waitoption(int socketfd, uint32_t waitoption)
{
    int i = 0;

    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net &&
            mbedtls_net_list[i].net->fd == socketfd) {
            break;
        }
    }

    if (i < TLS_SOCKET_MAX)
        mbedtls_net_list[i].ssl_send_waitoption = waitoption;
}

void aic_ssl_set_server_info(int socketfd, NXD_ADDRESS *server, uint16_t port)
{
    int i = 0;

    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net &&
            mbedtls_net_list[i].net->fd == socketfd) {
            break;
        }
    }

    if (i < TLS_SOCKET_MAX) {
        mbedtls_net_list[i].server_port = port;
        mbedtls_net_list[i].server_addr.nxd_ip_version = server->nxd_ip_version;
        if (server->nxd_ip_version == NX_IP_VERSION_V4)
            mbedtls_net_list[i].server_addr.nxd_ip_address.v4 = server->nxd_ip_address.v4;
        else
            aos_memcpy(mbedtls_net_list[i].server_addr.nxd_ip_address.v6,
                       server->nxd_ip_address.v6, 16);
    }
}

void aic_ssl_set_recv_waitoption(int socketfd, uint32_t waitoption)
{
    int i = 0;

    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net &&
            mbedtls_net_list[i].net->fd == socketfd) {
            break;
        }
    }

    if (i < TLS_SOCKET_MAX)
        mbedtls_net_list[i].ssl_recv_waitoption = waitoption;
}

int aic_ssl_send(int fd, NX_PACKET *packet_ptr, int flag)
{
    int status = MBEDTLS_ERR_ERROR_GENERIC_ERROR, i = 0;
    NX_TCP_SOCKET *tcp_socket;
    NX_UDP_SOCKET *udp_socket;
    int is_dtls = NX_FALSE;
    uint32_t wait_option;

    if (fd == 0 || fd == -1)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net &&
            mbedtls_net_list[i].net->fd == fd) {
            break;
        }
    }

    if (i == TLS_SOCKET_MAX)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    tcp_socket = (NX_TCP_SOCKET *)fd;
    if (tcp_socket->nx_tcp_socket_id != NX_TCP_ID) {
        udp_socket = (NX_UDP_SOCKET *)fd;
        if (udp_socket->nx_udp_socket_id == NX_UDP_ID)
            is_dtls = NX_TRUE;
        else
            return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

    if (mbedtls_net_list[i].ssl_send_waitoption != 0)
        wait_option = mbedtls_net_list[i].ssl_send_waitoption;
    else if (flag == 0)
        wait_option = NX_IP_PERIODIC_RATE * 10;
    else
        wait_option = NX_NO_WAIT;

    if (is_dtls) {
        status = nxd_udp_socket_send(udp_socket, packet_ptr,
                                     &mbedtls_net_list[i].server_addr,
                                     mbedtls_net_list[i].server_port);
    } else {
        status = nx_tcp_socket_send(tcp_socket, packet_ptr, wait_option);
    }

    return status;
}

int  aic_ssl_recv(int fd, char *buf, int len, uint32_t timeout)
{
    int status = MBEDTLS_ERR_ERROR_GENERIC_ERROR, i = 0;
    NX_PACKET *packet_ptr;
    NX_TCP_SOCKET *tcp_socket;
    NX_UDP_SOCKET *udp_socket;
    uint32_t bytes_received, offset;
    int is_dtls = NX_FALSE;

    if (fd == 0 || fd == -1)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net &&
            mbedtls_net_list[i].net->fd == fd) {
            break;
        }
    }

    if (i == TLS_SOCKET_MAX)
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;

    timeout = mbedtls_net_list[i].ssl_recv_waitoption;

    tcp_socket = (NX_TCP_SOCKET *)fd;
    if (tcp_socket->nx_tcp_socket_id != NX_TCP_ID) {
        udp_socket = (NX_UDP_SOCKET *)fd;
        if (udp_socket->nx_udp_socket_id == NX_UDP_ID)
            is_dtls = NX_TRUE;
        else
            return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }
    do {
        packet_ptr = mbedtls_net_list[i].packet_header;
        if (packet_ptr)
            break;
        else {
            if (!is_dtls) {
                /* It is, check the socket TCP receive queue with a zero wait
                   option (no suspension).  */
                status = nx_tcp_socket_receive(tcp_socket, &packet_ptr,
                                               timeout);

                /* Check for no packet on the queue.  */
                if (status == NX_NOT_CONNECTED) {
                    /* Is peer shutdown orderly? */
                    if ((tcp_socket->nx_tcp_socket_state == NX_TCP_CLOSE_WAIT) ||
                        (tcp_socket->nx_tcp_socket_state >= NX_TCP_CLOSING)) {

                        /* Yes. Return 0. */
                        return(NX_SUCCESS);
                    }
                    return -1;
                }
            } else {
                status = nx_udp_socket_receive(udp_socket, &packet_ptr, timeout);
            }

            if (status == NX_NO_PACKET)
                return MBEDTLS_ERR_SSL_WANT_READ;

            if ((status == NX_SUCCESS) && (packet_ptr)) {
                mbedtls_net_list[i].packet_header = packet_ptr;
                mbedtls_net_list[i].recv_offset = 0;
                break;
            }
        }
    } while (1);
    offset = mbedtls_net_list[i].recv_offset;
    status = nx_packet_data_extract_offset(packet_ptr, offset, (VOID*)buf,
                                           (ULONG)len, &bytes_received);
    if (status) {
        nx_packet_release(packet_ptr);
        mbedtls_net_list[i].packet_header = NULL;
        return -1;
    }
    offset =  offset + bytes_received;

    /* Determine if all the packet data was consumed.  */
    if (packet_ptr->nx_packet_length <= offset) {
        mbedtls_net_list[i].packet_header = packet_ptr->nx_packet_queue_next;
        nx_packet_release(packet_ptr);
        mbedtls_net_list[i].recv_offset =  0;
    } else {
        mbedtls_net_list[i].recv_offset =  offset;
    }

    return bytes_received;
}

int mbedtls_ssl_set_socket(mbedtls_net_context *ctx, int socket_fd)
{
    int i;

    ctx->fd = socket_fd;
    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net == NULL) {
            mbedtls_net_list[i].net = ctx;
            break;
        }
    }

    if (i == TLS_SOCKET_MAX)
        return -1;

    return 0;
}

void mbedtls_ssl_reset_socket(mbedtls_net_context *ctx)
{
    int i;

    ctx->fd = -1;
    for (i = 0; i < TLS_SOCKET_MAX; i++) {
        if (mbedtls_net_list[i].net == ctx) {
            if (mbedtls_net_list[i].packet_header)
                nx_packet_release(mbedtls_net_list[i].packet_header);
            memset(&mbedtls_net_list[i], 0, sizeof(mbedtls_net_cfg));
            break;
        }
    }
}


#endif /* MBEDTLS_NET_C */
