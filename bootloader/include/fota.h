/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of fota.
*/

#ifndef __FOTA_H__
#define __FOTA_H__

#include <aic_type.h>
#include <io.h>

/**
 * fota_entry() - Process FOTA
 *
 * Update firmware which loaded to local before, restart after
 * update done.
 *
 * Return: None
 *
 */
void fota_entry(void);

/**
 * is_fota_needed() - Need to process FOTA or not
 *
 * Check ota status flag and backup ota status flag, if any of
 * them equals s1(0x6f746173),return true, or return false.
 *
 * Return: true if need to process FOTA; false if not.
 *
 */
BOOL is_fota_needed(void);

#ifdef CONFIG_FOTA_SUPPORT
bool fota_is_normal_mode(void);
#else
static inline bool fota_is_normal_mode(void) { return true; }
#endif

#endif /* __FOTA_H__ */
