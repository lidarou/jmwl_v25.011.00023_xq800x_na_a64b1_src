/* libcoap unit tests
 *
 * Copyright (C) 2013,2015-2018 Olaf Bergmann <bergmann@tzi.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */
#define AIC_THIS_FILE    0x0cd2ac
#define LOG_MODULE    AMOD_NET

#include "aic_errno.h"
#include "coap3/coap.h"
#include "coap3/coap_session_internal.h"
#include "aic_net_api.h"
#include "aic_cmd_api.h"

#if COAP_CLIENT_SUPPORT
#include <stdio.h>

/* The error threshold for timeout calculations. The precision of
 * coap_calc_timeout() is assumed to be sufficient if the resulting
 * value deviates at most FP_ERR_THRESHOLD percent from the value that
 * was calculated with double precision floating point arithmetic
 * instead of fixed point integers.
 */
#define FP_ERR_THRESHOLD (1.0)

/* The maximum number of bits for fixed point integer representation.
 * This number must be identical to the definition of MAX_BITS in
 * net.c
 */
#define MAX_BITS 8

static coap_context_t *ctx; /* Holds the coap context for most tests */
static coap_session_t *session; /* Holds a reference-counted session object */

COAP_STATIC_INLINE int
fpeq(const coap_fixed_point_t a, const coap_fixed_point_t b) {
  return (a.integer_part == b.integer_part) &&
    (a.fractional_part == b.fractional_part);
}

static void t_session1(void)
{
  assert(session->ref == 1);

  coap_session_reference(session);
  assert(session->ref == 2);

  coap_session_release(session);
  assert(session->ref == 1);
}

static void t_session2(void)
{
  assert(session->max_retransmit == COAP_DEFAULT_MAX_RETRANSMIT);
  assert(fpeq(session->ack_timeout, COAP_DEFAULT_ACK_TIMEOUT));
  assert(fpeq(session->ack_random_factor, COAP_DEFAULT_ACK_RANDOM_FACTOR));

  assert(coap_session_get_max_retransmit(session) == COAP_DEFAULT_MAX_RETRANSMIT);
  assert(fpeq(coap_session_get_ack_timeout(session), COAP_DEFAULT_ACK_TIMEOUT));
  assert(fpeq(coap_session_get_ack_random_factor(session), COAP_DEFAULT_ACK_RANDOM_FACTOR));
}

COAP_STATIC_INLINE double fp_to_double(const coap_fixed_point_t fp) {
  return fp.integer_part + fp.fractional_part/1000.0;
}

COAP_STATIC_INLINE double q_to_double(uint8_t q) {
  return (double)(q) / (1 << MAX_BITS);
}

/* Calculates the timeout using the following formula:
 *
 * COAP_TICKS_PER_SECOND * ack_timeout * (1 + (ack_random_factor - 1) * r)
 *
 * where @p r is the randomization value represented as Q0.MAX_BITS.
 * This function returns the result as unsigned int to be
 * checked against the result of coap_calc_timeout().
 */
COAP_STATIC_INLINE unsigned int timeout(const coap_fixed_point_t ack_timeout,
                                        const coap_fixed_point_t ack_random_factor,
                                        uint8_t r) {
  const unsigned int ctps = COAP_TICKS_PER_SECOND;
  double ato = fp_to_double(ack_timeout);
  double rnd = 1 + (fp_to_double(ack_random_factor) - 1.0) * q_to_double(r);

  return (unsigned int)(ctps * ato * rnd);
}

/* Checks if @p v is within FP_ERR_THRESHOLD percent of @p ref.  This
 * function returns 1 if it is within range, 0 otherwise.
 */
COAP_STATIC_INLINE int good_enough(unsigned int v, unsigned int ref) {
#define delta(a,b) (((a) < (b)) ? ((b) - (a)) : ((a) - (b)))
  return (delta(v,ref) / 100.0) <= FP_ERR_THRESHOLD;
}

static void t_session3(void) {
  const coap_fixed_point_t ato = COAP_DEFAULT_ACK_TIMEOUT;
  const coap_fixed_point_t arf = COAP_DEFAULT_ACK_RANDOM_FACTOR;

  assert(good_enough(coap_calc_timeout(session, 0), timeout(ato, arf, 0)));
  assert(good_enough(coap_calc_timeout(session, 12), timeout(ato, arf, 12)));
  assert(good_enough(coap_calc_timeout(session, 55), timeout(ato, arf, 55)));
  assert(good_enough(coap_calc_timeout(session, 83), timeout(ato, arf, 83)));
  assert(good_enough(coap_calc_timeout(session, 117), timeout(ato, arf, 117)));
  assert(good_enough(coap_calc_timeout(session, 210), timeout(ato, arf, 210)));
  assert(good_enough(coap_calc_timeout(session, 255), timeout(ato, arf, 255)));
}

static void t_session4(void) {
  const coap_fixed_point_t ato = {8,300};
  const coap_fixed_point_t arf = {2,011};

  coap_session_set_ack_timeout(session, ato);
  coap_session_set_ack_random_factor(session, arf);

  assert(fpeq(coap_session_get_ack_timeout(session), ato));
  assert(fpeq(coap_session_get_ack_random_factor(session), arf));

  assert(good_enough(coap_calc_timeout(session, 0), timeout(ato, arf, 0)));
  assert(good_enough(coap_calc_timeout(session, 12), timeout(ato, arf, 12)));
  assert(good_enough(coap_calc_timeout(session, 55), timeout(ato, arf, 55)));
  assert(good_enough(coap_calc_timeout(session, 83), timeout(ato, arf, 83)));
  assert(good_enough(coap_calc_timeout(session, 117), timeout(ato, arf, 117)));
  assert(good_enough(coap_calc_timeout(session, 210), timeout(ato, arf, 210)));
  assert(good_enough(coap_calc_timeout(session, 255), timeout(ato, arf, 255)));
}

static void t_session5(void) {
  coap_session_release(session);

  assert(!ctx->sessions);
  session = NULL;
}

/* Test 6 creates a new session after the original session has been
 * released by test 5 */
