#/****************************************************************************/
#/*                                                                          */
#/*     Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved. */
#/*                                                                          */
#/****************************************************************************/

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

target_include_directories(boot_driver
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_sources(boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/secure_boot.c
)

target_sources_ifdef(CONFIG_CRYPTO_SW_ACCELERATOR boot_driver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/rsa/rsa_verify.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sha256/sha256.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/aes.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/aesni.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/base64.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/rsa.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/entropy.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/entropy_poll.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/bignum.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/sha1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/sha256.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/sha512.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/ripemd160.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/platform_util.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/oid.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/timing.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/rsa_alt_helpers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/library/md.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/library/md5.c
)