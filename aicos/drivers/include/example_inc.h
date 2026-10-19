/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines EXTERNAL elements of example driver.
 *    The file will be included by software which need example module service.
 */

#ifndef __EXAMPLE_INC_H__
#define __EXAMPLE_INC_H__

#include <aic_bitmap.h>
#include "drv_common.h"

/* example control codes */
#define AIC_EXAMPLE_IO_A                    (0x01UL)
#define AIC_EXAMPLE_IO_B                    (0x02UL)
#ifdef CONFIG_EXAMPLE_VER_1
#define AIC_EXAMPLE_IO_C1                   (0x03UL)
#endif
#ifdef CONFIG_EXAMPLE_VER_2
#define AIC_EXAMPLE_IO_C2                   (0x03UL)
#endif

/* example capabilities */
#define AIC_EXAMPLE_CAP_A                    (BIT_0)
#define AIC_EXAMPLE_CAP_B                    (BIT_1)
#ifdef CONFIG_EXAMPLE_VER_1
#define AIC_EXAMPLE_CAP_C1                   (BIT_2)
#endif
#ifdef CONFIG_EXAMPLE_VER_2
#define AIC_EXAMPLE_CAP_C2                   (BIT_2)
#endif

/* example driver capabilities */
struct aic_example_capabilities {
    /* DEFINITION FOR SPECIFIED MODULE */
    uint32_t cap;
} ;

/**
 * example driver struct.
 * @get_versions: driver and hardware versions function
 * @get_capabilities: driver capabilities function
 * @initialize: initialize function
 * @uninitialize: uninitialize function
 * @io_control: io control function
 * @function_a: driver function a
 * @function_b: driver function b
 * @function_c: driver function c
 */
struct aic_example_driver {
  struct aic_driver_version       (*get_version)          (void);
  struct aic_example_capabilities (*get_capabilities)     (void);
  int32_t                         (*initialize)           (void);
  int32_t                         (*uninitialize)         (void);
  int32_t                         (*io_control)           (uint32_t control, uint32_t arg);
  int32_t                         (*function_a)           (void);
  int32_t                         (*function_b)           (void);
#ifdef CONFIG_EXAMPLE_VER_2
  int32_t              (*function_c)           (void);
#endif
};

extern struct aic_example_driver driver_example;

#endif /* __EXAMPLE_INC_H__ */

