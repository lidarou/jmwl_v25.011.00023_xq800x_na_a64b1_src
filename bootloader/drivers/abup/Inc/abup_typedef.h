/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_typedef.h
* Description:
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef _ABUP_TYPEDEF_H_
#define _ABUP_TYPEDEF_H_

#include "abup_config.h"

#ifndef abup_extern
#ifdef __cplusplus
#define abup_extern extern "C"
#else
#define abup_extern extern
#endif
#endif//abup_extern

#define ABUP_DATA_SEQ_MAX_LEN (1<<(ABUP_DEFAULT_SEGMENT_SIZE_INDEX + 4))

#if defined(ABUP_HTTP_5_0_MAX_LEN)
#define ABUP_HTTP_HEAD_LEN                   1536
#else
#define ABUP_HTTP_HEAD_LEN                   (ABUP_MID_MAXLEN + 1 + 768)
#endif
#define ABUP_HTTP_MAX_LEN                   (ABUP_DATA_SEQ_MAX_LEN + ABUP_HTTP_HEAD_LEN)
#if defined(ABUP_HTTP_5_0_MAX_LEN)&&(ABUP_HTTP_MAX_LEN < ABUP_HTTP_5_0_MAX_LEN)
#undef ABUP_HTTP_MAX_LEN
#define ABUP_HTTP_MAX_LEN                   (ABUP_HTTP_5_0_MAX_LEN)
#endif
#define ABUP_COAP_MAX                   (ABUP_DATA_SEQ_MAX_LEN + 32)
#define ABUP_COAP_MIN                   256
#if (ABUP_COAP_MAX < ABUP_COAP_MIN)
#define ABUP_COAP_MAX_LEN                   ABUP_COAP_MIN
#else
#define ABUP_COAP_MAX_LEN                   ABUP_COAP_MAX
#endif

#if defined(__GNUC__)&&!defined(WIN32)
  #ifndef abup_weak
    #define abup_weak   __attribute__((weak))
  #endif /* abup_weak */
#define ABUP_ASM            __asm                                      /*!< asm keyword for GNU Compiler          */
#define ABUP_INLINE         inline                                     /*!< inline keyword for GNU Compiler       */
#define ABUP_STATIC_INLINE  static inline
#define ABUP_ALIGN(x) __attribute__ ((aligned (x)))
#elif defined(__ICCARM__)||defined(__ICCRX__)||defined(__ICCRL78__)||defined(__ICCSTM8__)
  #ifndef abup_weak
    #define abup_weak   __weak
  #endif /* abup_weak */
#define ABUP_ASM            __asm                                      /*!< asm keyword for IAR Compiler          */
#define ABUP_INLINE         inline                                     /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define ABUP_STATIC_INLINE  static inline
#if defined(__ICCRL78__)
#define ABUP_ALIGN(x)
#else
#define ABUP_ALIGN(x) __attribute__((aligned(x)))
#endif
#elif defined(__SDCC)
  #ifndef abup_weak
    #define abup_weak
  #endif /* abup_weak */
#define ABUP_ASM            __asm__                                      /*!< asm keyword for IAR Compiler          */
#define ABUP_INLINE         inline                                     /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define ABUP_STATIC_INLINE  static inline
#define ABUP_ALIGN(x)
#elif defined(__CC_ARM)
  #ifndef abup_weak
    #define abup_weak   __weak
  #endif /* abup_weak */
#define ABUP_ASM            __asm                                      /*!< asm keyword for ARM Compiler          */
#define ABUP_INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */
#define ABUP_STATIC_INLINE  static __inline
#if defined(KEIL4)
#define ABUP_ALIGN(x) __attribute__ ((aligned (1)))
#else
#define ABUP_ALIGN(x) __align(x)
#endif
#elif defined(__CCRL__)||defined(WIN32)
  #ifndef abup_weak
    #define abup_weak   
  #endif /* abup_weak */
#define ABUP_ASM            __asm                                      /*!< asm keyword for IAR Compiler          */
#define ABUP_INLINE                                      /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
#define ABUP_STATIC_INLINE
#define ABUP_ALIGN(x)
#elif defined(ABUP_C51)
  #ifndef abup_weak
    #define abup_weak   
  #endif /* abup_weak */
