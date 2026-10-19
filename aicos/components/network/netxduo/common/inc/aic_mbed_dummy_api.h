/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_MBED_DUMMY_API_H__
#define __AIC_MBED_DUMMY_API_H__

#ifndef CONFIG_NET_SECURE
#ifdef CONFIG_CRYPTO_MBEDTLS_2
#include "mbedtls/net.h"
#else
#include "mbedtls/net_sockets.h"
#endif
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/x509_crt.h>
#endif

#ifndef CONFIG_CRYPTO_MBEDTLS
int mbedtls_ssl_close_notify( mbedtls_ssl_context *ssl );
void aic_ssl_set_send_waitoption(int socketfd, __uint32_t waitoption);
int mbedtls_ssl_send(mbedtls_ssl_context *ssl, NX_PACKET *packet_ptr);
int mbedtls_ssl_recv(mbedtls_ssl_context *ssl, NX_PACKET **packet_ptr_ptr);
void mbedtls_ssl_free( mbedtls_ssl_context *ssl );
void mbedtls_ssl_config_free( mbedtls_ssl_config *conf );
void mbedtls_ctr_drbg_free( mbedtls_ctr_drbg_context *ctx );
void mbedtls_entropy_free( mbedtls_entropy_context *ctx );
void aic_ssl_set_recv_waitoption(int socketfd, __uint32_t waitoption);
int mbedtls_ssl_handshake( mbedtls_ssl_context *ssl );
void mbedtls_net_free(mbedtls_net_context *ctx);
void mbedtls_ssl_reset_socket(mbedtls_net_context *ctx);

#endif

#endif

