/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_HW_CFG_H__
#define __USB_HW_CFG_H__

#include <ckm_en_glb_mcu.h>
#include <mcu_glb.h>
#include <reg_base.h>
#include <usb/usb_device.h>

__attribute__((section(".usb.data"), aligned(32)))
u8 trbMemSize[TRB_POOL_SIZE];

CUSBD_Config configDriver = {
    .regBase = REG_BASE_BLK_MCU_USB, // address where USB core is mapped
    .eb_base = REG_CKM_EN_GLB_MCU_MCU_USB_EB,
    .eb = BIT_CKM_EN_GLB_MCU_MCU_USB_EB,
    .epIN =
    {
        {
           .bufferingValue = EP0IN_BUFFER,
           .maxPacketSize = EP0IN_MAX_PACKET_SIZE,
           .startBuf = EP0IN_STARTADDR
        }, // ep0in
        {
           .bufferingValue = EP1IN_BUFFER,
           .maxPacketSize = EP1IN_MAX_PACKET_SIZE,
           .startBuf = EP1IN_STARTADDR
        }, // ep1in
    },
    .epOUT =
    {
        {
            .bufferingValue = EP0OUT_BUFFER,
            .maxPacketSize = EP0OUT_MAX_PACKET_SIZE,
            .startBuf = EP0OUT_STARTADDR
        }, //ep0out
        {
            .bufferingValue = EP1OUT_BUFFER,
            .maxPacketSize = EP1OUT_MAX_PACKET_SIZE,
            .startBuf = EP1OUT_STARTADDR
        }, //ep1out
    },
    .dmultEnabled = 1,
    .dmaSupport = 1,
    .trbAddr = (void*)trbMemSize,
    .trbDmaAddr = (uintptr_t)trbMemSize,
};

#endif  /* __USB_HW_CFG_H__ */
