/*****************************************************************************/
/*                                                                           */
/*  Copyright2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "initcall.h"
#include "platform/platform.h"
#include "secure/secure_boot.h"

#include "mbedtls/rsa.h"
#include "mbedtls/oid.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

int aic_rsa_verify(uint8_t *hash, uint8_t *sign, struct rsa_pubkey *pub_key)
{
    mbedtls_rsa_context rsa;
    mbedtls_mpi K;
    int ret;

    mbedtls_mpi_init( &K );
    mbedtls_rsa_init( &rsa );

    MBEDTLS_MPI_CHK( mbedtls_mpi_read_binary_le( &K, pub_key->n, pub_key->n_len) );
    MBEDTLS_MPI_CHK( mbedtls_rsa_import( &rsa, &K, NULL, NULL, NULL, NULL ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_read_binary_le( &K, pub_key->e, pub_key->e_len) );
    MBEDTLS_MPI_CHK( mbedtls_rsa_import( &rsa, NULL, NULL, NULL, NULL, &K ) );
    MBEDTLS_MPI_CHK( mbedtls_rsa_complete( &rsa ) );

    ret = mbedtls_rsa_set_padding( &rsa, MBEDTLS_RSA_PKCS_V15,
                                   MBEDTLS_MD_SHA256 );
    if (ret){
        log_print("mbedtls_rsa_set_padding fail ret = %d\n", ret);
        goto cleanup;
    }

    ret = mbedtls_rsa_pkcs1_verify(&rsa, MBEDTLS_MD_SHA256, 32, hash, sign);
    if (ret){
        log_print("aic_rsa_verify fail ret = %d\n", ret);
        goto cleanup;
    }

cleanup:
    mbedtls_mpi_free( &K );
    mbedtls_rsa_free( &rsa );
    return ret;
}


int aic_rsa_verify_init(void)
{
    aic_secboot_rsa_func_register(aic_rsa_verify);

    return 0;
}

driver_init(aic_rsa_verify_init);

