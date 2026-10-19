/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert dump interface.
 */

#ifndef __ASSERT_DUMP_API_H__
#define __ASSERT_DUMP_API_H__

#include <aic_type.h>

enum assert_dump_type {
    DUMP_MEM = 0,
    DUMP_REG,
    DUMP_RF_HREG,
    DUMP_RF_WREG,
};

typedef u32 (*dump_read_f)(u32 address);
typedef void (*dump_prepare_f)(void);
typedef void (*dump_post_f)(void);
typedef u8 (*dump_check_f)(void);

void assert_dump_region_register(enum assert_dump_type type, u32 address,
                                 u32 length, dump_read_f func,
                                 dump_prepare_f pre_func,
                                 dump_post_f pst_func);

void assert_dump_region_register_ex(enum assert_dump_type type,
                                    #ifdef CONFIG_SUBSYS_CTRL_SUPPORT
                                    u8 sys,
                                    u8 mpu_conf,
                                    #endif
                                    u32 address, u32 length,
                                    dump_read_f func,
                                    dump_prepare_f pre_func,
                                    dump_post_f pst_func,
                                    dump_check_f check_func, u32 default_val);

#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
void assert_dump_region_register_mem(u8 sys, u8 mpu_conf,
                                     u32 address, u32 length);
#endif

#endif /* __ASSERT_DUMP_API_H__ */
