/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of ipu module.
 *   It includes all the ipu common function api.
*/

#ifndef __IPU_INC_H__
#define __IPU_INC_H__

typedef enum _ipu_type_e
{
    IPU_MCU = 0x0,
    IPU_SEC,
    IPU_CP,
    IPU_ND,
    IPU_MAX,
}  IPU_TYPE_E;

void IPU_IRQEnable(u32 value, IPU_TYPE_E index);
void IPU_IRQDisable(u32 value, IPU_TYPE_E index);

#endif /* __IPU_INC_H__ */