#define ABUP_STATIC_INLINE
#define ABUP_ALIGN(x)
#else
#error "Alignment not supported for this compiler."
#endif

#if defined(_ABUP_INLINE_)
#define __ABUP_INLINE__         ABUP_INLINE
#define __ABUP_STATIC_INLINE__  ABUP_STATIC_INLINE
#else
#define __ABUP_INLINE__
#define __ABUP_STATIC_INLINE__ static
#endif
#if defined(ABUP_C51)
#define ABUP_UNUSED(X) X = X     /* To avoid gcc/g++ warnings */
#else
#define ABUP_UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif

typedef enum {
    ABUP_FILETYPE_APP,
    ABUP_FILETYPE_PATCH,
    ABUP_FILETYPE_BACKUP,
    ABUP_FILETYPE_PATCH_INFO,
    ABUP_FILETYPE_APP_BACKUP,
    ABUP_FILETYPE_BL,
    ABUP_FILETYPE_BL_BACKUP,
    ABUP_FILETYPE_END
} ABUP_PEER_FILETYPE;

#define ABUPSW32(x) ((((abup_uint32)(x)&(abup_uint32)0x000000ff) << 24 )\
										|(((abup_uint32)(x)&(abup_uint32)0x0000ff00 ) << 8 )\
										|(((abup_uint32)(x)&(abup_uint32)0x00ff0000 ) >> 8 )\
										|(((abup_uint32)(x)&(abup_uint32)0xff000000 ) >> 24 ))


#define ABUP_UPDATE_SUCCESS              1
#define ABUP_UPDATE_AUTH_FAIL              98
#define ABUP_UPDATE_FAIL              99
#define ABUP_DOWNLOAD_SUCCESS              100
#define ABUP_UPDATE_RESTORE              101
#define ABUP_UPDATE_RESTORE_FAIL              97
#define ABUP_UPDATE_RESTORE_SUCCESS              8
#define ABUP_NO_UPDATE_RESULT              1000

#ifndef ABUP_MID_MAXLEN
#define ABUP_MID_MAXLEN		31
#endif
#define ABUP_CODE_MAXLEN		32
#define ABUP_DEVICEID_MAX_LEN 31
#define ABUP_DEVICESECRET_MAX_LEN 63
#define ABUP_CRC_MAXLEN     4
#define ABUP_VERSION_MAXLEN     4
#define ABUP_SN_DATETIME_MAXLEN    22
#if defined(ABUP_WIFI_SSID_PWD)
#define ABUP_WIFI_SSID_MAXLEN		32
#define ABUP_WIFI_PWD_MAXLEN		32
#endif

typedef char                    abup_char;

typedef unsigned short          abup_wchar;

#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ abup_uint8;
#else
typedef unsigned char           abup_uint8;
#endif

#ifdef __INT8_TYPE__
typedef __INT8_TYPE__ abup_int8;
#else
typedef signed char             abup_int8;
#endif

#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ abup_uint16;
#else
typedef unsigned short int      abup_uint16;
#endif

#ifdef __INT16_TYPE__
typedef __INT16_TYPE__ abup_int16;
#else
typedef signed short int        abup_int16;
#endif

typedef unsigned int            abup_uint;

typedef signed int              abup_int;

typedef unsigned long            abup_ulong;

typedef signed long              abup_long;

#ifdef ABUP_C51
typedef unsigned long        abup_ull;

typedef signed long          abup_ll;

typedef unsigned long            abup_uint32;

typedef signed long              abup_int32;

typedef unsigned long   abup_uint64;

typedef signed long     abup_int64;
#else
typedef unsigned long long        abup_ull;

typedef signed long long          abup_ll;

#ifdef ABUP_8BIT
typedef unsigned long   abup_uint32;

typedef signed long     abup_int32;

typedef unsigned long long   abup_uint64;

typedef signed long long     abup_int64;
#else
#ifdef __UINT32_TYPE__
//typedef __UINT32_TYPE__            abup_uint32;
typedef unsigned int            abup_uint32;
#else
typedef unsigned int            abup_uint32;
#endif

#ifdef __INT32_TYPE__
typedef __INT32_TYPE__ abup_int32;
#else
typedef signed int              abup_int32;
#endif

