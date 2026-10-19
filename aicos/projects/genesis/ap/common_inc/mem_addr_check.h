/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This memory address check header file.
 */

#ifndef __MEM_ADDR_CHECK_H__
#define __MEM_ADDR_CHECK_H__

#include <aic_type.h>
#include <flash_config.h>
#include <flash_mem_config.h>

#define MEM_ADDR_CHECK(a, b, c) (((a) >= (b)) && ((a) <= ((b) + (c) - 1)))
static inline boolean mem_is_access_in_flash_area(u32 addr)
{
#if (QSPI_NOR_DEVICE_NUM == 1)
    return MEM_ADDR_CHECK(addr, FLASH0_BASE, FLASH0_TOTAL_SIZE);
#elif (QSPI_NOR_DEVICE_NUM == 2)
    return (MEM_ADDR_CHECK(addr, FLASH0_BASE, FLASH0_TOTAL_SIZE) ||
            MEM_ADDR_CHECK(addr, FLASH1_BASE, FLASH1_TOTAL_SIZE));
#endif
}

#endif /* __MEM_ADDR_CHECK_H__ */

