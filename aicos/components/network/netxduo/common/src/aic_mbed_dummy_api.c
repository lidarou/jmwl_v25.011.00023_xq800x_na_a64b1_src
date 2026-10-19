/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05ecf1
#define LOG_MODULE    AMOD_NET

#include <aic_mbed_dummy_api.h>

#ifndef CONFIG_CRYPTO_MBEDTLS
volatile int mbedtls_ssl_close_notify( mbedtls_ssl_context *ssl )
{
    return 0;
}

volatile void aic_ssl_set_send_waitoption(int socketfd, __uint32_t waitoption)
{
    return;
}

volatile int mbedtls_ssl_send(mbedtls_ssl_context *ssl, NX_PACKET *packet_ptr)
{
    return 0;
}

volatile int mbedtls_ssl_recv(mbedtls_ssl_context *ssl, NX_PACKET **packet_ptr_ptr)
{
    return 0;
}

volatile void mbedtls_ssl_free( mbedtls_ssl_context *ssl)
{
    return;
}

volatile void mbedtls_ssl_config_free( mbedtls_ssl_config *conf)
{
    return;
}

volatile void mbedtls_ctr_drbg_free( mbedtls_ctr_drbg_context *ctx)
{
    return;
}

volatile void mbedtls_entropy_free( mbedtls_entropy_context *ctx)
{
    return;
}

volatile void aic_ssl_set_recv_waitoption(int socketfd, __uint32_t waitoption)
{
    return;
}

volatile int mbedtls_ssl_handshake( mbedtls_ssl_context *ssl )
{
    return 0;
}

volatile void mbedtls_net_free(mbedtls_net_context *ctx)
{
    return;
}

volatile void mbedtls_ssl_reset_socket(mbedtls_net_context *ctx)
{
    return;
}


#endif

