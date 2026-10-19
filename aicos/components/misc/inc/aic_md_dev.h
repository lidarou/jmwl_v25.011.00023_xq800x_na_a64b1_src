/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the misc device operation struct and interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_MISC_DEVICE_H__
#define __AIC_MISC_DEVICE_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_module.h>
#include <aic_osa.h>
#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_md_cfg.h>
#include "aic_md_dl.h"
#include "aic_md_dir.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
/* mutex */
#define MD_ACQUIRE_MUTEX(mutex_ptr, wait_timeout) \
if (AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    u32 mutex_status; \
    AOS_ASSERT(NULL != mutex_ptr); \
    mutex_status = aos_get_mutex(mutex_ptr, wait_timeout); \
    AOS_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define MD_RELEASE_MUTEX(mutex_ptr) \
if (AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    AOS_ASSERT(NULL != mutex_ptr); \
    aos_put_mutex(mutex_ptr); \
}

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/**
 * misc block information.
 *
 * @blk_addr: block address.
 * @free_space: free space size of the block.
 * @invalid_space: invalid space size of the block.
 * @sector_num: sector number
 */
typedef struct MD_BLK_INFO {
    u32             blk_addr;
    u32             free_space;
    u32             invalid_space;
    u32             sector_num;
} MD_BLK_INFO_S;

/**
 * misc device information.
 *
 * @type: MD_TYPE_E
 * @blk_num: block number.
 * @blk_size: block size.
 * @sector_size: sector size.
 * @blk_info: block information.
 * @current_blk_index: current malloc block index.
 * @dev_mutex: device mutex
 * @handle: file handle.
 * @timestamp: device timestamp.
 */
typedef struct MD_DEV_INFO {
    u32                 type;
    u32                 blk_num;
    u32                 blk_size;
    u32                 sector_size;
    MD_BLK_INFO_S       *blk_info;
    u32                 current_blk_index;
    u32                 handle;
    u32                 timestamp;
    u8                  attribute;
} MD_DEV_INFO_S;

/**
 * misc spare information.
 *
 * @handle: file handle.
 * @addr: spare address.
 * @size: spare size.
 * @sector_size: sector size.
 */
typedef struct MD_SPARE_INFO {
    u32         handle;
    u32         addr;
    u32         size;
    u32         sector_size;
} MD_SPARE_INFO_S;

/**
 * misc object.
 *
 * @dev_info: device information.
 * @dl_info: download information.
 * @spare_info: spare information.
 * @dir_info: directory information.
 * @buf: temp buf, MAX_MALLOC_SECTOR_NUM * MISC_SECTOR_SIZE.
 * @mutex: misc mutex.
 */
typedef struct MD_OBJ {
    MD_DEV_INFO_S       dev_info[MD_MAX_TYPE];
    MD_DOWNLOAD_INFO_S  dl_info;
    MD_SPARE_INFO_S     spare_info;
    MD_DIR_INFO_S       dir_info;
    u8                  *buf;
    aos_mutex_ptr       mutex;
    MD_BLK_CFG          blk_cfg;
} MD_OBJ_S;

/**
 * misc block check info struct define.
 *
 * @type: block type
 * @blk_index: block index.
 * @sector_start: write sector start.
 * @sector_num: write sector number.
 * @blk_addr: block address.
 * @status: block status, MD_BLK_STATUS_E.
 */
typedef struct MD_BLK_CHK_INFO {
    u32     type;
    u32     blk_index;
    u32     sector_start;
    u32     sector_num;
    u32     blk_addr;
    u32     status;
} MD_BLK_CHK_INFO_S;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * md_dev_init - misc device init.
 *
 * Parameter:
 *
 * Return:
 */
void md_dev_init(void);

/**
 * md_dev_write - write item data.
 *
 * Parameter:
 * @item_id: item id.
 * @size: write size.
 * @wbuf: write buf.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int md_dev_write(u16 item_id, u16 size, u8 *wbuf);

/**
 * md_dev_read - read item data.
 *
 * Parameter:
 * @item_id: item id.
 * @size: read size.
 * @rbuf: read buf.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int md_dev_read(u16 item_id, u16 size, u8 *rbuf);

/**
 * md_dev_add - add item data.
 *
 * Parameter:
 * @item_id: item id.
 * @size: write size.
 * @wbuf: write buf.
 * @type: device buf.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int md_dev_add(u16 item_id, u16 size, u8 *wbuf, u8 type);

/**
 * md_dev_del - delete item.
 *
 * Parameter:
 * @item_id: item id.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int md_dev_del(u16 item_id);

/**
 * md_dev_blk_check - check misc block.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 * @blk_index: block index.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int md_dev_block_check(MD_OBJ_S *md_obj, MD_TYPE_E type, u32 blk_index);

/**
 * md_dev_update_blk_status - update misc block check information.
 *
 * Parameter:
 * @handle: device handle.
 * @type: device type.
 * @blk_chk: block check information.
 * @status: block check status.
 * @block_addr:block address.
 *
 * Return:
 *
 */
void md_dev_update_blk_status(u32 handle, u32 type,
                              MD_BLK_CHK_INFO_S *blk_chk,
                              MD_BLK_STATUS_E status,
                              u32 block_addr);

/**
 * md_dev_get_dir - get item dir.
 *
 * Parameter:
 * @item_id: item id.
 * @dir: item directory.
 *
 * Return:
 *
 */
void md_dev_get_dir(u16 item_id, MD_DIR_ITEM_S *dir);

/**
 * md_dev_restore - restore misc block.
 *
 * Parameter:
 * @type: misc type.
 *
 * Return:
 *
 */
int md_dev_restore(MD_TYPE_E type);

/**
 * md_dev_var_init - misc device value init.
 *
 * Parameter:
 * @md_obj: misc object.

 * Return:
 */
void md_dev_var_init(MD_OBJ_S **md_obj_pp);

/**
 * md_dev_update - update misc.
 *
 * Parameter:
 * @md_obj: misc object.
 *
 * Return:
 *
 */
void md_dev_update(MD_OBJ_S *md_obj);

/**
 * md_dev_recovery_from_spare -
 *      abnormal power off, recovery misc from spare area.
 *
 * Parameter:
 * @md_obj: misc object.
 *
 * Return:
 *
 */
void md_dev_recovery_from_spare(MD_OBJ_S *md_obj);

/**
 * md_dev_check - check misc block.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 *
 * Return:
 *    @status: AIC_SUCCESS is success, other fail.
 */
int md_dev_check(MD_OBJ_S *md_obj, MD_TYPE_E type);

/**
 * md_dev_recovery_from_dl - recovery misc from download.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 *
 * Return:
 *
 */
void md_dev_recovery_from_dl(MD_OBJ_S *md_obj, MD_TYPE_E type);

/**
 * md_dev_erase_block - erase misc block.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 *
 * Return:
 *
 */
void md_dev_erase_block(MD_OBJ_S *md_obj, MD_TYPE_E type);

/**
 * md_dev_cali_update - update misc calibration partition.
 *
 * Parameter:
 * @md_obj: misc object.
 * @type: misc type.
 *
 * Return:
 *
 */
void md_dev_cali_update(MD_OBJ_S *md_obj, MD_TYPE_E type);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_MISC_DEVICE_H__ */