static void t_session6(void) {
  coap_address_t laddr, saddr;
  coap_address_init(&laddr);
  coap_address_init(&saddr);

  assert(!session);

  laddr.size = sizeof(struct sockaddr_in6);
  laddr.addr.sin6.sin6_family = AF_INET6;
  laddr.addr.sin6.sin6_addr = in6addr_any;
  laddr.addr.sin6.sin6_port = htons(COAP_DEFAULT_PORT);

  coap_address_copy(&saddr, &laddr);
  saddr.addr.sin6.sin6_addr = IN6ADDR_LOOPBACK_INIT;
  saddr.addr.sin6.sin6_port = htons(0xC00A);

  coap_log(LOG_CRIT, "[COAP]0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
           saddr.addr.sin6.sin6_addr._S6_un._S6_u32[0],
           saddr.addr.sin6.sin6_addr._S6_un._S6_u32[1],
           saddr.addr.sin6.sin6_addr._S6_un._S6_u32[2],
           saddr.addr.sin6.sin6_addr._S6_un._S6_u32[3],
           ntohl(saddr.addr.sin6.sin6_addr._S6_un._S6_u32[0]),
           ntohl(saddr.addr.sin6.sin6_addr._S6_un._S6_u32[1]),
           ntohl(saddr.addr.sin6.sin6_addr._S6_un._S6_u32[2]),
           ntohl(saddr.addr.sin6.sin6_addr._S6_un._S6_u32[3]));
  session = coap_new_client_session(ctx, &saddr, &laddr, COAP_PROTO_UDP);
  assert(session);
  assert(ctx->sessions);
  assert(session->state == COAP_SESSION_STATE_ESTABLISHED);
  coap_session_release(session);
}

/* This function creates a set of nodes for testing. These nodes
 * will exist for all tests and are modified by coap_insert_node()
 * and coap_remove_from_queue().
 */
static int t_session_tests_create(void) {
  coap_address_t addr;
  coap_address_init(&addr);

  addr.size = sizeof(struct sockaddr_in6);
  addr.addr.sin6.sin6_family = AF_INET6;
  addr.addr.sin6.sin6_addr = in6addr_any;
  addr.addr.sin6.sin6_port = htons(COAP_DEFAULT_PORT);

  ctx = coap_new_context(&addr);

  if (ctx != NULL) {
    addr.addr.sin6.sin6_addr = IN6ADDR_LOOPBACK_INIT;
    session = coap_new_client_session(ctx, NULL, &addr, COAP_PROTO_UDP);
  }

  return (ctx == NULL) || (session == NULL);
}

static int t_session_tests_remove(void) {
  coap_free_context(ctx);
  return 0;
}

static int t_init_session_tests(int argc, char **argv)
{
    coap_set_log_level(8);

    coap_startup();

    t_session_tests_create();

    t_session1();
    t_session2();
    t_session3();
    t_session4();
    t_session5();
    t_session6();

    t_session_tests_remove();

    CONS_OUT("%d %d %d %d %d\r\n", 10 % 100, 100 % 10, 100 % 100, 0 % 10, 101 % 100);

    return AIC_SUCCESS;
}
CONSOLE_EXPORT_CMD(coap_session_test, t_init_session_tests, coap session test);

coap_pdu_t *pdu;              /* Holds the parsed PDU for most tests */

/************************************************************************
 ** PDU decoder
 ************************************************************************/

static void t_parse_pdu1(void) {
  uint8_t teststr[] = {0x40, 0x01, 0x93, 0x34};
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_CON);
  assert(pdu->token_length == 0);
  assert(pdu->code == COAP_REQUEST_CODE_GET);
  assert(pdu->mid == 0x9334);
  assert(!pdu->data);
}

static void t_parse_pdu2(void) {
  uint8_t teststr[] = {0x55, 0x69, 0x12, 0x34, 't', 'o', 'k', 'e', 'n'};
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_NON);
  assert(pdu->token_length == 5);
  assert(pdu->code == 0x69);
  assert(pdu->mid == 0x1234);
  assert(memcmp(pdu->token, teststr + 4, 5) == 0);
  assert(!pdu->data);
}

static void t_parse_pdu3(void) {
  uint8_t teststr[] = {0x53, 0x69, 0x12, 0x34, 't', 'o', 'k', 'e', 'n' };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void t_parse_pdu4(void) {
  /* illegal token length */
  uint8_t teststr[] = {  0x59, 0x69, 0x12, 0x34,
                      't', 'o', 'k', 'e', 'n', '1', '2', '3', '4' };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);

  teststr[0] = 0x5f;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void t_parse_pdu5(void) {
  /* PDU with options */
  uint8_t teststr[] = {  0x55, 0x73, 0x12, 0x34, 't', 'o', 'k', 'e',
                      'n',  0x00, 0xc1, 0x00
  };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_NON);
  assert(pdu->token_length == 5);
  assert(pdu->code == 0x73);
  assert(pdu->mid == 0x1234);
  assert(memcmp(pdu->token, teststr + 4, 5) == 0);
  assert(!pdu->data);

  /* FIXME: check options */
}

static void t_parse_pdu6(void) {
  /* PDU with options that exceed the PDU */
  uint8_t teststr[] = {  0x55, 0x73, 0x12, 0x34, 't', 'o', 'k', 'e',
                      'n',  0x00, 0xc1, 0x00, 0xae, 0xf0, 0x03
  };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void t_parse_pdu7(void) {
  /* PDU with options and payload */
  uint8_t teststr[] = {0x55, 0x73, 0x12, 0x34, 't', 'o', 'k', 'e',
                       'n',  0x00, 0xc1, 0x00, 0xff, 'p', 'a', 'y',
                       'l', 'o', 'a', 'd'};
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_NON);
  assert(pdu->token_length == 5);
  assert(pdu->code == 0x73);
  assert(pdu->mid == 0x1234);
  assert(memcmp(pdu->token, teststr + 4, 5) == 0);

  /* FIXME: check options */

  assert(pdu->data == pdu->token + 9);
  assert(memcmp(pdu->data, teststr + 13, 7) == 0);
}

static void t_parse_pdu8(void) {
  /* PDU without options but with payload */
  uint8_t teststr[] = {0x50, 0x73, 0x12, 0x34,
                       0xff, 'p', 'a', 'y', 'l', 'o', 'a',
                       'd'};
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_NON);
  assert(pdu->token_length == 0);
  assert(pdu->code == 0x73);
  assert(pdu->mid == 0x1234);

  /* FIXME: check options */

  assert(pdu->data == pdu->token + 1);
  assert(memcmp(pdu->data, teststr + 5, 7) == 0);
}

