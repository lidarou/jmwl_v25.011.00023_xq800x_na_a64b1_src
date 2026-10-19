/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    l1 cache interface declareration.
*/

#ifndef __L1C_H__
#define __L1C_H__

void l1dcache_inv_all(void);

void l1dcache_clean_all(void);

void l1dcache_invcc_all(void);

void l1dcache_inv_by_range(u32 addr, size_t length);

void l1dcache_clean_by_range(u32 addr, size_t length);

void l1dcache_invcc_by_range(u32 addr, size_t length);

void l1dcache_enable(void);

void l1dcache_disable(void);

void l1icache_inv_all(void);

void l1icache_inv_by_range(u32 addr, size_t length);

void l1icache_enable(void);

void l1icache_disable(void);

void slp_l1dc_enable(void);

void slp_l1ic_enable(void);

void slp_l1c_enable(void);

#endif /* __L1C_H__ */
