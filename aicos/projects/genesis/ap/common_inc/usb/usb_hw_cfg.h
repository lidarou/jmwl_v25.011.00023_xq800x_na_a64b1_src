/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

#ifndef __USB_HW_CFG_H__
#define __USB_HW_CFG_H__

#include <aic_auto_reg.h>
#include <reg_base.h>
#include <usb/usb_buf_cfg.h>
#include <usb_device.h>

#define USB_ANA_BASE_ADDR 0x400c0618
#define USB_PHY_SOURCE_CLK_PARENT_EN     CLK_UPLL_EN
#define USB_PHY_SOURCE_CLK_EN            CLK_UPLL_24M_EN
#define USB_ENDPOINT_NUM                 9

__attribute__((section(".usb.data"), aligned(32)))
u8 trbMemSize[TRB_POOL_SIZE];

CUSBD_Config configDriver = {
    .regBase = REG_BASE_AP_USB20,
    .eb_base = REG_MCU_GLB_USB_EB,
    .eb = BIT_MCU_GLB_USB_EB,
    .irq = IRQ_USB,
    .rst = REG_MCU_GLB_SW_RST_GLB_CLK_MCU_USB,
    .rsteb = BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_USB,
    .usb_ana_base = USB_ANA_BASE_ADDR,
    .wakeup_irq = IRQ_USBWU,
    .epIN =
    {
        {.bufferingValue = EP0IN_BUFFER,  .maxPacketSize = EP0IN_MAX_PACKET_SIZE,  .startBuf = EP0IN_STARTADDR }, // ep0in
        {.bufferingValue = EP1IN_BUFFER,  .maxPacketSize = EP1IN_MAX_PACKET_SIZE,  .startBuf = EP1IN_STARTADDR }, // ep1in
        {.bufferingValue = EP2IN_BUFFER,  .maxPacketSize = EP2IN_MAX_PACKET_SIZE,  .startBuf = EP2IN_STARTADDR }, // ep2in
        {.bufferingValue = EP3IN_BUFFER,  .maxPacketSize = EP3IN_MAX_PACKET_SIZE,  .startBuf = EP3IN_STARTADDR }, // ep3in
        {.bufferingValue = EP4IN_BUFFER,  .maxPacketSize = EP4IN_MAX_PACKET_SIZE,  .startBuf = EP4IN_STARTADDR }, // ep4in
        {.bufferingValue = EP5IN_BUFFER,  .maxPacketSize = EP5IN_MAX_PACKET_SIZE,  .startBuf = EP5IN_STARTADDR }, // ep5in
        {.bufferingValue = EP6IN_BUFFER,  .maxPacketSize = EP6IN_MAX_PACKET_SIZE,  .startBuf = EP6IN_STARTADDR }, // ep6in
        {.bufferingValue = EP7IN_BUFFER,  .maxPacketSize = EP7IN_MAX_PACKET_SIZE,  .startBuf = EP7IN_STARTADDR }, // ep7in
        {.bufferingValue = EP8IN_BUFFER,  .maxPacketSize = EP8IN_MAX_PACKET_SIZE,  .startBuf = EP8IN_STARTADDR }, // ep8in
    },
    .epOUT =
    {
        {.bufferingValue = EP0OUT_BUFFER,  .maxPacketSize = EP0OUT_MAX_PACKET_SIZE,  .startBuf = EP0OUT_STARTADDR }, //ep0out
        {.bufferingValue = EP1OUT_BUFFER,  .maxPacketSize = EP1OUT_MAX_PACKET_SIZE,  .startBuf = EP1OUT_STARTADDR }, //ep1out
        {.bufferingValue = EP2OUT_BUFFER,  .maxPacketSize = EP2OUT_MAX_PACKET_SIZE,  .startBuf = EP2OUT_STARTADDR }, //ep2out
        {.bufferingValue = EP3OUT_BUFFER,  .maxPacketSize = EP3OUT_MAX_PACKET_SIZE,  .startBuf = EP3OUT_STARTADDR }, //ep3out
        {.bufferingValue = EP4OUT_BUFFER,  .maxPacketSize = EP4OUT_MAX_PACKET_SIZE,  .startBuf = EP4OUT_STARTADDR }, //ep4out
        {.bufferingValue = EP5OUT_BUFFER,  .maxPacketSize = EP5OUT_MAX_PACKET_SIZE,  .startBuf = EP5OUT_STARTADDR }, //ep5out
        {.bufferingValue = EP6OUT_BUFFER,  .maxPacketSize = EP6OUT_MAX_PACKET_SIZE,  .startBuf = EP6OUT_STARTADDR }, //ep6out
        {.bufferingValue = EP7OUT_BUFFER,  .maxPacketSize = EP7OUT_MAX_PACKET_SIZE,  .startBuf = EP7OUT_STARTADDR }, //ep7out
        {.bufferingValue = EP8OUT_BUFFER,  .maxPacketSize = EP8OUT_MAX_PACKET_SIZE,  .startBuf = EP8OUT_STARTADDR }, //ep8out
    },
    .dmultEnabled = 1,
    .trbAddr = (void*)trbMemSize,
    .trbDmaAddr = (uintptr_t)trbMemSize,
};

#endif  /* __USB_HW_CFG_H__ */