static void t_parse_pdu9(void) {
  /* PDU without options and payload but with payload start marker */
  uint8_t teststr[] = {  0x70, 0x00, 0x12, 0x34, 0xff };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void t_parse_pdu10(void) {
  /* PDU without payload but with options and payload start marker */
  uint8_t teststr[] = {  0x53, 0x73, 0x12, 0x34, 't', 'o', 'k',
                      0x31, 'a', 0xc1, 0x00, 0xff
  };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void t_parse_pdu11(void) {
  uint8_t teststr[] = {  0x60, 0x00, 0x12, 0x34 };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_ACK);
  assert(pdu->token_length == 0);
  assert(pdu->code == 0);
  assert(pdu->mid == 0x1234);
}

static void t_parse_pdu12(void) {
  /* RST */
  uint8_t teststr[] = {  0x70, 0x00, 0x12, 0x34 };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result > 0);

  assert(pdu->used_size == sizeof(teststr) - 4);
  assert(pdu->type == COAP_MESSAGE_RST);
  assert(pdu->token_length == 0);
  assert(pdu->code == 0);
  assert(pdu->mid == 0x1234);
}

static void t_parse_pdu13(void) {
  /* RST with content */
  uint8_t teststr[] = {0x70, 0x00, 0x12, 0x34,
                       0xff, 'c', 'o', 'n', 't', 'e', 'n', 't'
  };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void t_parse_pdu14(void) {
  /* ACK with content */
  uint8_t teststr[] = {  0x60, 0x00, 0x12, 0x34,
                      0xff, 'c', 'o', 'n', 't', 'e', 'n', 't'
  };
  int result;

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

/*
 * To test Issue #199 which reads one byte past the end of teststr[]
 * before fix to coap_opt_parse() as delta is two byte value and only
 * one byte left
 * Credit to OSS-Fuzz for finding this, work done by Bhargava Shastry
 */
static void t_parse_pdu15(void) {
  int result;
  uint8_t teststr[] = {
  64,  91,  91,  91, 139,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  91,  91, 224, 224, 224,
 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224,
 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224,   1,   0,   0,   0,
   0,   0,   0,   0, 224, 192, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224,
 224, 224, 224, 224, 224, 224, 224, 224, 228, 224, 224, 224, 224, 224, 224, 224,
 224, 224, 224, 224, 224, 224, 224, 224, 224,  91,  91,  91,  91,  91,  91,  91,
  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,
  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,
  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,
  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,
  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,
  91,  91,  91,  91,  91,  91,  91, 224, 224, 224, 224, 224, 224, 224, 224, 224,
 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224,
 224, 224, 224, 224, 224, 224,   1,   0,   0,   0,   0,   0,   0,   0, 224, 224,
 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224, 224,
 224, 224, 224, 224, 224};

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

static void log_handler(coap_log_t level, const char *message)
{
  (void)level;
  (void)message;
}

/*
 * To test Issue #214 which allows the token size to be set larger than the
 * decoded PDU in coap_pdu_parse_header().  This then causes coap_show_pdu()
 * to access invalid memory.
 * Credit to OSS-Fuzz for finding this, work done by Bhargava Shastry
 */
static void t_parse_pdu16(void) {
  int result;
  coap_pdu_t *testpdu;
  uint8_t teststr[] = { 0x5a, 0x0a, 0x5b, 0x5b };

  testpdu = coap_pdu_init(0, 0, 0, sizeof(teststr));
  assert(testpdu != NULL);

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), testpdu);
  assert(result == 0);

  coap_set_show_pdu_output(0);
  coap_set_log_handler(log_handler);
  coap_show_pdu(LOG_ERR, testpdu);        /* display PDU */
  coap_set_log_handler(NULL);

  coap_delete_pdu(testpdu);
}

static void t_parse_pdu17(void) {
  uint8_t teststr[512] = {  0x40, 0x01, 0x93, 0x34 };
  size_t idx;
  int result;

  /* 245 * option delta 268 > 65535, causing a overflow in the option
   * number */
  for (idx = 4; idx < sizeof(teststr) - 4; idx += 2) {
    teststr[idx] = 0xd0;     /* 1 byte option delta follows */
    teststr[idx + 1] = 0xff; /* option delta 268 */
  }

  result = coap_pdu_parse(COAP_PROTO_UDP, teststr, sizeof(teststr), pdu);
  assert(result == 0);
}

/************************************************************************
 ** PDU encoder
 ************************************************************************/

static void t_encode_pdu1(void) {
  uint8_t teststr[] = { 0x45, 0x01, 0x12, 0x34, 't', 'o', 'k', 'e', 'n' };
  int result;

  coap_pdu_clear(pdu, pdu->max_size);
  pdu->type = COAP_MESSAGE_CON;
  pdu->code = COAP_REQUEST_CODE_GET;
  pdu->mid = 0x1234;

  result = coap_add_token(pdu, 5, (const uint8_t *)"token");

  assert(result == 1);
  assert(pdu->used_size == 5);
  assert(!pdu->data);
  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu2(void) {
  size_t old_max = pdu->max_size;
  int result;

  coap_pdu_clear(pdu, 3);        /* set very small PDU size */

  pdu->type = COAP_MESSAGE_CON;
  pdu->code = COAP_REQUEST_CODE_GET;
  pdu->mid = 0x1234;

  result = coap_add_token(pdu, 5, (const uint8_t *)"token");

  assert(result == 0);

  coap_pdu_clear(pdu, old_max);        /* restore PDU size */
}

static void t_encode_pdu3(void) {
  int result;

  result = coap_add_token(pdu, 9, (const uint8_t *)"123456789");

  assert(result == 0);
}

static void t_encode_pdu4(void) {
  /* PDU with options */
  uint8_t teststr[] = { 0x60, 0x99, 0x12, 0x34, 0x3d, 0x05, 0x66, 0x61,
                     0x6e, 0x63, 0x79, 0x70, 0x72, 0x6f, 0x78, 0x79,
                     0x2e, 0x63, 0x6f, 0x61, 0x70, 0x2e, 0x6d, 0x65,
                     0x84, 0x70, 0x61, 0x74, 0x68, 0x00, 0xe8, 0x1e,
                     0x28, 0x66, 0x61, 0x6e, 0x63, 0x79, 0x6f, 0x70,
                     0x74
  };
  int result;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_ACK;
  pdu->code = 0x99;
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_option(pdu, COAP_OPTION_URI_HOST,
       18, (const uint8_t *)"fancyproxy.coap.me");

  assert(result == 20);
  assert(pdu->max_opt == 3);
  assert(pdu->used_size == 20);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_URI_PATH,
                           4, (const uint8_t *)"path");

  assert(result == 5);
  assert(pdu->max_opt == 11);
  assert(pdu->used_size == 25);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_URI_PATH, 0, NULL);

  assert(result == 1);
  assert(pdu->max_opt == 11);
  assert(pdu->used_size == 26);
  assert(!pdu->data);

  result = coap_add_option(pdu, 8000, 8, (const uint8_t *)"fancyopt");

  assert(result == 11);
  assert(pdu->max_opt == 8000);
  assert(pdu->used_size == 37);
  assert(!pdu->data);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu5(void) {
  /* PDU with token and options */
  uint8_t teststr[] = { 0x68, 0x84, 0x12, 0x34, '1',  '2',  '3',  '4',
                       '5',  '6',  '7',  '8',  0x18, 0x41, 0x42, 0x43,
                       0x44, 0x45, 0x46, 0x47, 0x48, 0xd1, 0x03, 0x12
  };
  int result;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_ACK;
  pdu->code = COAP_RESPONSE_CODE(404);
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_token(pdu, 8, (const uint8_t *)"12345678");

  assert(pdu->used_size == 8);

  result = coap_add_option(pdu, COAP_OPTION_IF_MATCH,
                           8, (const uint8_t *)"ABCDEFGH");

  assert(result == 9);
  assert(pdu->max_opt == 1);
  assert(pdu->used_size == 17);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_ACCEPT,
                           1, (const uint8_t *)"\x12");

  assert(result == 3);
  assert(pdu->max_opt == 17);
  assert(pdu->used_size == 20);
  assert(!pdu->data);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu6(void) {
  /* PDU with data */
  uint8_t teststr[] = { 0x50, 0x02, 0x12, 0x34, 0xff, '1',  '2',  '3',
                     '4', '5',  '6',  '7',  '8'
  };
  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_NON;
  pdu->code = COAP_REQUEST_CODE_POST;
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);
  assert(!pdu->data);

  coap_add_data(pdu, 8, (const uint8_t *)"12345678");

  assert(pdu->used_size == 9);
  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu7(void) {
  /* PDU with empty data */
  uint8_t teststr[] = { 0x40, 0x43, 0x12, 0x34 };
  int result;
  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_CON;
  pdu->code = COAP_RESPONSE_CODE(203);
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_data(pdu, 0, NULL);

  assert(result > 0);
  assert(pdu->used_size == 0);
  assert(!pdu->data);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu8(void) {
  /* PDU with token and data */
  uint8_t teststr[] = { 0x42, 0x43, 0x12, 0x34, 0x00, 0x01, 0xff, 0x00 };
  int result;
  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_CON;
  pdu->code = COAP_RESPONSE_CODE(203);
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_token(pdu, 2, (const uint8_t *)"\x00\x01");

  assert(result > 0);

  result = coap_add_data(pdu, 1, (const uint8_t *)"\0");

  assert(result > 0);
  assert(pdu->used_size == 4);
  assert(pdu->data == pdu->token + 3);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu9(void) {
  /* PDU with options and data */
  uint8_t teststr[] = { 0x60, 0x44, 0x12, 0x34, 0x48, 's',  'o',  'm',
                     'e',  'e',  't',  'a',  'g',  0x10, 0xdd, 0x11,
                     0x04, 's',  'o',  'm',  'e',  'r',  'a',  't',
                     'h',  'e',  'r',  'l',  'o',  'n',  'g',  'u',
                     'r',  'i',  0xff, 'd',  'a',  't',  'a'
  };
  int result;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_ACK;
  pdu->code = COAP_RESPONSE_CODE(204);
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_option(pdu, COAP_OPTION_ETAG, 8, (const uint8_t *)"someetag");

  assert(result == 9);
  assert(pdu->max_opt == 4);
  assert(pdu->used_size == 9);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_IF_NONE_MATCH, 0, NULL);

  assert(result == 1);
  assert(pdu->max_opt == 5);
  assert(pdu->used_size == 10);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_PROXY_URI,
                           17, (const uint8_t *)"someratherlonguri");

  assert(result == 20);
  assert(pdu->max_opt == 35);
  assert(pdu->used_size == 30);
  assert(!pdu->data);

  result = coap_add_data(pdu, 4, (const uint8_t *)"data");

  assert(result > 0);
  assert(pdu->used_size == 35);
  assert(pdu->data == pdu->token + 31);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu10(void) {
  /* PDU with token, options and data */
  uint8_t teststr[] = { 0x62, 0x44, 0x12, 0x34, 0x00, 0x00, 0x8d, 0xf2,
                     'c',  'o',  'a',  'p',  ':',  '/',  '/',  'e',
                     'x',  'a',  'm',  'p',  'l',  'e',  '.',  'c',
                     'o',  'm',  '/',  '1',  '2',  '3',  '4',  '5',
                     '/',  '%',  '3',  'F',  'x',  'y',  'z',  '/',
                     '3',  '0',  '4',  '8',  '2',  '3',  '4',  '2',
                     '3',  '4',  '/',  '2',  '3',  '4',  '0',  '2',
                     '3',  '4',  '8',  '2',  '3',  '4',  '/',  '2',
                     '3',  '9',  '0',  '8',  '4',  '2',  '3',  '4',
                     '-',  '2',  '3',  '/',  '%',  'A',  'B',  '%',
                     '3',  '0',  '%',  'a',  'f',  '/',  '+',  '1',
                     '2',  '3',  '/',  'h',  'f',  'k',  's',  'd',
                     'h',  '/',  '2',  '3',  '4',  '8',  '0',  '-',
                     '2',  '3',  '4',  '-',  '9',  '8',  '2',  '3',
                     '5',  '/',  '1',  '2',  '0',  '4',  '/',  '2',
                     '4',  '3',  '5',  '4',  '6',  '3',  '4',  '5',
                     '3',  '4',  '5',  '2',  '4',  '3',  '/',  '0',
                     '1',  '9',  '8',  's',  'd',  'n',  '3',  '-',
                     'a',  '-',  '3',  '/',  '/',  '/',  'a',  'f',
                     'f',  '0',  '9',  '3',  '4',  '/',  '9',  '7',
                     'u',  '2',  '1',  '4',  '1',  '/',  '0',  '0',
                     '0',  '2',  '/',  '3',  '9',  '3',  '2',  '4',
                     '2',  '3',  '5',  '3',  '2',  '/',  '5',  '6',
                     '2',  '3',  '4',  '0',  '2',  '3',  '/',  '-',
                     '-',  '-',  '-',  '/',  '=',  '1',  '2',  '3',
                     '4',  '=',  '/',  '0',  '9',  '8',  '1',  '4',
                     '1',  '-',  '9',  '5',  '6',  '4',  '6',  '4',
                     '3',  '/',  '2',  '1',  '9',  '7',  '0',  '-',
                     '-',  '-',  '-',  '-',  '/',  '8',  '2',  '3',
                     '6',  '4',  '9',  '2',  '3',  '4',  '7',  '2',
                     'w',  'e',  'r',  'e',  'r',  'e',  'w',  'r',
                     '0',  '-',  '9',  '2',  '1',  '-',  '3',  '9',
                     '1',  '2',  '3',  '-',  '3',  '4',  '/',  0x0d,
                     0x01, '/',  '/',  '4',  '9',  '2',  '4',  '0',
                     '3',  '-',  '-',  '0',  '9',  '8',  '/',  0xc1,
                     '*',  0xff, 'd',  'a',  't',  'a'
  };
  int result;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_ACK;
  pdu->code = COAP_RESPONSE_CODE(204);
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_token(pdu, 2, (const uint8_t *)"\0\0");

  assert(result > 0);
  result = coap_add_option(pdu, COAP_OPTION_LOCATION_PATH, 255,
                           (const uint8_t *)"coap://example.com/12345/%3Fxyz/3048234234/23402348234/239084234-23/%AB%30%af/+123/hfksdh/23480-234-98235/1204/243546345345243/0198sdn3-a-3///aff0934/97u2141/0002/3932423532/56234023/----/=1234=/098141-9564643/21970-----/82364923472wererewr0-921-39123-34/");

  assert(result == 257);
  assert(pdu->max_opt == 8);
  assert(pdu->used_size == 259);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_LOCATION_PATH, 14,
                           (const uint8_t *)"//492403--098/");

  assert(result == 16);
  assert(pdu->max_opt == 8);
  assert(pdu->used_size == 275);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_LOCATION_QUERY,
                           1, (const uint8_t *)"*");

  assert(result == 2);
  assert(pdu->max_opt == 20);
  assert(pdu->used_size == 277);
  assert(!pdu->data);

  result = coap_add_data(pdu, 4, (const uint8_t *)"data");

  assert(result > 0);
  assert(pdu->used_size == 282);
  assert(pdu->data == pdu->token + 278);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);
}