#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ abup_uint64;
#else
typedef unsigned long long   abup_uint64;
#endif

#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ abup_int64;
#else
typedef signed long long     abup_int64;
#endif
#endif
#endif
#if defined(__SIZEOF_POINTER__)&&(__SIZEOF_POINTER__ == 8)
#define ABUP_ISALIGN(x, n)                  ( ( (abup_uint64)(x) & (abup_uint64)((n)-1) ) == 0)
	typedef unsigned long abup_size_t;
#else
#define ABUP_ISALIGN(x, n)                  ( ( (abup_uint32)(x) & (abup_uint32)((n)-1) ) == 0)
	typedef unsigned int abup_size_t;
#endif

typedef   signed           int abup_intptr_t;

typedef unsigned           int abup_uintptr_t;

#define abup_sizeof(x)		(abup_size_t)sizeof(x)

#ifdef ABUP_CRC8
typedef abup_uint8     abup_crc_t;
#define calc_crc_ex calc_crc8_maxim
#define calc_crc_ex1 calc_crc8_ex
#define abup_check_key abup_crc8_check_key
#else
typedef abup_uint32     abup_crc_t;
#define calc_crc_ex calc_crc32_ex
#define calc_crc_ex1 calc_crc32_ex1
#define abup_check_key abup_crc32_check_key
#endif
typedef enum {
    STATE_INIT = 1,
    STATE_CV,//检测版本
    STATE_DL,//下载
    STATE_RD,//上报下载结果
    STATE_UG,//
    STATE_RU,//上报升级结果
    STATE_KY, //KEY
    STATE_RG, //注册
#if defined(ABUP_HTTP_5_0_MAX_LEN)
    STATE_CA, //根证书
    STATE_CFG, //服务器配置
    STATE_RD_CFG, //上报设备端信息
    STATE_RD_LOG, //上报升级日志
    STATE_CT, //终止
#endif
    STATE_END
} FotaState;

#define E_ABUP_SUCCESS			0
#define E_ABUP_FAILURE			-1
#define E_ABUP_NOMEMORY			-2
#define E_ABUP_NOTEXIST			-3
#define E_ABUP_WOULDBLOCK		-4
#define E_ABUP_SIM_NOTEXIST		-5
#define E_ABUP_MID_ERROR		-6
#define E_ABUP_NETWORK_ERROR		-7
#define E_ABUP_DOWNLOAD_FAIL		-8
#define E_ABUP_LAST_VERSION		-9
#define E_ABUP_TOKEN_WORNG		-10
#define E_ABUP_PROJECT_WORONG		-11
#define E_ABUP_PARAM_WORONG		-12
#define E_ABUP_NET_CONNECTING	-13
#define E_ABUP_DOWNLOAD_CONNECTING		-14
typedef enum 
{
	ABUP_DIFF_PATCH,
	ABUP_FULL_PATCH,
	ABUP_PATCH_END
} ABUP_PATCH_METHOD;
typedef abup_uint8   abup_bool;
#define abup_true  ((abup_bool)1)
#define abup_false ((abup_bool)0)

abup_char* abup_get_manufacturer(void);
abup_char* abup_get_model_number(void);
abup_char* abup_get_product_id(void);
abup_char* abup_get_product_sec(void);
abup_char* abup_get_device_type(void);
abup_char* abup_get_platform(void);
abup_char* abup_get_sdk_version(void);
abup_char* abup_get_apk_version(void);
abup_char* abup_get_firmware_version(void);
abup_char* abup_get_hw_version(void);
abup_char* abup_get_sw_version(void);
abup_char* abup_get_network_type(void);
abup_uint abup_get_try_timer(void);
abup_int8 abup_get_try_count(void);
abup_int32 abup_get_utc_time(void);
abup_int32 abup_get_downStart_time(void);
abup_int32 abup_get_downEnd_time(void);
abup_uint abup_get_default_module(void);
abup_bool abup_set_default_module(abup_uint module);
abup_uint8 abup_get_default_protocol(void);
abup_uint8 abup_get_default_segment_size(void);
abup_uint abup_get_data_max_len(void);
abup_char* abup_get_buf(void);
void abup_reset_buf(void);
abup_uint abup_get_buf_len(void);
#endif

