/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include "initcall.h"
#include "inttypes.h"
#include <string.h>
#include "mbedtls/sha256.h"
#include "secure/secure_boot.h"

int sha256_hash(const uint8_t *buf, uint32_t length, uint8_t *out)
{
    return mbedtls_sha256(buf, length, out, 0);
}

int sha256_init(void)
{
    aic_secboot_hash_func_register(sha256_hash);

    return 0;
}

driver_init(sha256_init);