static void t_encode_pdu11(void) {
  coap_log_t level = coap_get_log_level();
  /* data too long for PDU */
  size_t old_max = pdu->max_size;
  int result;

  coap_pdu_clear(pdu, 8);        /* clear PDU, with small maximum */

  assert(pdu->data == NULL);
  coap_set_log_level(LOG_CRIT);
  result = coap_add_data(pdu, 10, (const uint8_t *)"0123456789");
  coap_set_log_level(level);

  assert(result == 0);
  assert(pdu->data == NULL);
  assert(pdu->used_size == 0);

  pdu->max_size = old_max;
}

static void t_encode_pdu12(void) {
  coap_optlist_t *optlist = NULL;
  int n;
  uint8_t opt_num[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 };
  uint8_t opt_val[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 };
  uint8_t opt_srt[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 };
  coap_opt_iterator_t oi;
  coap_opt_t *option;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  for (n = 0; n < (int)sizeof(opt_num); n++) {
    coap_insert_optlist(&optlist, coap_new_optlist(opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]));
  }
  coap_add_optlist_pdu(pdu, &optlist);

  /* Check options in pdu are in right order */
  coap_option_iterator_init(pdu, &oi, COAP_OPT_ALL);
  for (n = 0; n < (int)sizeof(opt_num); n++) {
    option = coap_option_next(&oi);
    assert(oi.bad == 0);
    assert(option != NULL);
    assert(coap_opt_length(option) == 1);
    assert(*coap_opt_value(option) == opt_srt[n]);
  }
  option = coap_option_next(&oi);
  assert(oi.bad == 1);
  assert(option == NULL);
  coap_delete_optlist(optlist);
}

