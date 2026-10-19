/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __SECURE_BOOT_H__
#define __SECURE_BOOT_H__

#include "io.h"
#include "secure/sec_boot_cfg.h"

#define RSA_KEY_BYTE_LEN_MAX (RSA_KEY_BITS_LEN_MAX >> 3)

struct aic_certificate {
        uint32_t version;
        uint32_t type;
        uint8_t hash[HASH_BYTE_LEN];
        uint8_t pubkey[0];
};

struct aic_key_certificate {
        uint32_t type;
        uint8_t hash[HASH_BYTE_LEN];
};

struct rsa_pubkey {
    uint32_t  n_len;
    uint32_t  e_len;
    uint8_t e[4];       /* Public exponent */
    uint8_t n[0];       /* Modulus */
};

typedef int (*HASH_FUNC)(const uint8_t *buf, uint32_t length, uint8_t *out);
typedef int (*RSA_VERIFY)(uint8_t *hash, uint8_t *sign,
                            struct rsa_pubkey *pub_key);

void aic_secboot_hash_func_register(HASH_FUNC hash);
void aic_secboot_rsa_func_register(RSA_VERIFY rsa_verify);
int aic_secboot_check_img_sec_attr(char * img_name);


#endif /* __SECURE_BOOT_H__ */
