/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of performance monitor module.
 *   It includes all the performance monitor common function api.
 */

#ifndef __PERF_INC_H__
#define __PERF_INC_H__

typedef enum  {
    PERF_MEM_M0,
    PERF_MEM_M1,
    PERF_MEM_M2,
    PERF_IP_MAX
} PERF_IP;

typedef enum  {
    PERF_CHN0,
    PERF_CHN1,
    PERF_CHN2,
    PERF_CHN_MAX
} PERF_CHN;

typedef enum {
    PERF_WRITE = 0,
    PERF_READ,
    PERF_RW
} PERF_OP_TYPE;

typedef enum {
    PERF_ALL_MEM = 0,
    PERF_IRAM,
    PERF_FLASH,
    PERF_PSRAM
} PERF_ADDR_TYPE;

void gx_perf_set_userid(PERF_IP ip, PERF_CHN chn, u32 userid);
void gx_perf_unset_userid(PERF_IP ip, PERF_CHN chn);
void gx_perf_cfg(PERF_IP ip, PERF_CHN chn, PERF_OP_TYPE op, PERF_ADDR_TYPE type);

#endif /* __PERF_INC_H__ */