static void t_encode_pdu13(void) {
  coap_optlist_t *optlist = NULL;
  int n;
  uint8_t opt_num[] = { 59, 58, 57, 56, 55, 54, 53, 52, 51, 50 };
  uint8_t opt_val[] = { 59, 58, 57, 56, 55, 54, 53, 52, 51, 50 };
  uint8_t opt_srt[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 };
  coap_opt_iterator_t oi;
  coap_opt_t *option;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  for (n = 0; n < (int)sizeof(opt_num); n++) {
    coap_insert_optlist(&optlist, coap_new_optlist(opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]));
  }
  coap_add_optlist_pdu(pdu, &optlist);

  /* Check options in pdu are in right order */
  coap_option_iterator_init(pdu, &oi, COAP_OPT_ALL);
  for (n = 0; n < (int)sizeof(opt_num); n++) {
    option = coap_option_next(&oi);
    assert(oi.bad == 0);
    assert(option != NULL);
    assert(coap_opt_length(option) == 1);
    assert(*coap_opt_value(option) == opt_srt[n]);
  }
  option = coap_option_next(&oi);
  assert(oi.bad == 1);
  assert(option == NULL);
  coap_delete_optlist(optlist);
}

static void t_encode_pdu14(void) {
  coap_optlist_t *optlist = NULL;
  int n;
  uint8_t opt_num[] = { 53, 52, 51, 50, 51, 52, 52, 51, 50, 50 };
  uint8_t opt_val[] = { 59, 56, 53, 50, 54, 57, 58, 55, 51, 52 };
  uint8_t opt_srt[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 };
  coap_opt_iterator_t oi;
  coap_opt_t *option;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  for (n = 0; n < (int)sizeof(opt_num); n++) {
    coap_insert_optlist(&optlist, coap_new_optlist(opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]));
  }
  coap_add_optlist_pdu(pdu, &optlist);

  /* Check options in pdu are in right order */
  coap_option_iterator_init(pdu, &oi, COAP_OPT_ALL);
  for (n = 0; n < (int)sizeof(opt_num); n++) {
    option = coap_option_next(&oi);
    assert(oi.bad == 0);
    assert(option != NULL);
    assert(coap_opt_length(option) == 1);
    assert(*coap_opt_value(option) == opt_srt[n]);
  }
  option = coap_option_next(&oi);
  assert(oi.bad == 1);
  assert(option == NULL);
  coap_delete_optlist(optlist);
}

/* Check inserting options with random types get put into the PDU in the
   right order */
static void t_encode_pdu15(void) {
  size_t n;
  uint16_t opt_num[] = { 300,  13,  10,   7,  11, 268, 269,  12,   8,   9 };
  uint8_t  opt_val[] = {  59,  56,  53,  50,  54,  57,  58,  55,  51,  52 };
  uint8_t  opt_srt[] = {  50,  51,  52,  53,  54,  55,  56,  57,  58,  59 };
  coap_opt_iterator_t oi;
  coap_opt_t *option;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_insert_option(pdu, opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]);
  }

  /* Check options in pdu are in right order */
  coap_option_iterator_init(pdu, &oi, COAP_OPT_ALL);
  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    option = coap_option_next(&oi);
    assert(oi.bad == 0);
    assert(option != NULL);
    assert(coap_opt_length(option) == 1);
    assert(*coap_opt_value(option) == opt_srt[n]);
  }
  option = coap_option_next(&oi);
  assert(oi.bad == 1);
  assert(option == NULL);
}

