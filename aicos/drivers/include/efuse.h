/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/
#ifndef __EFUSE_H__
#define __EFUSE_H__
#include <aic_type.h>
/**
 * aic_efuse_block_write - write efuse block.
 * @blk: efuse block ID.
 * @blk_val: value write to efuse block.
 * return 0 if set success, else fail.
 */
int aic_efuse_block_write(int blk, u32 blk_val);

/**
 * aic_efuse_block_read - read efuse block data.
 * @blk: efuse block ID.
 * @blk_val: point to value read from efuse block.
 * return 0 if set success, else fail.
 */
int aic_efuse_block_read(int blk, u32 *blk_val);

/**
 * aic_get_uid - get chip uid.
 * @uid: point to uid read from efuse.
 * @len: point to uid read length.
 * ATTENTION:uid pointer to space should greater than 50 bytes.
 * return 0 if set success, else fail.
 */
int aic_get_uid(u8 *uid, int *len);

/**
 * aic_write_efuse_enob - write efuse block only effective number of bits.
 * @src: point to data to be written.
 * @blk_idx: write start efuse block idx.
 * @bit_idx: write start efuse bit idx.
 * @bits_len: bits len.
 * return 0 if set success, else fail.
 */
int aic_write_efuse_enob(uint8_t *src, int blk_idx, int bit_idx, int bits_len);

/**
 * aic_read_efuse_enob - read efuse block only effective number of bits.
 * @dest: point to data read from efuse.
 * @blk_idx: read start efuse block idx.
 * @bit_idx: read start efuse bit idx.
 * @bits_len: bits len.
 * ATTENTION:dest pointer to space should match with bits_len.
 * return 0 if set success, else fail.
 */
int aic_read_efuse_enob(uint8_t *dest, int blk_idx, int bit_idx, int bits_len);

/**
 * aic_efuse_block_lock - lock efuse block, blk unable to write.
 * @blk: efuse block num.
 * return 0 if set success, else fail.
 */
int aic_efuse_block_lock(int blk);
#endif /*__EFUSE_H__*/