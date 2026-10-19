/* coap_io_netxduo.c -- Network I/O functions for libcoap on netxduo
 *
 * Copyright (C) 2023 Qingsheng Li <qingsheng.li@aicxtek.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

/**
 * @file coap_io_netxduo.c
 * @brief NETXDUO specific functions
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ba89e
#define LOG_MODULE    AMOD_NET

#include "coap3/coap_internal.h"
#include <nxd_bsd.h>

void coap_packet_get_memmapped(coap_packet_t *packet, unsigned char **address, size_t *length)
{
    if (!packet || !packet->packet_ptr || address) {
        coap_log("[COAP] param is invalid when get memmapped\r\n");
        return;
    }

    *address = packet->packet_ptr->nx_packet_prepend_ptr;
    *length = packet->packet_ptr->nx_packet_length;
}
void coap_free_packet(coap_packet_t *packet)
{
    if (packet->packet_ptr)
        nx_packet_release(packet->packet_ptr);

    coap_free_type(COAP_PACKET, packet);
}

NX_PACKET *coap_packet_extract_pbuf(coap_packet_t *packet)
{
    NX_PACKET *ret = packet->packet_ptr;
    packet->packet_ptr = NULL;
    return ret;
}

/** Callback from NETXDUO when a package was received.
 *
 * The current implementation deals this to coap_dispatch immediately, but
 * other mechanisms (as storing the package in a queue and later fetching it
 * when coap_io_do_io is called) can be envisioned.
 *
 * It handles everything coap_io_do_io does on other implementations.
 */
static void coap_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
  coap_endpoint_t *ep = (coap_endpoint_t*)arg;
  coap_pdu_t *pdu = NULL;
  coap_session_t *session;
  coap_tick_t now;
  coap_packet_t *packet;

  if (p->len < 4) {
    /* Minimum size of CoAP header - ignore runt */
    return;
  }

  packet = coap_malloc_type(COAP_PACKET, sizeof(coap_packet_t));

  packet->pbuf = p;
  /* Need to do this as there may be holes in addr_info */
  memset(&packet->addr_info, 0, sizeof(packet->addr_info));
  packet->addr_info.remote.port = port;
  packet->addr_info.remote.addr = *addr;
  packet->addr_info.local.port = upcb->local_port;
  packet->addr_info.local.addr = *ip_current_dest_addr();
  packet->ifindex = netif_get_index(ip_current_netif());

  pdu = coap_pdu_from_packet(p);
  if (!pdu)
    goto error;

  if (!coap_pdu_parse(ep->proto, p->payload, p->len, pdu)) {
    goto error;
  }

  coap_ticks(&now);
  session = coap_endpoint_get_session(ep, packet, now);
  if (!session)
    goto error;
  LWIP_ASSERT("Proto not supported for LWIP", COAP_PROTO_NOT_RELIABLE(session->proto));
  coap_dispatch(ep->context, session, pdu);

  coap_delete_pdu(pdu);
  packet->pbuf = NULL;
  coap_free_packet(packet);
  return;

error:
  /*
   * https://tools.ietf.org/html/rfc7252#section-4.2 MUST send RST
   * https://tools.ietf.org/html/rfc7252#section-4.3 MAY send RST
   */
  if (session)
    coap_send_rst(session, pdu);
  coap_delete_pdu(pdu);
  if (packet) {
    packet->pbuf = NULL;
    coap_free_packet(packet);
  }
  return;
}

coap_endpoint_t *
coap_new_endpoint(coap_context_t *context, const coap_address_t *addr, coap_proto_t proto) {
        coap_endpoint_t *result;
        err_t err;

        LWIP_ASSERT("Proto not supported for LWIP endpoints", proto == COAP_PROTO_UDP);

        result = coap_malloc_type(COAP_ENDPOINT, sizeof(coap_endpoint_t));
        if (!result) return NULL;

        result->sock.pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
        if (result->sock.pcb == NULL) goto error;

        udp_recv(result->sock.pcb, coap_recv, (void*)result);
        err = udp_bind(result->sock.pcb, &addr->addr, addr->port);
        if (err) {
                udp_remove(result->sock.pcb);
                goto error;
        }

        result->default_mtu = COAP_DEFAULT_MTU;
        result->context = context;
        result->proto = proto;

        return result;

error:
        coap_free_type(COAP_ENDPOINT, result);
        return NULL;
}

void coap_free_endpoint(coap_endpoint_t *ep)
{
        udp_remove(ep->sock.pcb);
        coap_free_type(COAP_ENDPOINT, ep);
}

ssize_t
coap_socket_send_pdu(coap_socket_t *sock, coap_session_t *session,
  coap_pdu_t *pdu) {
  /* FIXME: we can't check this here with the existing infrastructure, but we
  * should actually check that the pdu is not held by anyone but us. the
  * respective pbuf is already exclusively owned by the pdu. */

  pbuf_realloc(pdu->pbuf, pdu->used_size + coap_pdu_parse_header_size(session->proto, pdu->pbuf->payload));
  udp_sendto(sock->pcb, pdu->pbuf, &session->addr_info.remote.addr,
    session->addr_info.remote.port);
  return pdu->used_size;
}

ssize_t
coap_socket_send(coap_socket_t *sock, coap_session_t *session,
  const uint8_t *data, size_t data_len ) {
  /* Not implemented, use coap_socket_send_pdu instead */
  return -1;
}

int
coap_socket_bind_udp(coap_socket_t *sock,
  const coap_address_t *listen_addr,
  coap_address_t *bound_addr) {
  return 0;
}

int
coap_socket_connect_udp(coap_socket_t *sock,
  const coap_address_t *local_if,
  const coap_address_t *server,
  int default_port,
  coap_address_t *local_addr,
  coap_address_t *remote_addr) {
  return 0;
}

int
coap_socket_connect_tcp1(coap_socket_t *sock,
                         const coap_address_t *local_if,
                         const coap_address_t *server,
                         int default_port,
                         coap_address_t *local_addr,
                         coap_address_t *remote_addr) {
  return 0;
}

int
coap_socket_connect_tcp2(coap_socket_t *sock,
                         coap_address_t *local_addr,
                         coap_address_t *remote_addr) {
  return 0;
}

int
coap_socket_bind_tcp(coap_socket_t *sock,
                     const coap_address_t *listen_addr,
                     coap_address_t *bound_addr) {
  return 0;
}

int
coap_socket_accept_tcp(coap_socket_t *server,
                        coap_socket_t *new_client,
                        coap_address_t *local_addr,
                        coap_address_t *remote_addr) {
  return 0;
}

ssize_t
coap_socket_write(coap_socket_t *sock, const uint8_t *data, size_t data_len) {
  return -1;
}

ssize_t
coap_socket_read(coap_socket_t *sock, uint8_t *data, size_t data_len) {
  return -1;
}

void coap_socket_close(coap_socket_t *sock) {
  return;
}