/* Check changing value of options works */
static void t_encode_pdu16(void) {
  size_t n;
  uint16_t opt_num[] = { 300,  10,   7 };
  uint8_t  opt_val[] = {  53,  51,  50 };
  uint8_t  data[] = { 'd', 'a', 't', 'a' };
  uint8_t  data1[] = { 0x71, 0x32, 0x31, 0x33, 0xe1, 0x00, 0x15, 0x35,
                       0xff, 0x64, 0x61, 0x74, 0x61 };
  uint8_t  data2[] = { 0x71, 0x32, 0x33, 0x01, 0x23, 0x45, 0xe1, 0x00,
                       0x15, 0x35, 0xff, 0x64, 0x61, 0x74, 0x61 };
  uint8_t  data3[] = { 0x70, 0x31, 0x33, 0xe1, 0x00, 0x15, 0x35, 0xff,
                       0x64, 0x61, 0x74, 0x61 };
  uint8_t  data4[] = { 0x71, 0x32, 0x31, 0x33, 0xe4, 0x00, 0x15, 0x06,
                       0x54, 0x32, 0x10, 0xff, 0x64, 0x61, 0x74, 0x61 };
  int new_val;
  unsigned char buf[4];

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_add_option(pdu, opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]);
  }
  coap_add_data(pdu, sizeof(data), data);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
  /* Now update an option in the middle */
  new_val = 0x12345;
  coap_update_option(pdu, 10,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data2, pdu->used_size) == 0);
  /* Shrink it back again */
  new_val = 51;
  coap_update_option(pdu, 10,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
  /* Now update an option at the start */
  new_val = 0;
  coap_update_option(pdu, 7,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data3, pdu->used_size) == 0);
  /* put it back again */
  new_val = 50;
  coap_update_option(pdu, 7,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
  /* Now update an option at the end */
  new_val = 0x6543210;
  coap_update_option(pdu, 300,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data4, pdu->used_size) == 0);
  /* put it back again */
  new_val = 53;
  coap_update_option(pdu, 300,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
}

/* Same as t_encode_pdu16, but without any data, but with a token */
static void
t_encode_pdu17(void) {
  size_t n;
  uint8_t  token[] = { 't' };
  uint16_t opt_num[] = { 300,  10,   7 };
  uint8_t  opt_val[] = {  53,  51,  50 };
  uint8_t  data1[] = { 0x74, 0x71, 0x32, 0x31, 0x33, 0xe1, 0x00, 0x15,
                       0x35 };
  uint8_t  data2[] = { 0x74, 0x71, 0x32, 0x33, 0x01, 0x23, 0x45, 0xe1,
                       0x00, 0x15, 0x35 };
  uint8_t  data3[] = { 0x74, 0x70, 0x31, 0x33, 0xe1, 0x00, 0x15, 0x35 };
  uint8_t  data4[] = { 0x74, 0x71, 0x32, 0x31, 0x33, 0xe4, 0x00, 0x15,
                       0x06, 0x54, 0x32, 0x10 };
  int new_val;
  unsigned char buf[4];

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  coap_add_token(pdu, sizeof(token), token);
  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_add_option(pdu, opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]);
  }
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
  /* Now update an option in the middle */
  new_val = 0x12345;
  coap_update_option(pdu, 10,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data2, pdu->used_size) == 0);
  /* Shrink it back again */
  new_val = 51;
  coap_update_option(pdu, 10,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
  /* Now update an option at the start */
  new_val = 0;
  coap_update_option(pdu, 7,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data3, pdu->used_size) == 0);
  /* put it back again */
  new_val = 50;
  coap_update_option(pdu, 7,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
  /* Now update an option at the end */
  new_val = 0x6543210;
  coap_update_option(pdu, 300,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data4, pdu->used_size) == 0);
  /* put it back again */
  new_val = 53;
  coap_update_option(pdu, 300,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);
}

