/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    tcm interface.
*/

#ifndef __TCM_H__
#define __TCM_H__

/* Put ATCM and BTCM section to TCM and fetch code from TCM */
void tcm_preload_enable(void);

/* Disable fetch code from tcm */
void tcm_disable(void);

#endif /* __TCM_H__ */
