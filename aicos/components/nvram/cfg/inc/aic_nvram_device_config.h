/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * EXport the nvram device config API.
 */

#ifndef __AIC_NVRAM_DEVICE_CONFIG_H__
#define __AIC_NVRAM_DEVICE_CONFIG_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_math.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/* Device Name Definitions */
#define NVRAM_MAX_DEVICE_NAME_LEN           30

/* Device ID Definitions */
typedef enum _NVRAM_DEVICE_ID{
    NVRAM_DIRECTORY_DEVICE_ID = 0,
    NVRAM_DIFF_DEVICE_ID,
    NVRAM_DOWNLOAD_DEVICE_ID,
    NVRAM_CUSTOM_DOWNLOAD_DEVICE_ID,
    NVRAM_CALIBRATION_DEVICE_ID,
    NVRAM_NORMAL_DEVICE_ID,
    NVRAM_MAX_DEVICE_ID
} NVRAM_DEVICE_ID;

#define NVRAM_DEVICE_MAXNUM                 NVRAM_MAX_DEVICE_ID

/* category define */
typedef enum _NVRAM_CATEGORY{
    NVRAM_CATEGORY_NORMAL = 1,
    NVRAM_CATEGORY_CALIBRATION,
    NVRAM_CATEGORY_MAX
} NVRAM_CATEGORY;

/* 
 * dirtysize = maxsize * 4byte(32bit 1<<5), every bit is mean one sector size.
 */
#define NVRAM_DEVICE_DIRTYTABLE_MAXSIZE     13

/* directory size */
#define NVRAM_DIRECTORY_ITEM_SIZE           sizeof(NVRAM_DIRECTORY_ENTRY)

/* item use status */
#define NVRAM_ITEM_NOT_USED                 0x0
#define NVRAM_ITEM_USED                     BIT_0
#define NVRAM_ITEM_USED_DOWNLOAD            BIT_1

#define NVRAM_MAGICNUM                      0x4e56524d      /* "NVRM" */
#define NVRAM_INIT_MAGICNUM                 0x4e564954      /* "NVIT" */
#define NVRAM_ERR_MAGICNUM                  0xFFFFFFFF

/**
 * nvram block status.
 *
 * @NVRAM_BLK_ERASE: ERAS->0x45524153
 * @NVRAM_BLK_PASS: PASS->0x50415353
 * @NVRAM_BLK_RESTORE: REST->0x52455354, restore factore settings.
 */
typedef enum NVRAM_BLK_STATUS {
    NVRAM_BLK_ERASE    = 0x45524153,
    NVRAM_BLK_PASS     = 0x50415353,
    NVRAM_BLK_RESTORE  = 0x52455354,
    NVRAM_BLK_NOWR     = 0xFFFFFFFF
} NVRAM_BLK_STATUS_E;

/**-----------------------------nvram sync ram malloc from AIPC----------------*
 |      name                 |      size
 |--------------------------------------------|
 |NVRAM_PACKET_SEND          |
 |--------------------------------------------|
 |NVRAM_PACKET_RESPOND       |
 |--------------------------------------------|
 |TOTAL                      |
 **---------------------------------------------------------------------------*/
#define NVRAM_SYNC_PACKET_SEND_SIZE \
        (sizeof(NVRAM_SYNC_PACKET_DATA) + sizeof(NVRAM_SYNC_PACKET_HEAD))
#define NVRAM_SYNC_PACKET_RESPOND_SIZE \
        sizeof(NVRAM_SYNC_PACKET_HEAD)
#define NVRAM_SYNC_PACKET_TOTAL_SIZE \
        (NVRAM_SYNC_PACKET_SEND_SIZE + NVRAM_SYNC_PACKET_RESPOND_SIZE)

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef struct
{
    u32     id;
    char    *name;
    char    *bak_name;
    u32     ro_addr;    /* ro addr */
    u32     addr;       /* operation addr */
    u32     size;
}NVRAM_DEVICE_CONFIG;

typedef struct _NVRAM_HEADER{
    u16     id;
    u16     size;
    u16     checksum;
    u8      category;
    u8      status;
}NVRAM_HEADER;

typedef struct _NVRAM_DIRECTORY_ENTRY {
    /* Size in byte of data field of an item */
    u16     size;
    u8      category;
    /* Status of the item */
    u8      status;
    /* Offset of the item, measured from the beginning of nvram partition */
    u32     offset;
} NVRAM_DIRECTORY_ENTRY, *PNVRAM_DIRECTORY_ENTRY;

/* data sync */
typedef struct
{
    u32  magicnum;
    u32  group_idx;
    u32  size;
}NVRAM_SYNC_PACKET_HEAD;

typedef struct
{
    u32 part_index;                        /* parition Id */
    u32 addr;                              /* sync addr*/
    u32 dirty[NVRAM_DEVICE_DIRTYTABLE_MAXSIZE]; /* sync data */
    u32 size;                              /* size of sync data */
}NVRAM_SYNC_PACKET_DATA;

typedef struct {
    u32 sector_size;
    u32 blk_size;
    u32 blk_chk_size;
    u32 blk_data_size;
    u32 flash_sector_size;
} NVRAM_BLK_CFG;

/**---------------------------------------------------------------------------*
 **                         Function Prototypes                               *
 **---------------------------------------------------------------------------*/
/**
* nvram_device_config_table_get - get device config table.
*
* Return:
*    @device config table.
*/
NVRAM_DEVICE_CONFIG *nvram_device_config_table_get(void);

/**
* aic_nvram_get_whitelist_cfg - get nvram whitelist id config.
*
* Parameter:
*   @whitelist: nvram whitelist item id.
*   @count: whitelist id count.
*
* Return:
*
*/
void aic_nvram_get_whitelist_cfg(u16 **whitelist, u32 *count);

/**
* aic_nvram_get_blk_cfg - get nvram block config.
*
* Parameter:
*   @blk_cfg: nvram block config.
*
* Return:
*
*/
void aic_nvram_get_blk_cfg(NVRAM_BLK_CFG *blk_cfg);

/**
* aic_nvram_get_devname - get nvram device name by category.
*
* Parameter:
*   @category: nvram category, NVRAM_CATEGORY.
*
* Return:
*   nvram device name.
*/
char *aic_nvram_get_devname(u8 category);

/**
* aic_nvram_get_devname_bydevid - get nvram device name by device id.
*
* Parameter:
*   @devid: nvram device id, NVRAM_DEVICE_ID.
*
* Return:
*   nvram device name.
*/
char *aic_nvram_get_devname_bydevid(u32 devid);

/**
* aic_nvram_get_max_id - get nvram max id.
*
* Parameter:
*
* Return:
*   return nvram max id.
*/
u16 aic_nvram_get_max_id();

/**
* aic_nvram_get_min_id - get nvram min id.
*
* Parameter:
*
* Return:
*   return nvram min id.
*/
u16 aic_nvram_get_min_id();

#endif /* __AIC_NVRAM_DEVICE_CONFIG_H__ */

