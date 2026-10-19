/*****************************************************************************/
/*                                                                           */
/*         Copyright 2023 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the remap api.
 */
#ifndef __CP_REMAP_H__
#define __CP_REMAP_H__

#ifdef CONFIG_CP_REMAP_DIS_MDM
#include <psram_mem_config.h>
#endif

/*
 * cp_remap_psram_map
 * cp remap a section of psram.
 *
 * @src: the source base addr of psram.
 * @dst: the dst address of map. the dst can only be
 *       0x1xxxxxxx, 0x2xxxxxxx, 0x8xxxxxxx.
 * @size the size  of the remap section.
 *
 * After remap:
 *   the src range is [src, src + size)
 *   the dst range is [dst, dst + size)
 * the dst range and the src range can overlaps, if the
 * overlap, the remap is valid in the overlapping part.
 */
int cp_remap_psram_map(u32 src, u32 dst, u32 size);

/*
 * cp_remap_iram_map
 * cp remap a section of iram.
 *
 * @src: the source base addr of iram.
 * @dst: the dst address of map. the dst can only be
 *       0x1xxxxxxx, 0x2xxxxxxx, 0x8xxxxxxx.
 * @size the size  of the remap section.
 *
 * After remap:
 *   the src range is [src, src + size)
 *   the dst range is [dst, dst + size)
 * the dst range and the src range can overlaps, if the
 * overlap, the remap is valid in the overlapping part.
 */
int cp_remap_iram_map(u32 src, u32 dst, u32 size);

/*
 * cp_remap_ram_enable
 * enable or disable psram or iram remap function.
 *
 * @enable: TRUE - enable, FALSE - disable.
 * @psram:  TRUE - psram, FALSE - iram.
 */
void cp_remap_ram_enable(bool enable, bool psram);

#define cp_remap_psram_enable()     cp_remap_ram_enable(TRUE, TRUE)
#define cp_remap_psram_disable()    cp_remap_ram_enable(FALSE, TRUE)

#define cp_remap_iram_enable()      cp_remap_ram_enable(TRUE, FALSE)
#define cp_remap_iram_disable()     cp_remap_ram_enable(FALSE, FALSE)

#ifdef CONFIG_CP_REMAP_DIS_MDM
#define cp_psram_addr_to_mdm_addr(addr) ((u32)(addr) + CP_PSRAM_REMAP_OFFSET)
#define cp_iram_addr_to_mdm_addr(addr)  ((u32)(addr) + CP_IRAM_REMAP_OFFSET)
#else
#define cp_psram_addr_to_mdm_addr(addr) (addr)
#define cp_iram_addr_to_mdm_addr(addr)  (addr)
#endif

#endif /* __CP_REMAP_H__ */

