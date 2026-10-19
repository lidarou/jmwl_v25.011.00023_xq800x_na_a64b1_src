# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_CRYPTO_MBEDTLS_3 mbedtls-3.3.0)
add_subdirectory_ifdef(CONFIG_CRYPTO_MBEDTLS_2 mbedtls-2.1.8)
