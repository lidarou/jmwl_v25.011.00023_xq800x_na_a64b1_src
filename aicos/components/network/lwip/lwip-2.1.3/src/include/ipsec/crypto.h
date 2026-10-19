#ifndef __IPSEC_CRYPTO_H__
#define __IPSEC_CRYPTO_H__

#include "lwip/opt.h"

#if defined(EIPS) && EIPS == 1
#include "ipsec/sa.h"

u8* get_default_aes_key(void);
u8* get_default_aes_iv(void);
u8* get_default_hmac_key(void);
u32 ipsec_encrypt_len(u32 size);
u32 ipsec_decrypt_len(u32 size, u8* output);
u32 ipsec_hash_len(void);

#if 0
int aes_128_gcm_encrypt(byte* key, byte* iv, byte* input, word32 size, byte* output, word32 output_size,
                   byte* auth_in, word32 auth_in_size, byte* auth_tag, word32 auth_tag_size);
int aes_128_gcm_decrypt(byte* key, byte* iv, byte* input, word32 size, byte* output, word32 output_size,
                   byte* auth_in, word32 auth_in_size, byte* auth_tag, word32 auth_tag_size);
int hmac_sha256(byte* key, word32 key_size, byte* input, word32 input_size, byte* output, word32 output_size);
#endif

int aes_128_cbc_encrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                        u32 size, u8* output, u32 output_size);
int aes_128_cbc_decrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                        u32 size, u8* output, u32 output_size);
int des_ede3_cbc_encrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                         u32 size, u8* output, u32 output_size);
int des_ede3_cbc_decrypt(sad_entry *sa, u8* key, u8* iv, u8* input,
                         u32 size, u8* output, u32 output_size);
int hmac_sha1(u8* auth_key, int key_size, u8* input, u32 input_size,
              u8* output, u32 output_size);
int hmac_md5(u8* auth_key, int key_size, u8* input, u32 input_size,
             u8* output, u32 output_size);

#endif /* defined(EIPS) && EIPS == 1 */

#endif /* __IPSEC_CRYPTO_H__ */