static void t_encode_pdu18(void) {
  /* PDU with token, options and data */
  uint8_t teststr[] = { 0x62, 0x44, 0x12, 0x34, 0x00, 0x00, 0x8d, 0xf2,
                     'c',  'o',  'a',  'p',  ':',  '/',  '/',  'e',
                     'x',  'a',  'm',  'p',  'l',  'e',  '.',  'c',
                     'o',  'm',  '/',  '1',  '2',  '3',  '4',  '5',
                     '/',  '%',  '3',  'F',  'x',  'y',  'z',  '/',
                     '3',  '0',  '4',  '8',  '2',  '3',  '4',  '2',
                     '3',  '4',  '/',  '2',  '3',  '4',  '0',  '2',
                     '3',  '4',  '8',  '2',  '3',  '4',  '/',  '2',
                     '3',  '9',  '0',  '8',  '4',  '2',  '3',  '4',
                     '-',  '2',  '3',  '/',  '%',  'A',  'B',  '%',
                     '3',  '0',  '%',  'a',  'f',  '/',  '+',  '1',
                     '2',  '3',  '/',  'h',  'f',  'k',  's',  'd',
                     'h',  '/',  '2',  '3',  '4',  '8',  '0',  '-',
                     '2',  '3',  '4',  '-',  '9',  '8',  '2',  '3',
                     '5',  '/',  '1',  '2',  '0',  '4',  '/',  '2',
                     '4',  '3',  '5',  '4',  '6',  '3',  '4',  '5',
                     '3',  '4',  '5',  '2',  '4',  '3',  '/',  '0',
                     '1',  '9',  '8',  's',  'd',  'n',  '3',  '-',
                     'a',  '-',  '3',  '/',  '/',  '/',  'a',  'f',
                     'f',  '0',  '9',  '3',  '4',  '/',  '9',  '7',
                     'u',  '2',  '1',  '4',  '1',  '/',  '0',  '0',
                     '0',  '2',  '/',  '3',  '9',  '3',  '2',  '4',
                     '2',  '3',  '5',  '3',  '2',  '/',  '5',  '6',
                     '2',  '3',  '4',  '0',  '2',  '3',  '/',  '-',
                     '-',  '-',  '-',  '/',  '=',  '1',  '2',  '3',
                     '4',  '=',  '/',  '0',  '9',  '8',  '1',  '4',
                     '1',  '-',  '9',  '5',  '6',  '4',  '6',  '4',
                     '3',  '/',  '2',  '1',  '9',  '7',  '0',  '-',
                     '-',  '-',  '-',  '-',  '/',  '8',  '2',  '3',
                     '6',  '4',  '9',  '2',  '3',  '4',  '7',  '2',
                     'w',  'e',  'r',  'e',  'r',  'e',  'w',  'r',
                     '0',  '-',  '9',  '2',  '1',  '-',  '3',  '9',
                     '1',  '2',  '3',  '-',  '3',  '4',  '/',  0x0d,
                     0x01, '/',  '/',  '4',  '9',  '2',  '4',  '0',
                     '3',  '-',  '-',  '0',  '9',  '8',  '/',  0xc1,
                     '*',  0xff, 'd',  'a',  't',  'a'
  };
  int result;

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  pdu->type = COAP_MESSAGE_ACK;
  pdu->code = COAP_RESPONSE_CODE(204);
  pdu->mid = 0x1234;

  assert(pdu->used_size == 0);

  result = coap_add_token(pdu, 2, (const uint8_t *)"\0\0");

  assert(result > 0);
  result = coap_add_option(pdu, COAP_OPTION_LOCATION_PATH, 255,
                           (const uint8_t *)"coap://example.com/12345/%3Fxyz/3048234234/23402348234/239084234-23/%AB%30%af/+123/hfksdh/23480-234-98235/1204/243546345345243/0198sdn3-a-3///aff0934/97u2141/0002/3932423532/56234023/----/=1234=/098141-9564643/21970-----/82364923472wererewr0-921-39123-34/");

  assert(result == 257);
  assert(pdu->max_opt == 8);
  assert(pdu->used_size == 259);
  assert(!pdu->data);

  result = coap_add_option(pdu, COAP_OPTION_LOCATION_QUERY,
                           1, (const uint8_t *)"*");

  assert(result == 2);
  assert(pdu->used_size == 261);
  assert(!pdu->data);

  result = coap_insert_option(pdu, COAP_OPTION_LOCATION_PATH, 14,
                              (const uint8_t *)"//492403--098/");

  assert(result == 16);
  assert(pdu->used_size == 277);
  assert(!pdu->data);

  result = coap_add_data(pdu, 4, (const uint8_t *)"data");

  assert(result > 0);
  assert(pdu->used_size == 282);
  assert(pdu->data == pdu->token + 278);

  assert(coap_pdu_encode_header(pdu, COAP_PROTO_UDP) == 4);
  assert(memcmp(pdu->token - 4, teststr, sizeof(teststr)) == 0);

}

/* Remove an option (no data) */
static void t_encode_pdu19(void) {
  size_t n;
  uint8_t  token[] = { 't' };
  uint16_t opt_num[] = { 300,   7,  21,  25 };
  uint8_t  opt_val[] = {  54,  50,  52,  53 };
  uint8_t  data1[] = { 0x74, 0x71, 0x32, 0xd1, 0x01, 0x34, 0x41, 0x35,
                       0xe1, 0x00, 0x06, 0x36 };
  uint8_t  data2[] = { 0x74, 0x71, 0x32, 0xd1, 0x05, 0x35, 0xe1, 0x00,
                       0x06, 0x36 };
  uint8_t  data3[] = { 0x74, 0xd1, 0x0c, 0x35, 0xe1, 0x00, 0x06, 0x36 };
  uint8_t  data4[] = { 0x74, 0xd1, 0x0c, 0x35 };
  uint8_t  data5[] = { 0x74 };
  uint8_t  data6[] = { 0x74, 0xd1, 0x0c, 0x0a };
  int new_val;
  unsigned char buf[4];

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  coap_add_token(pdu, sizeof(token), token);
  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_add_option(pdu, opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]);
  }
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);

  /* Now remove an option in the middle */
  coap_remove_option(pdu, 21);
  assert(memcmp(pdu->token, data2, pdu->used_size) == 0);

  /* Now remove an option from the start */
  coap_remove_option(pdu, 7);
  assert(memcmp(pdu->token, data3, pdu->used_size) == 0);

  /* Now remove an option from the end */
  coap_remove_option(pdu, 300);
  assert(memcmp(pdu->token, data4, pdu->used_size) == 0);

  /* Now remove the final option */
  coap_remove_option(pdu, 25);
  assert(memcmp(pdu->token, data5, pdu->used_size) == 0);

  /* Now insert an option */
  new_val = 10;
  coap_update_option(pdu, 25,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data6, pdu->used_size) == 0);
}

/* Remove an option (with data) */
static void t_encode_pdu20(void) {
  size_t n;
  uint8_t  token[] = { 't' };
  uint16_t opt_num[] = { 300,   7,  21,  25 };
  uint8_t  opt_val[] = {  54,  50,  52,  53 };
  uint8_t  data[] = { 'd', 'a', 't', 'a' };
  uint8_t  data1[] = { 0x74, 0x71, 0x32, 0xd1, 0x01, 0x34, 0x41, 0x35,
                       0xe1, 0x00, 0x06, 0x36, 0xff, 0x64, 0x61, 0x74,
                       0x61 };
  uint8_t  data2[] = { 0x74, 0x71, 0x32, 0xd1, 0x05, 0x35, 0xe1, 0x00,
                       0x06, 0x36, 0xff, 0x64, 0x61, 0x74, 0x61 };
  uint8_t  data3[] = { 0x74, 0xd1, 0x0c, 0x35, 0xe1, 0x00, 0x06, 0x36,
                       0xff, 0x64, 0x61, 0x74, 0x61 };
  uint8_t  data4[] = { 0x74, 0xd1, 0x0c, 0x35, 0xff, 0x64, 0x61, 0x74,
                       0x61 };
  uint8_t  data5[] = { 0x74, 0xff, 0x64, 0x61, 0x74, 0x61 };
  uint8_t  data6[] = { 0x74, 0xd1, 0x0c, 0x0a, 0xff, 0x64, 0x61, 0x74,
                       0x61 };
  int new_val;
  unsigned char buf[4];

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  coap_add_token(pdu, sizeof(token), token);
  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_add_option(pdu, opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]);
  }
  coap_add_data(pdu, sizeof(data), data);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);

  /* Now remove an option in the middle */
  coap_remove_option(pdu, 21);
  assert(memcmp(pdu->token, data2, pdu->used_size) == 0);

  /* Now remove an option from the start */
  coap_remove_option(pdu, 7);
  assert(memcmp(pdu->token, data3, pdu->used_size) == 0);

  /* Now remove an option from the end */
  coap_remove_option(pdu, 300);
  assert(memcmp(pdu->token, data4, pdu->used_size) == 0);

  /* Now remove the final option */
  coap_remove_option(pdu, 25);
  assert(memcmp(pdu->token, data5, pdu->used_size) == 0);

  /* Now insert an option */
  new_val = 10;
  coap_update_option(pdu, 25,
                     coap_encode_var_safe(buf, sizeof(buf), new_val), buf);
  assert(memcmp(pdu->token, data6, pdu->used_size) == 0);
}

