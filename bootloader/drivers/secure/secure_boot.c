/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "initcall.h"
#include "platform/platform.h"
#include "secure/secure_boot.h"

#define SEC_IMG_NUM    4
char *secure_img[SEC_IMG_NUM] = {"mbl", "cp", "ap", "ap_extend"};

struct aic_secboot {
    HASH_FUNC hash_calc;
    RSA_VERIFY rsa_verify;
};

struct aic_secboot aic_secboot;

static int aic_secboot_rsa_verify(struct secure_boot *sec_boot,
                void *crt, uint8_t *sign, uint32_t type)
{
    struct aic_key_certificate *key_crt = (struct aic_key_certificate *)crt;
    struct aic_certificate *img_crt = (struct aic_certificate *)crt;
    struct rsa_pubkey *pub_key;
    uint8_t *hash;
    int ret;

    hash = type ? img_crt->hash : key_crt->hash;
    if(!aic_secboot.rsa_verify) {
        log_print("rsa_verify func is unregistered.\n");
        return -1;
    }

    pub_key = (struct rsa_pubkey *)sec_boot->pubkey;
    ret = aic_secboot.rsa_verify(hash, sign, pub_key);
    if (ret) {
        log_print("rsa verify fail.\n");
        return -1;
    }

    return 0;
}

static int aic_secboot_next_key_authorization(
                            struct secure_boot *sec_boot,
                            struct image_header *header,
                            struct boot_parameters *bp,
                            uint8_t *next_img)
{
    struct aic_key_certificate *key_certificate;
    uint8_t *key_signed;
    int ret;

    key_certificate = (struct aic_key_certificate *)(next_img +
            header->key_certificates_offset);
    key_signed = (uint8_t *)(next_img + header->key_sign_offset);
    ret = aic_secboot_rsa_verify(sec_boot, (void *)key_certificate, key_signed,
                            key_certificate->type);
    if (ret) {
        log_print("next key rsa verify fail ret = %d.\n", ret);
        return ret;
    }

    log_print("next key authorization pass.\n");
    memcpy(bp->pubkey_hash, key_certificate->hash, HASH_BYTE_LEN);

    return 0;
}

int aic_secboot_pubkey_authorization(struct aic_certificate *crt,
                struct boot_parameters *bp, int pk_size)
{
    uint8_t pubkey_hash[HASH_BYTE_LEN];
    int ret;

    ret = aic_secboot.hash_calc(crt->pubkey, pk_size, pubkey_hash);
    if (ret) {
        log_print("hash calculate fail.\n");
        return -1;
    }

    if (memcmp(bp->pubkey_hash, pubkey_hash, HASH_BYTE_LEN)) {
        log_print("the hash of calculate pubkey is not match.\n");
        return -1;
    }

    return 0;
}

static int aic_secboot_img_authorization(
                            struct secure_boot *sec_boot,
                            struct image_header *header,
                            struct boot_parameters *bp,
                            uint8_t *next_img)
{
    struct aic_certificate *img_certificate;
    uint8_t image_hash[HASH_BYTE_LEN];
    uint8_t *img_signed;
    uint8_t *payload;
    int ret;

    img_certificate = (struct aic_certificate *)(next_img +
            header->image_certificates_offset);
    img_signed = (uint8_t *)(next_img + header->image_sign_offset);
    payload = (uint8_t *)(next_img + IMAGE_HEADER_SIZE );

    if (bp->SW_version != img_certificate->version) {
        log_print("aic secboot rollback check fail.\n");
        return -1;
    }

    if (aic_secboot_pubkey_authorization(img_certificate, bp,
                    header->pubkey_size)) {
        log_print("pubkey authorization is fail.\n");
        return -1;
    }

    sec_boot->pubkey = (void *)img_certificate->pubkey;
    log_print("pubkey authorization pass.\n");

    ret = aic_secboot.hash_calc(payload,
        header->image_size + sizeof(img_certificate->version), image_hash);
    if (ret) {
        log_print("image hash calculate fail.\n");
        return -1;
    }

    if (memcmp(img_certificate->hash, image_hash, HASH_BYTE_LEN)) {
        log_print("the hash of calculate image is not match.\n");
        return -1;
    }

    ret = aic_secboot_rsa_verify(sec_boot, img_certificate, img_signed,
                            img_certificate->type);
    if (ret) {
        log_print("rsa verify ret = %d.\n", ret);
        return ret;
    }

    log_print("img authorization pass.\n");

    return 0;
}

static int aic_secboot_authorization(struct platform_core *p)
{
    struct secure_boot *sec_boot = &p->secure_boot;
    struct boot_parameters *bp = p->bp;
    int ret;

    sec_boot->header = (struct image_header *)p->next_img;
    if (sec_boot->header->magic_number != AIC_MAGIC_NUMBER) {
        log_print("the magic_number(0x%x) is error, secboot exit.\n",
            sec_boot->header->magic_number);
        return -1;
    }

    if(!aic_secboot.rsa_verify) {
        log_print("rsa_verify func is unregistered.\n");
        return -1;
    }

    if(!aic_secboot.hash_calc) {
        log_print("hash_calc func is unregistered.\n");
        return -1;
    }

    ret = aic_secboot_img_authorization(sec_boot, sec_boot->header, bp,
                    p->next_img);
    if (ret)
        return ret;

    if (sec_boot->header->key_sign_size) {
        ret = aic_secboot_next_key_authorization(sec_boot, sec_boot->header, bp,
                        p->next_img);
        if (ret)
            return ret;
    }

    p->next_img = p->next_img + IMAGE_HEADER_SIZE;

    return 0;
}

int aic_secboot_check_img_sec_attr(char * img_name)
{
    int i;

    for (i = 0; i< SEC_IMG_NUM; i++) {
        if (!strcmp(secure_img[i], img_name))
            return 1;
    }

    return 0;
}

void aic_secboot_rsa_func_register(RSA_VERIFY rsa_verify)
{
    aic_secboot.rsa_verify = rsa_verify;
}

void aic_secboot_hash_func_register(HASH_FUNC hash)
{
    aic_secboot.hash_calc = hash;
}

int aic_secboot_init(void)
{
    platform.secure_boot.img_attr_check = aic_secboot_check_img_sec_attr;
    platform_secboot_init(aic_secboot_authorization);
    return 0;
}

driver_init(aic_secboot_init);
