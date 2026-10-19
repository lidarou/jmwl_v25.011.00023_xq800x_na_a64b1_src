/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines pmic LPR(long press reset) driver api.
 */

#ifndef __LPR_H__
#define __LPR_H__

#define DOWNLOAD_LAST_PARTITION "factory_bak"
/*period call this api make sure wouldn't trigger lpr*/
int lpr_disable_kick(void);
void lpr_disable_loop(void);

#endif /* __LPR_H__ */