/* Update token */
static void t_encode_pdu21(void) {
  size_t n;
  uint8_t  token[] = { 't' };
  uint8_t  new_token[] = { 't', 'o', 'k', 'e', 'n' };
  uint16_t opt_num[] = { 300,  10,   7,  21,  25 };
  uint8_t  opt_val[] = {  54,  51,  50,  52,  53 };
  uint8_t  data[] = { 'd', 'a', 't', 'a' };
  uint8_t  data1[] = { 0x74, 0x71, 0x32, 0x31, 0x33, 0xb1, 0x34, 0x41,
                       0x35, 0xe1, 0x00, 0x06, 0x36, 0xff, 0x64, 0x61,
                       0x74, 0x61 };
  uint8_t  data2[] = { 0x74, 0x6f, 0x6b, 0x65, 0x6e, 0x71, 0x32, 0x31,
                       0x33, 0xb1, 0x34, 0x41, 0x35, 0xe1, 0x00, 0x06,
                       0x36, 0xff, 0x64, 0x61, 0x74, 0x61 };
  uint8_t  data3[] = { 0x71, 0x32, 0x31, 0x33, 0xb1, 0x34, 0x41, 0x35,
                       0xe1, 0x00, 0x06, 0x36, 0xff, 0x64, 0x61, 0x74,
                       0x61 };

  coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

  assert(pdu->data == NULL);

  coap_add_token(pdu, sizeof(token), token);
  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_add_option(pdu, opt_num[n],
                     sizeof(opt_val[n]), &opt_val[n]);
  }
  coap_add_data(pdu, sizeof(data), data);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);

  /* Now update token */
  coap_update_token(pdu, sizeof(new_token), new_token);
  assert(memcmp(pdu->token, data2, pdu->used_size) == 0);

  /* Now restore token */
  coap_update_token(pdu, sizeof(token), token);
  assert(memcmp(pdu->token, data1, pdu->used_size) == 0);

  /* Now set token to zero length */
  coap_update_token(pdu, 0, NULL);
  assert(memcmp(pdu->token, data3, pdu->used_size) == 0);
}

/* insert option before (large) final one */
static void t_encode_pdu22(void) {
  size_t n;
  uint8_t  token[] = { 't' };
  uint8_t  buf[4];
  uint16_t opt_num[] = { 28,  28,    28,      28 };
  uint32_t opt_val[] = { 0x1, 0x100, 0x10000, 0x1000000 };
  uint8_t data1[][8] = {
                        { 0x74, 0xd1, 0x0f, 0x01 },
                        { 0x74, 0xd2, 0x0f, 0x01, 0x00 },
                        { 0x74, 0xd3, 0x0f, 0x01, 0x00, 0x00 },
                        { 0x74, 0xd4, 0x0f, 0x01, 0x00, 0x00, 0x00 }};
  uint8_t  data2[][16] = {
                        { 0x74, 0xd3, 0x0a, 0xff, 0xff, 0xf6, 0x51, 0x01 },
                        { 0x74, 0xd3, 0x0a, 0xff, 0xff, 0xf6, 0x52, 0x01,
                          0x00 },
                        { 0x74, 0xd3, 0x0a, 0xff, 0xff, 0xf6, 0x53, 0x01,
                          0x00, 0x00 },
                        { 0x74, 0xd3, 0x0a, 0xff, 0xff, 0xf6, 0x54, 0x01,
                          0x00, 0x00, 0x00 }};

  for (n = 0; n < (sizeof(opt_num)/sizeof(opt_num[0])); n++) {
    coap_pdu_clear(pdu, pdu->max_size);        /* clear PDU */

    assert(pdu->data == NULL);

    coap_add_token(pdu, sizeof(token), token);
    coap_add_option(pdu, opt_num[n],
                    coap_encode_var_safe(buf, sizeof(buf), opt_val[n]), buf);
    assert(memcmp(pdu->token, data1[n], pdu->used_size) == 0);

    /* Now insert option */
    coap_insert_option(pdu, 23,
                       coap_encode_var_safe(buf, sizeof(buf), 0xfffff6), buf);
    assert(memcmp(pdu->token, data2[n], pdu->used_size) == 0);
  }
}


static int t_pdu_tests_create(void) {
  pdu = coap_pdu_init(0, 0, 0, COAP_DEFAULT_MTU);

  return pdu == NULL;
}

static int t_pdu_tests_remove(void) {
  coap_delete_pdu(pdu);
  return 0;
}

int t_init_pdu_tests(int argc, char **argv) {
  coap_set_log_level(LOG_INFO);

  t_pdu_tests_create();

  t_parse_pdu1();
  t_parse_pdu2();
  t_parse_pdu3();
  t_parse_pdu4();
  t_parse_pdu5();
  t_parse_pdu6();
  t_parse_pdu7();
  t_parse_pdu8();
  t_parse_pdu9();
  t_parse_pdu10();
  t_parse_pdu11();
  t_parse_pdu12();
  t_parse_pdu13();
  t_parse_pdu14();
  t_parse_pdu15();
  t_parse_pdu16();
  t_parse_pdu17();

  t_pdu_tests_remove();

  t_pdu_tests_create();
  t_encode_pdu1();
  t_encode_pdu2();
  t_encode_pdu3();
  t_encode_pdu4();
  t_encode_pdu5();
  t_encode_pdu6();
  t_encode_pdu7();
  t_encode_pdu8();
  t_encode_pdu9();
  t_encode_pdu10();
  t_encode_pdu11();
  t_encode_pdu12();
  t_encode_pdu13();
  t_encode_pdu14();
  t_encode_pdu15();
  t_encode_pdu16();
  t_encode_pdu17();
  t_encode_pdu18();
  t_encode_pdu19();
  t_encode_pdu20();
  t_encode_pdu21();
  t_encode_pdu22();
  t_pdu_tests_remove();

  return AIC_SUCCESS;
}
CONSOLE_EXPORT_CMD(coap_pdu_test, t_init_pdu_tests, coap pdu test);

#endif /* COAP_CLIENT_SUPPORT */

