/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sdmmc dev interface.
 */

#ifndef __SDMMC_DEV__
#define __SDMMC_DEV__

#include <aic_type.h>
#include <sdmmc_card.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sdmmc_dev_st {
    char *name;
    u32 ref_count;
    u32 sector_size;
    u32 sector_num;
    u64 total_size;
    sdmmc_card *card;
    size_t max_req_size;
    u32 card_status;  /* 1: ready, 0: not ready */
};

/**
* sdmmc_get_card_status() - get sdcard status.
*
* Return: 1: ready, 0: not ready.
*/
u32 sdmmc_get_card_status();
int sdmmc_open(char *name, u32 *handle);
int sdmmc_close(u32 handle);
int sdmmc_read(u32 handle, u32 sector_id, u32 sector_num, u8 *buf);
int sdmmc_write(u32 handle, u32 sector_id, u32 sector_num, u8 *buf);
int sdmmc_release(u32 handle, u32 sector_id, u32 sector_num);
int sdmmc_get_sector_size(u32 handle, u32 *retsize);
int sdmmc_get_sector_num(u32 handle, u32 *retnum);
int sdmmc_get_partition_size(u32 handle, u32 *retsize);

#ifdef __cplusplus
}
#endif

#endif
