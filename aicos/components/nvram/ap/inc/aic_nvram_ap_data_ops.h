/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** Export the nvram data process Interface (API).
 **---------------------------------------------------------------------------*/

#ifndef __AIC_NVRAM_AP_DATA_OPS_H__
#define __AIC_NVRAM_AP_DATA_OPS_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_nvram_device_config.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Data Structures
 **---------------------------------------------------------------------------*/
/**
 * nvram ap device block information.
 *
 * @addr: block address.
 * @sector_num: sector number
 */
typedef struct _NVRAM_AP_DEV_BLK_INFO {
    u32     addr;
    u32     sector_num;
} NVRAM_AP_DEV_BLK_INFO;

/**
 * nvram ap device information.
 *
 * @blk_info: block information.
 * @config: device config
 * @id: device id
 * @handle: file handle.
 * @bak_handle: backup file handle.
 * @blk_size: block size.
 * @sector_size: sector size.
 * @blk_num: block number.
 * @size: device flash size.
 * @timestamp: device timestamp.
 * @tmcrc: download and custom download timestamp crc.
 */
typedef struct _NVRAM_AP_DEV_INFO {
    NVRAM_AP_DEV_BLK_INFO   *blk_info;
    NVRAM_DEVICE_CONFIG     *config;
    u32                     id;
    u32                     handle;
    u32                     bak_handle;
    u32                     blk_size;
    u32                     sector_size;
    u32                     blk_num;
    u32                     size;
    u32                     timestamp;
    u32                     tmcrc;
} NVRAM_AP_DEV_INFO;

/**
 * nvram ap device spare information.
 *
 * @handle: file handle.
 * @addr: spare address.
 * @size: spare size.
 * @sector_size: sector size.
 * @sector_num: sector number.
 */
typedef struct _NVRAM_AP_DEV_SPARE_INFO {
    u32     handle;
    u32     addr;
    u32     size;
    u32     sector_size;
    u32     sector_num;
} NVRAM_AP_DEV_SPARE_INFO;

/**
 * nvram ap device block check information.
 *
 * @id: device id
 * @blk_index: block index.
 * @sector_start: start sector offset.
 * @sector_num: sector number.
 * @blk_addr: block address.
 * @status: block status, NVRAM_BLK_STATUS.
 */
typedef struct _NVRAM_AP_BLK_CHK_INFO {
    u32     id;
    u32     blk_index;
    u32     sector_start;
    u32     sector_num;
    u32     blk_addr;
    u32     status;
} NVRAM_AP_BLK_CHK_INFO;

/**
 * nvram file check.
 *
 * @magicnum: file magic number.
 * @size: data size.
 * @checksum: data checksum.
 * @reserved: no use
 */
typedef struct _NVRAM_FILE_CHK {
    u32     magicnum;
    u32     size;
    u16     checksum;
    u16     reserved;
} NVRAM_FILE_CHK;

/**
 * nvram object.
 *
 * @dev_info: device information.
 * @spare_info: spare information.
 * @recv_packet_ptr: receive packet ptr.
 * @buf: malloc buf, use for data cache.
 */
typedef struct _NVRAM_AP_OBJ_INFO {
    NVRAM_AP_DEV_INFO       *dev_info;
    NVRAM_AP_DEV_SPARE_INFO *spare_info;
    u8                      *packet_ptr;
    u8                      *recv_packet_ptr;
    u8                      *buf;
    aos_mutex_ptr           mutex;
    NVRAM_BLK_CFG           blk_cfg;
} NVRAM_AP_OBJ_INFO;

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
* nvram_ap_data_process - data process.
 *
 * Parameter:
 * @cmd: NVRAM_SYNC_CMD
 * @data: process data.
 *
 * Return:
 */
void nvram_ap_data_process(u16 cmd, u32 data);

/**
 * nvram_ap_data_init - data init and check
 *
 * Parameter:
 *
 * Return:
 *
 */
void nvram_ap_data_init(void);

/**
 * nvram_ap_load_data - load device data to psram.
 *
 * Parameter:
 *
 * Return:
 *
 */
void nvram_ap_load_data(void);

/**
 * nvram_ap_free_download_buf - free download buf.
 *
 * Parameter:
 *
 * Return:
 *
 */
void nvram_ap_free_download_buf(void);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_NVRAM_AP_DATA_OPS_H__ */

