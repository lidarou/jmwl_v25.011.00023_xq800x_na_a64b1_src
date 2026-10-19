
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b8679
#define LOG_MODULE    AMOD_SYS
#include <stdint.h>
#include "mbedtls/pkcs7.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
#ifdef MBEDTLS_PKCS7_C
    mbedtls_pkcs7 pkcs7;

    mbedtls_pkcs7_init( &pkcs7 );

    mbedtls_pkcs7_parse_der( &pkcs7, Data, Size );

    mbedtls_pkcs7_free( &pkcs7 );
#else
    (void) Data;
    (void) Size;
#endif

    return 0;
}
