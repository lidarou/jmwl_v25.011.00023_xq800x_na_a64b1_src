/*****************************************************************************/
/*                                                                           */
/*      Copyright 2021 by AIC Corporation. All rights reserved.              */
/*                                                                           */
/*****************************************************************************/

#ifndef __SECURE_BOOT_H__
#define __SECURE_BOOT_H__

#include "aic_io.h"
#include <secure_cfg.h>

#define RSA_KEY_BYTE_LEN_MAX (3072 >> 3)

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
    uint32_t n_len;
    uint32_t e_len;
    uint8_t e[4];       /* Public exponent */
    uint8_t n[0];       /* Modulus */
};

struct boot_parameters {
    uint32_t boot_mode;
    uint32_t dl_type;
    uint8_t pubkey_hash[HASH_BYTE_LEN];
    uint32_t SW_version;
    uint32_t version_id_check;
};

struct image_header {
    uint32_t magic_number;
    uint32_t pubkey_size;
    uint32_t image_offset;
    uint32_t image_size;
    uint32_t image_certificates_offset;
    uint32_t image_certificates_size;
    uint32_t image_sign_offset;
    uint32_t image_sign_size;
    uint32_t key_certificates_offset;
    uint32_t key_certificates_size;
    uint32_t key_sign_offset;
    uint32_t key_sign_size;
};

typedef int (*HASH_FUNC)(const uint8_t *buf,
    uint32_t length, uint8_t *out);
typedef int (*RSA_VERIFY)(uint8_t *hash,
    uint8_t *sign, struct rsa_pubkey *pub_key);

int sha256_hash_init(void);
int aic_rsa_verify_init(void);
void aic_secboot_hash_func_register(HASH_FUNC hash);
void aic_secboot_rsa_func_register(RSA_VERIFY rsa_verify);
int aic_secboot_img_authorization(
                            struct image_header *header,
                            struct boot_parameters *bp,
                            uint8_t *next_img);
/**
* aic_secboot_get_verified_hash() - get verified pubkey hash, this function
*    do all signed image authorization use the returned hash.
* @hash_ptr: pointer to hash memory.
* @hash_len: pointer of hash size.
* Return: 0 for success, other for error.
*/
int aic_secboot_get_verified_hash(uint8_t *hash_ptr, uint32_t *hash_len);

/**
* aic_secboot_get_pubkey() - get pubkey from image
* @pp_pubkey: pointer to pubkey memory pointer.
* @pubkey_len: pointer of pubkey size.
* Return: 0 for success, other for error.
*/
int aic_secboot_get_pubkey(uint8_t **pp_pubkey, uint32_t *pubkey_len);
/**
* aic_secboot_get_image_pubkey_hash() - get pubkey from specified image
* @image_name: image name.
* @hash: pointer of pubkey hash.
* Return: 0 for success, other for error.
*/
int aic_secboot_get_image_pubkey_hash(const char *image_name, uint8_t *hash);
/**
* aic_secboot_write_pubkey_hash() - write pubkey hash to efuse
* @pubkey_hash: pointer of pubkey hash.
* Return: 0 for success, other for error.
*/
int aic_secboot_write_pubkey_hash(uint8_t *pubkey_hash);
/**
* aic_secboot_enable() - write secureboot enable bit to efuse
* Return: 0 for success, other for error.
*/
int aic_secboot_enable(void);
/**
* aic_rsa_verify() - do rsa verify
* @hash: The buffer holding the message digest or raw data.
* @sign: The buffer holding the signature..
* @pub_key: pointer to pubkey.
* Return: 0 for success, other for error.
*/
int aic_rsa_verify(uint8_t *hash, uint8_t *sign, struct rsa_pubkey *pub_key);
#endif /* __SECURE_BOOT_H__ */
