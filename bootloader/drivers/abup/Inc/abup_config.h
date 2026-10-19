/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_config.h
* Description:
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef _ABUP_CONFIG_H_
#define _ABUP_CONFIG_H_
#include "abup_os.h"
#include "reg_base.h"
#include "flash_mem_config.h"

#define ABUP_HOLD_KEY
//#include "xxx_custom_decompress.h"
//#define EXTERNAL_MEMORY_MANAGEMENT
#define ABUP_CHECK_PATCH_FILE 1
#define ABUP_FLASH_DEFAULT_VALUE 0xFF
#ifndef ABUP_ALGORITHM_PAUSE
//#define ABUP_ALGORITHM_PAUSE
#endif
#define ABUP_SEGMENT_SIZE_16_INDEX        0
#define ABUP_SEGMENT_SIZE_32_INDEX        1
#define ABUP_SEGMENT_SIZE_64_INDEX        2
#define ABUP_SEGMENT_SIZE_128_INDEX       3
#define ABUP_SEGMENT_SIZE_256_INDEX       4
#define ABUP_SEGMENT_SIZE_512_INDEX       5

#define ABUP_PROTOCOL_NONE 0
#define ABUP_PROTOCOL_COAP 1
#define ABUP_PROTOCOL_HTTP 2
#define ABUP_PROTOCOL_COAP_HTTP (ABUP_PROTOCOL_COAP|ABUP_PROTOCOL_HTTP)
//#define ABUP_UA
#define ABUP_NO_DISPLAY
#define _ABUP_INLINE_
#define ABUP_SLIM_RES
//#define ABUP_FILE
#define ABUP_MODULE_ENABLE_POWERONOFF 0
#define ABUP_LZMA_DICT_SIZE_DEFAULT 0x40000
#define ABUP_APP_TEST_DATA 0
#define LOG_DEBUG 0
#define ABUP_MAX_DELTA_NUM 20


#ifndef ABUP_MAX_DELTA_NUM
#define ABUP_MAX_DELTA_NUM 5
#endif
//The algorithm needs memory, wosun as large as possible, Lusun minimum 32 bytes
#ifndef ABUP_WORKING_BUFFER_LEN
#define ABUP_WORKING_BUFFER_LEN FLASH0_FOTA_BAK_SIZE
#endif
//0 is lusun, memory requirement is not less than 32 bytes, 1 is wosun bz, memory requirement is not less than 200K 2 is wosun lzma, memory requirement is not less than 512K
#define ABUP_FOTA_ALGORITHM 1
#ifndef WOSUN_PURE_ALGORITHM
//#define WOSUN_PURE_ALGORITHM
#endif
#ifdef WOSUN_PURE_ALGORITHM
#ifndef EXTERNAL_MEMORY_MANAGEMENT
#define EXTERNAL_MEMORY_MANAGEMENT
#endif
#endif
//debug
#if (ABUP_BOOTLOADER_DEBUG == 1)||(ABUP_APP_DEBUG == 1)
#ifndef ABUP_DEBUG_MODE
#define ABUP_DEBUG_MODE
#endif
#endif

//Download segment length
#define ABUP_DEFAULT_SEGMENT_SIZE_INDEX        ABUP_SEGMENT_SIZE_512_INDEX
//bid
//#define ABUP_UB_BID "bid"
//device
//SECTOR/BLOCK size
#define ABUP_DEFAULT_SECTOR_SIZE   0x1000
//Flash base address
#define ABUP_FLASH_BASE_ADDR       0x08000000
//Flash end address
#define ABUP_FLASH_BASE_END        0x08200000
//bootloader address
#define ABUP_FLASH_BL_ADDR         0x08000000
//bootloader size
#define ABUP_BL_SIZE               0x00005000
//App address
#define ABUP_FLASH_APP_ADDR        0x08005000
//App size
#define ABUP_APP_SIZE              0x00008000
//Differential packet data area address
#define ABUP_UPDATE_ADDR           FLASH0_FOTA_BASE
//The size of the differential packet data area, including the maximum value of the differential packet + restore backup block (1 SECTOR/BLOCK size) + upgrade information block (default 1SECTOR/BLOCK size, you can leave the differential packet data area)
#define ABUP_UPDATE_SIZE           FLASH0_FOTA_SIZE
//Whether the upgrade information block is stored separately 0 Not stored separately 1 Stored separately
#define ABUP_FLASH_UNEVEN_FLASH_BLOCK 0
//Upgrade information block address
#define ABUP_FLASH_BASE_PATCH_INFO 0x00000000
//Upgrade information block size
#define ABUP_FLASH_BASE_PATCH_SECTOR_SIZE 0x00000000
//App backup address
#define ABUP_FLASH_APP_BK_ADDR FLASH0_FOTA_BAK_BASE
//#define GX316
#define ABUP_COMPRESS_SUPPORT

#define HEADER_STR                              "lenc"
#define MAX_PARTITION_NAME_SIZE                 (10)
#define ABUP_ENC_DATA_SIZE                      (ABUP_WORKING_BUFFER_LEN)
#define ABUP_DEC_DATA_SIZE                      (0x80000)
#define LOG_DETAIL  0

//NETWORK TYPE
#define ABUP_NETWORK_TYPE            "NB"
//KEY VERSION
#define ABUP_KEY_VERSION            2
#if defined(ABUP_KEY_VERSION)&&(ABUP_KEY_VERSION == 4)
#ifndef ABUP_CRC8
#define ABUP_CRC8
#endif
#endif
#ifndef ABUP_CRC32_LOOKUP_TABLE
//#define ABUP_CRC32_LOOKUP_TABLE
#endif

#if defined(ABUP_CHECK_PATCH_FILE)&&(ABUP_CHECK_PATCH_FILE == 1)
#ifndef ABUP_FIND_BADCRC
#define ABUP_FIND_BADCRC
#endif
#endif
//"IOT5.0_ LUSUN11_ R50426 "is Lusun algorithm
//Otherwise, it is the wosun algorithm, which defaults to "IOT4.0_R42641"
#define ADUPS_FOTA_LUSUN_VERSION "IOT5.0_LUSUN11_R50426"
#define ADUPS_FOTA_WOSUN_VERSION "IOT4.0_R42641"
#if (ABUP_FOTA_ALGORITHM == 0)
#define ADUPS_FOTA_SDK_VER ADUPS_FOTA_LUSUN_VERSION
#else
#define ADUPS_FOTA_SDK_VER ADUPS_FOTA_WOSUN_VERSION
#endif
#define ADUPS_FOTA_APP_VERSION    "ADUPS_V4.0"


#define ABUP_HW_VERSION              "HW01"
#define ABUP_SW_VERSION              "SW01"

#endif

