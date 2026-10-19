
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the secboot header file of the subsys.
 */
#ifndef __SUBSYS_SEC_H__
#define __SUBSYS_SEC_H__
#include <subsys_drv.h>

/*
 * CONFIG_SECURE_VERIFY_ON_RAM
 * if the lzma image compress frist then sign, can verufy it on flash directly,
*  and need pass the header first then uncompress to ram.
 * otherwise need uncompress to ram first, then verify it,
*  and need provide the sec_head for every sec image to verify.
 */
#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_SEC_COMP_FIRST_THEN_SIGN
#define CONFIG_SECURE_VERIFY_ON_RAM
#endif
#endif

size_t subsys_get_head_size(void);
size_t subsys_sec_get_head_size(void);
size_t subsys_sec_get_tail_size(void);

#ifdef CONFIG_SECURE_BOOT
void subsys_sec_disble_protect(u8 sys);
void subsys_sec_enable_protect(u8 sys);
void subsys_sec_verify_and_start(u8 sys, u32 region_num, subsys_region_t *region_tab);

#endif

#endif /* __SUBSYS_SEC_H__ */

