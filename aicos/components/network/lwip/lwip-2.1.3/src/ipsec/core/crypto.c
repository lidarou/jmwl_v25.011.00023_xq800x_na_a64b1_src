
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07a77c
#define LOG_MODULE    AMOD_NET
#include "lwip/opt.h"

#if defined(EIPS) && EIPS == 1
#include "psa/crypto.h"
#include "mbedtls/cipher.h"
#include "mbedtls/aes.h"
#include "mbedtls/des.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/md.h"

#include "ipsec/crypto.h"
#include "lwip/mem.h"

#define AES_BLOCK_SIZE 8
#define SHA256_DIGEST_SIZE 64


static u8 default_aes_key[16] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
static u8 default_aes_iv[16] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
};

static u8 default_hmac_key[24] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#if 0
int aes_128_gcm_encrypt(byte* key, byte* iv, byte* input, word32 size, byte* output, word32 output_size,
                               byte* auth_in, word32 auth_in_size, byte* auth_tag, word32 auth_tag_size) {
  Aes aes;
  int ret;

  ret = wc_AesInit(&aes, NULL, INVALID_DEVID);
  if (ret != 0) {
    return ret;
  }

  ret = wc_AesGcmSetKey(&aes, key, AES_BLOCK_SIZE);
  if (ret != 0) {
    return ret;
  }

  ret = wc_AesGcmEncrypt(&aes, output, input, size, iv, AES_BLOCK_SIZE, auth_tag, auth_tag_size, auth_in, auth_in_size);
  if (ret != 0) {
    return ret;
  }

  return 0;
}

int aes_128_gcm_decrypt(byte* key, byte* iv, byte* input, word32 size, byte* output, word32 output_size,
                               byte* auth_in, word32 auth_in_size, byte* auth_tag, word32 auth_tag_size) {
  Aes aes;
  int ret;

  ret = wc_AesInit(&aes, NULL, INVALID_DEVID);
  if (ret != 0) {
    return ret;
  }

  ret = wc_AesGcmSetKey(&aes, key, AES_BLOCK_SIZE);
  if (ret != 0) {
    return ret;
  }

  ret = wc_AesGcmDecrypt(&aes, output, input, size, iv, AES_BLOCK_SIZE, auth_tag, auth_tag_size, auth_in, auth_in_size);
  if (ret != 0) {
    return ret;
  }

  return 0;
}
#endif

int aes_128_cbc_encrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                        u32 size, u8* output, u32 output_size)
{
    mbedtls_aes_context aes;
    int ret;

    mbedtls_aes_init(&aes);

    ret = mbedtls_aes_setkey_enc(&aes, sa->enckey, sa->enckey_size << 3);
    if (ret != 0) {
      return ret;
    }

    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_ENCRYPT, size, iv, input, output);
    if (ret != 0) {
      return ret;
    }

    return 0;
}

int aes_128_cbc_decrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                        u32 size, u8* output, u32 output_size)
{
    mbedtls_aes_context aes;
    int ret;

    mbedtls_aes_init(&aes);

    ret = mbedtls_aes_setkey_dec(&aes, sa->enckey, sa->enckey_size << 3);
    if (ret != 0) {
      return ret;
    }

    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_DECRYPT, size, iv, input, output);
    if (ret != 0) {
      return ret;
    }

    return 0;
}

int des_ede3_cbc_encrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                         u32 size, u8* output, u32 output_size)
{
    mbedtls_des3_context des3;
    int ret;

    mbedtls_des3_init(&des3);

    ret = mbedtls_des3_set3key_enc(&des3, sa->enckey);
    if (ret != 0) {
        return ret;
    }

    ret = mbedtls_des3_crypt_cbc(&des3, MBEDTLS_ENCRYPT, size, iv, input, output);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int des_ede3_cbc_decrypt(sad_entry *sa, u8 *key, u8* iv, u8* input,
                         u32 size, u8* output, u32 output_size)
{
    mbedtls_des3_context des3;
    int ret;

    mbedtls_des3_init(&des3);

    ret = mbedtls_des3_set3key_dec(&des3, sa->enckey);
    if (ret != 0) {
        return ret;
    }

    ret = mbedtls_des3_crypt_cbc(&des3, MBEDTLS_DECRYPT, size, iv, input, output);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

u8* get_default_aes_key(void) {
    return default_aes_key;
}

u8* get_default_aes_iv(void) {
    return default_aes_iv;
}

u8* get_default_hmac_key(void) {
    return default_hmac_key;
}

#if 0
int hmac_sha256(u8* key, u32 key_size, u8* input, u32 input_size, u8* output, u32 output_size) {
    Hmac hmac;

    wc_HmacInit(&hmac, NULL, INVALID_DEVID);
    wc_HmacSetKey(&hmac, WC_SHA256, key, key_size);
    wc_HmacUpdate(&hmac, input, input_size);
    wc_HmacFinal(&hmac, output);
}
#endif

int hmac_sha1(u8* auth_key, int key_size, u8* input, u32 input_size,
              u8* output, u32 output_size)
{
    const mbedtls_md_type_t alg = MBEDTLS_MD_SHA1;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(alg);
    u8 out[IPSEC_AUTH_MAX_ICV] = {'\0'};
    int ret = 0;

    mbedtls_md_init(&ctx);
    ret = mbedtls_md_setup(&ctx, info, 1);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_starts(&ctx, auth_key, key_size);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_update(&ctx, input, input_size);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_finish( &ctx, out);
    if (ret != 0)
        goto exit;

    MEMCPY(output, out, output_size);

exit:
    mbedtls_md_free( &ctx );

#if 0
    mbedtls_sha1_init(&sha1);
    mbedtls_sha1_starts(&sha1);
    mbedtls_sha1_update(&sha1, input, input_size);
    mbedtls_sha1_finish(&sha1, output);
    mbedtls_sha1_free(&sha1);
#endif
    return ret;
}

int hmac_md5(u8* auth_key, int key_size, u8* input, u32 input_size,
             u8* output, u32 output_size)
{
    const mbedtls_md_type_t alg = MBEDTLS_MD_MD5;
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(alg);
    u8 out[IPSEC_AUTH_MAX_ICV] = {'\0'};
    int ret = 0;

    mbedtls_md_init(&ctx);
    ret = mbedtls_md_setup(&ctx, info, 1);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_starts(&ctx, auth_key, key_size);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_update(&ctx, input, input_size);
    if (ret != 0)
        goto exit;

    ret = mbedtls_md_hmac_finish( &ctx, out);
    if (ret != 0)
        goto exit;

    MEMCPY(output, out, output_size);;
exit:
    mbedtls_md_free( &ctx );

#if 0
    mbedtls_md5_init(&md5);
    mbedtls_md5_starts(&md5);
    mbedtls_md5_update(&md5, input, input_size);
    mbedtls_md5_finish(&md5, output);
    mbedtls_md5_free(&md5);
#endif
    return ret;
}

u32 ipsec_encrypt_len(u32 size) {
    int padCounter = 0;
    u32 tmp_len;

    tmp_len = size;

    /* pads the length until it evenly matches a block / increaess pad number*/
    while (tmp_len % AES_BLOCK_SIZE != 0 || padCounter == 0) {
        tmp_len++;
        padCounter++;
    }

    return tmp_len;
}

u32 ipsec_decrypt_len(u32 size, u8* output) {
    u32 length;
    u8 padding_len = output[size-1];

    length = size - padding_len;
    return length;
}

u32 ipsec_hash_len(void) {
    return SHA256_DIGEST_SIZE;
}

#endif /* defined(EIPS) && EIPS == 1 */
