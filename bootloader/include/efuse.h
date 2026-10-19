/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of efuse module.
 *   It includes all the efuse phy function api.
*/
#ifndef __EFUSE_PHY_H__
#define __EFUSE_PHY_H__

#define EFUSE_BLK_NUMS      64
#define EFUSE_BLK_LOCK_BIT      31
//#define EFUSE_CV_DEBUG
#define EFUSE_INT_WR_FINISH
#define EFUSE_WRITE_MAGIC       0X12EF
#define EFUSE_WAIT_TIME_CNT     100
#define AIC_CACHE_ENABLED       TRUE
#define AIC_AUTO_PD_ENABLED     TRUE
#define EFUSE_ENOB_LENTH        31

/*efuse phy driver interface*/
int aic_efuse_write(int blk, u32 data);
int aic_efuse_read(int blk,u32 *data);
int aic_write_efuse_enob(uint8_t *src, int blk_idx, int bit_idx, int bits_len);
int aic_read_efuse_enob(uint8_t *dest, int blk_idx, int bit_idx, int bits_len);

/**
 * get_uid - get chip uid.
 * @buf: point to uid read from efuse,
 * @len: read uid len bytes.
 * ATTENTION:uid pointer to space should greater than 50 bytes.
 * return 0 if set success, else fail.
 */
int get_uid(u8 *buf, int *len);
#endif /* __EFUSE_PHY_H__ */
