/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ed357
#define LOG_MODULE    AMOD_USB

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_bitmap.h>
#include <aic_clk_map.h>
#include <aic_iot_clk.h>
#include <aic_osa.h>
#include <cache.h>
#include <irq.h>
#include <core_star.h>
#include <usb_dma.h>
#include <usb_device.h>
#include <usb/usb_hw_cfg.h>
#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>
#include <timer_inc.h>
#include <usb_service.h>

#define USB_SPEED_DETECT_TIME        (2)
#define USB_PHY_RESET_UDELAY            2000

UDC_Device aic_udc;

static ISR_RET_E usb_wakeup_irq(void * privateData)
{
    UDC_Device *au = (UDC_Device*) privateData;

    NVIC_DisableIRQ(configDriver.wakeup_irq);
    writeb_relaxed(&au->regs->cpuctrl, BIT_0);
    au->ustatus = UDC_RESUME;
    USB_AIDXPF(0x1bfdc7, "", "usb resume\n");
    return NONE_WORKER;
}

static void usb_suspend_enable(UDC_Device *au, int enable)
{
    AOS_INTERRUPT_DECLARE
    UCRegs * regs;
    uint8_t reg;

    if (!au)
        return;

    AOS_INTERRUPT_DISABLE
    regs = au->regs;
    reg = readb_relaxed(&regs->usbien);
    if (enable)
        reg |= UCM_USBIR_SUSP;
    else
        reg &= ~UCM_USBIR_SUSP;
    writeb_relaxed(&regs->usbien, reg);
    AOS_INTERRUPT_RESTORE
}

/**
 *  Function notifies the host that it should resume from its suspended state.
 */
static void usb_resume(UDC_Device * au)
{
    AOS_INTERRUPT_DECLARE
    uint8_t usbcs;

    USB_AIDXPF(0x102eea, "", "remote wake-up\n");

    AOS_INTERRUPT_DISABLE
    au->power_disable_depth++;
    if (au->power_disable_depth > 1){
       AOS_INTERRUPT_RESTORE
       return;
    }
    writeb_relaxed(&au->regs->cpuctrl, BIT_0);
    usb_suspend_enable(au, 0);

    if (!_ux_system_slave->ux_system_slave_remote_wakeup_capability ||
        (!_ux_system_slave->ux_system_slave_remote_wakeup_enabled)) {
        AOS_INTERRUPT_RESTORE
        return;
    }
    if (au->ustatus) {
        usbcs = readb_relaxed(&au->regs->usbcs);
        usbcs |= UCM_USBCS_SIGRSUME;
        writeb_relaxed(&au->regs->usbcs, usbcs);
    }
    AOS_INTERRUPT_RESTORE
}

static void usb_put_autosuspend(UDC_Device * au)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (!au->power_disable_depth) {
        AOS_INTERRUPT_RESTORE
        return;
    }

    au->power_disable_depth--;
    if (au->power_disable_depth){
       AOS_INTERRUPT_RESTORE
       return;
    }
    usb_suspend_enable(au, 1);
    AOS_INTERRUPT_RESTORE
}

static void usb_suspend(UDC_Device * au)
{
    if (au->ustatus == UDC_SUSPEND) {
        USB_AIDXPF(0x03e955, "", "usb suspend already.\n");
        return;
    }

    writeb_relaxed(&au->regs->usbirq, UCM_USBIR_SUSP);
    writeb_relaxed(&au->regs->clkgate, 1);
    writeb_relaxed(&au->regs->cpuctrl, 0);
    au->ustatus = UDC_SUSPEND;
    NVIC_ClearPendingIRQ(configDriver.wakeup_irq);
    NVIC_EnableIRQ(configDriver.wakeup_irq);
}

void usb_driver_get_mutex(void)
{
    UDC_Device *au = &aic_udc;
    uint32_t ret;

    ret = aos_get_mutex(au->mutex, AOS_WAIT_FOREVER);
    AICX_ASSERT_IFX(!ret, 0x01452b, "x", "USB driver send get mutex fail! ret = 0x%x", ret);
}

void usb_driver_put_mutex(void)
{
    UDC_Device *au = &aic_udc;

    aos_put_mutex(au->mutex);
}


uint32_t usb_EpXDataReceive(UX_SLAVE_ENDPOINT *ep,
                        uint8_t *buffer, uint32_t length)
{
    uint32_t num =ep->ux_slave_endpoint_descriptor.bEndpointAddress & 0xf;
    UDC_Device *au = &aic_udc;
    uint32_t index = num << 1;
    CUSBDMA_Status status;
    uint8_t epType;
    uint32_t ret;

    epType = ep->ux_slave_endpoint_descriptor.bmAttributes &
            UX_MASK_ENDPOINT_TYPE;

    usb_resume(au);
    usb_driver_get_mutex();
    status = usb_dma_get_channel_status(&au->dma, au->channel[index]);
    if((status >= CUSBDMA_STATUS_BUSY && !au->cfg.dmultEnabled) ||
        (status >= CUSBDMA_STATUS_BUSY
        &&  (epType != UX_ISOCHRONOUS_ENDPOINT)))   {
        USB_AIDXPF(0x111907, "d", "outdata transfer for ep%d is pending now\n", num);
        AICX_ASSERT_IFX(0, 0x1e1e9b, "", "Multiple transfer are not allowed except in ISO.");
        usb_driver_put_mutex();
        usb_put_autosuspend(au);
        return AIC_EIO;
    }

    dcache_inv_by_range(DCLINE_ALIGN(buffer), DCLINE_GRANULARITY(buffer, length));
    ret = usb_dma_channel_program(&au->dma, au->channel[index],
                ep->ux_slave_endpoint_descriptor.wMaxPacketSize,
                (uint32_t)buffer, length, NULL, 0);
    usb_driver_put_mutex();
    usb_put_autosuspend(au);
    return ret;
}

uint32_t usb_EpXDataSend(UX_SLAVE_ENDPOINT *ep,
                     uint8_t *buffer, uint32_t length)
{
    uint32_t num =ep->ux_slave_endpoint_descriptor.bEndpointAddress & 0xf;
    UDC_Device *au = &aic_udc;
    CUSBDMA_Status status;
    uint32_t index = (num << 1) + 1;
    uint8_t epType;
    uint32_t ret;

    /*start critical section*/
    epType = ep->ux_slave_endpoint_descriptor.bmAttributes &
            UX_MASK_ENDPOINT_TYPE;

    usb_resume(au);
    usb_driver_get_mutex();
    status = usb_dma_get_channel_status(&au->dma, au->channel[index]);
    if((status >= CUSBDMA_STATUS_BUSY && !au->cfg.dmultEnabled) ||
        (status >= CUSBDMA_STATUS_BUSY &&
        (epType != UX_ISOCHRONOUS_ENDPOINT)))    {
        USB_AIDXPF(0x0d95cc, "dx", "indata transfer for ep%d  is pending now, channelStatus = 0x%x\n",
            num, status);
        AICX_ASSERT_IFX(0, 0x1e1e9b, "", "Multiple transfer are not allowed except in ISO.");
        usb_driver_put_mutex();
        usb_put_autosuspend(au);
        return AIC_EIO;
    }

    dcache_clean_by_range((int32_t)buffer, length);
    ret = usb_dma_channel_program(&au->dma, au->channel[index],
            ep->ux_slave_endpoint_descriptor.wMaxPacketSize,
            (uint32_t)buffer, length, NULL, 0);
    usb_driver_put_mutex();
    usb_put_autosuspend(au);
    return ret;
}

void usb_abort_endpoint(uint8_t num, uint8_t isInEp)
{
    UDC_Device *au = &aic_udc;
    uint32_t index;
    AOS_INTERRUPT_DECLARE

     usb_resume(au);
    USB_AIDXPF(0x0249d0, "dd", "Abort Device endpoint: %d, isInEp = %d\n", num, isInEp);
    index = (num << 1) + isInEp;
    AOS_INTERRUPT_DISABLE
    /*For endpoint different than 0 firmware has to release DMA Channel*/
    if (au->channel[index])
        usb_dma_channel_abort(&au->dma, au->channel[index]);
    AOS_INTERRUPT_RESTORE
    usb_put_autosuspend(au);
}

void usb_release_endpoint(uint8_t num, uint8_t isInEp)
{
    UDC_Device *au = &aic_udc;
    uint32_t index;

    usb_resume(au);
    USB_AIDXPF(0x010c44, "dd", "release Device endpoint: %d, isInEp = %d\n", num, isInEp);
    index = (num << 1) + isInEp;
    /*For endpoint different than 0 firmware has to release DMA Channel*/
    if (au->channel[index]) {
        usb_dma_channel_release(&au->dma, au->channel[index]);
        au->channel[index] = NULL;
    }
    usb_put_autosuspend(au);
}

static void usb_ep0_reset(void)
{
    UX_DCD_AIC *dcd_aic;
    UX_DCD_AIC_ED *ed;
    UX_SLAVE_DCD *dcd;

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave->ux_system_slave_dcd;

    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    /* Fetch the address of the physical endpoint. */
    ed = &dcd_aic->ux_dcd_aic_ed[0];

    ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_IDLE;
}

static int usb_ep0_enable(UDC_Device *au, uint8_t num, uint8_t isInEp)
{
    uint8_t chan_index = (num << 1) + isInEp ;
    UCRegs *regs = au->regs;
    uint16_t tmp;

    if (!isInEp) {
        tmp = readw_relaxed(&regs->rxien);
        /*disable out irq for Ep*/
        tmp &= ~(1 << num);
        writew_relaxed(&regs->rxien, tmp);
        writeb_relaxed(&regs->ep0fifoctrl,
            UCM_FIFOCTRL_IO_TX | UCM_FIFOCTRL_FIFOAUTO);
    } else {
        tmp = readw_relaxed(&regs->txien);
        /*disable out irq for Ep*/
        tmp &= ~(1 << num);
        writew_relaxed(&regs->txien, tmp);
        writeb_relaxed(&regs->ep0fifoctrl, UCM_FIFOCTRL_FIFOAUTO );
    }

    au->channel[chan_index] = usb_dma_channel_alloc(&au->dma,
            isInEp, num, 0);
    if(!au->channel[chan_index] ) {
        USB_AIDXPF(0x038763, "", "Error: Dma Channel not allocated\n");
        return AIC_EIO;
    }
    writeb_relaxed(&regs->ep0maxpack, 0x40);
    return AIC_SUCCESS;
}

static int usb_epX_enable(UDC_Device *au, uint8_t num,
                          usb_device_endpoint_init_struct_t *epInit,
                          uint8_t isInEp)
{
    uint8_t chan_index = (num << 1) + isInEp;
    uint16_t tmp = epInit->maxPacketSize;
    UCRegs *regs = au->regs;
    uint8_t type, reg, iso = 0;
    uint32_t payload;

    if (!(tmp & 0x07ff))
        return AIC_EINVAL;

    payload = tmp & 0x07ff;
    switch (epInit->transferType) {
    case UX_ISOCHRONOUS_ENDPOINT:
        type = UCM_CON_TYPE_ISOC;
        switch (tmp >> 11) {
        case 0:
            iso = UCM_CON_1_ISOD;
            break;
        case 1:
            payload *=2;
            iso = UCM_CON_2_ISOD;
            break;
        case 2:
            payload *=3;
            iso = UCM_CON_3_ISOD;
            break;
        }
        break;
    case UX_BULK_ENDPOINT:
        type = UCM_CON_TYPE_BULK;
        break;
    case UX_INTERRUPT_ENDPOINT:
        type = UCM_CON_TYPE_INT;
        break;
    default:
        type = epInit->transferType;
    }

    tmp &= 0x07ff;
    if (isInEp) {
        if (tmp > au->cfg.epIN[num].maxPacketSize)
            return AIC_EINVAL;

        writew_relaxed(&regs->txmaxpack[num - 1], tmp);
        writeb_relaxed(&regs->ep[num - 1].txcon,
                UCM_CON_VAL | type | iso
                | (au->cfg.epIN[num].bufferingValue - 1));

        writeb_relaxed(&regs->fifoctrl,
                (UCM_FIFOCTRL_FIFOAUTO | UCM_FIFOCTRL_IO_TX | num));

        /*Reset Endpoint (fifo and toggle bit*/
        writeb_relaxed(&regs->endprst, UCM_ENDPRST_IO_TX | num);
        writeb_relaxed(&regs->endprst,
                UCM_ENDPRST_IO_TX | num | UCM_ENDPRST_FIFORST
                | UCM_ENDPRST_TOGRST);
    } else {
        if (tmp > au->cfg.epOUT[num].maxPacketSize)
            return AIC_EINVAL;

        writew_relaxed(&regs->rxmaxpack[num - 1], tmp);
        writeb_relaxed(&regs->ep[num - 1].rxcon,
                UCM_CON_VAL | type | iso
                | (au->cfg.epIN[num].bufferingValue - 1));

        writeb_relaxed(&au->regs->fifoctrl,
                (UCM_FIFOCTRL_FIFOAUTO | num));

        /*Reset Endpoint (fifo and toggle bit*/
        writeb_relaxed(&regs->endprst, num);
        writeb_relaxed(&regs->endprst, num |
            UCM_ENDPRST_FIFORST | UCM_ENDPRST_TOGRST);
    }

    au->channel[chan_index] = usb_dma_channel_alloc(&au->dma,
            isInEp, num, (type == UCM_CON_TYPE_ISOC)? 1 : 0);

    if(type == UCM_CON_TYPE_ISOC) {
        if(!au->cfg.dmultEnabled) {
            reg = readb_relaxed(&regs->usbien);
            reg |= UCM_USBIR_SOF;
            /*Enable interrupt for sof. In each SOF Irq software
         * call busInterval callback*/
            writeb_relaxed(&regs->usbien, reg);
        }

        if(isInEp) {
            writew_relaxed(&regs->isoautodump, 1 << num);
            writew_relaxed(&regs->isodctrl, 1 << num);
        }
        usb_dma_set_max_length(&au->dma, au->channel[chan_index], payload);
    }

    return AIC_SUCCESS;
}

static void
usb_ep0_stall_set(UDC_Device * au, UX_SLAVE_ENDPOINT *ep, uint8_t stall)
{
    uint32_t num, isInEp;
    uint8_t ep0cs;

    USB_AIDXPF(0x129cc3, "x", "Disable Endpoint 0x%x \n",
        ep->ux_slave_endpoint_descriptor.bEndpointAddress);

    num = ep->ux_slave_endpoint_descriptor.bEndpointAddress & 0x0f;
    isInEp = ep->ux_slave_endpoint_descriptor.bEndpointAddress &
        UX_ENDPOINT_DIRECTION ? 1 : 0;

    ep0cs = readb_relaxed(&au->regs->ep0cs);
    if (stall)
       writeb_relaxed(&au->regs->ep0cs, ep0cs | UCDM_EP0CS_STALL);
    else
       writeb_relaxed(&au->regs->ep0cs, ep0cs & ~UCDM_EP0CS_STALL);
}

static void
usb_epX_stall_set(UDC_Device * au, UX_SLAVE_ENDPOINT *ep, uint8_t stall)
{
    uint8_t isInEp, tmp =0;
    uint32_t num;

    num = ep->ux_slave_endpoint_descriptor.bEndpointAddress & 0x0f;
    USB_AIDXPF(0x129cc3, "x", "Disable Endpoint 0x%x \n",
        ep->ux_slave_endpoint_descriptor.bEndpointAddress);

    /* start critical section */
    tmp = ep->ux_slave_endpoint_descriptor.bmAttributes &
       UX_MASK_ENDPOINT_TYPE;
    isInEp = ep->ux_slave_endpoint_descriptor.bEndpointAddress &
        UX_ENDPOINT_DIRECTION ? 1 : 0;

    /* Disable interrupt for sof. SOF IRQ should be enabled only for
     * ISOC endpoints */
    if(tmp == UX_ISOCHRONOUS_ENDPOINT && !au->cfg.dmultEnabled) {
        tmp =  readb_relaxed(&au->regs->usbien);
        tmp &= ~UCM_USBIR_SOF;
        /* Disable interrupt for sof. SOF IRQ should be enabled only
     * for ISOC endpoints */
        writeb_relaxed(&au->regs->usbien, tmp);
    }

    if (isInEp) {
        tmp = readb_relaxed(&au->regs->ep[num - 1].txcon);
        tmp = stall ? (tmp | UCDM_CON_STALL) : (tmp & ~UCDM_CON_STALL);
        writeb_relaxed(&au->regs->ep[num - 1].txcon, tmp);
    } else {
        tmp = readb_relaxed(&au->regs->ep[num - 1].rxcon);
        tmp = stall ? (tmp | UCDM_CON_STALL) : (tmp & ~UCDM_CON_STALL);
        writeb_relaxed(&au->regs->ep[num - 1].rxcon, tmp);
    }
}

void usb_endpoint_unstall(UDC_Device * au, UX_SLAVE_ENDPOINT *endpoint)
{
    uint32_t num;

    usb_resume(au);
    num = endpoint->ux_slave_endpoint_descriptor.bEndpointAddress & 0x0f;
    if (num)
        usb_epX_stall_set(au, endpoint, 0);
    else
        usb_ep0_stall_set(au, endpoint, 0);
    usb_put_autosuspend(au);
}

void usb_endpoint_stall(UDC_Device * au, UX_SLAVE_ENDPOINT *endpoint)
{
    uint32_t num;

    usb_resume(au);
    num = endpoint->ux_slave_endpoint_descriptor.bEndpointAddress & 0x0f;
    if (num)
        usb_epX_stall_set(au, endpoint, 1);
    else
        usb_ep0_stall_set(au, endpoint, 1);
    usb_put_autosuspend(au);
}

void usb_endpoint_create(UDC_Device *au,
                         usb_device_endpoint_init_struct_t *epInit)
{
   uint8_t isInEp = epInit->endpointAddress & UX_ENDPOINT_DIRECTION ? 1 : 0;
   uint8_t num = epInit->endpointAddress & 0xf;

   usb_resume(au);
   usb_release_endpoint(num, isInEp);
   if (epInit->endpointAddress & 0x0f)
     usb_epX_enable(au, num, epInit, isInEp);
   else
     usb_ep0_enable(au, num, isInEp);
   usb_put_autosuspend(au);
}

static void usb_clear_ep_status(uint8_t num, uint8_t isInEp)
{
    UX_DCD_AIC *dcd_aic;
    UX_DCD_AIC_ED *ed;
    UX_SLAVE_DCD *dcd;
    uint8_t index = (num << 1) + isInEp;

    dcd =  &_ux_system_slave->ux_system_slave_dcd;
    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;
    ed = dcd_aic->ux_dcd_aic_ed;
    ed[index].ux_dcd_aic_ed_status = UX_DCD_AIC_ED_STATUS_UNUSED;
}

static void usb_stop_activity(UDC_Device *au)
{
    uint8_t i;

    USB_AIDXPF(0x050d4e, "", "USB Stop Activity\n");

    for (i = 0; i < 8; i++) {
        usb_clear_ep_status(i, 0);
        usb_clear_ep_status(i, 1);
        if (!i) {
            usb_release_endpoint(i, 0);
            usb_release_endpoint(i, 1);
            usb_ep0_reset();
            continue;
        }
        usb_release_endpoint(i, 0);
        usb_release_endpoint(i, 1);
    }
}

/**
 * Gets actual speed in that controller works
 */
static uint8_t usb_get_actual_speed(UDC_Device *au)
{
    uint8_t speed = UCM_SPEEDCTRL_FS;

    speed = readb_relaxed(&au->regs->speedctrl)
                 & ~UCM_SPEEDCTRL_HSDISABLE;

    switch(speed) {
    case UCM_SPEEDCTRL_HS:
        return UX_HIGH_SPEED_DEVICE;
    case UCM_SPEEDCTRL_FS:
        return UX_FULL_SPEED_DEVICE;
    default:
        return UX_FULL_SPEED_DEVICE;
    }
}

static void usb_enter_high_speed(UDC_Device *au)
{
    USB_AIDXPF(0x143cee, "", "usb enter high speed\n");
    _ux_system_slave->ux_system_slave_speed = usb_get_actual_speed(au);
    _ux_dcd_aic_initialize_complete();
}

static void usb_reset(UDC_Device *au)
{
    aicx_info(0x0032c3, "", "usb_reset \n");

    if (configDriver.enum_timeout)
        usb_service_enum_timer_deactive();

    usb_resume(au);
    ux_device_stack_disconnect();
    usb_stop_activity(au);
    _ux_system_slave->ux_system_slave_speed = UX_FULL_SPEED_DEVICE;
    /* Complete the device initialization.  */
    _ux_dcd_aic_initialize_complete();
    /* Mark the device as attached now.  */
    _ux_system_slave->ux_system_slave_device.ux_slave_device_state
            = UX_DEVICE_ATTACHED;
    usb_put_autosuspend(au);
}

static void usb_ep0_setup_irq_enable(UDC_Device *au, u32 enable)
{
    AOS_INTERRUPT_DECLARE
    uint8_t reg;

    AOS_INTERRUPT_DISABLE
    reg = readb_relaxed(&au->regs->usbien);
    if (enable)
        reg |= UCDM_USBIR_SUDAV;
    else
        reg &= ~UCDM_USBIR_SUDAV;
    writeb_relaxed(&au->regs->usbien, reg);
    AOS_INTERRUPT_RESTORE
}
void usb_ep0_data_callback(void)
{
    UX_DCD_AIC *dcd_aic;
    UX_DCD_AIC_ED *ed;
    UX_SLAVE_DCD *dcd;
    UX_SLAVE_TRANSFER *req;
    uint32_t status;
    UDC_Device *au = &aic_udc;

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave->ux_system_slave_dcd;

    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    /* Fetch the address of the physical endpoint. */
    ed = &dcd_aic->ux_dcd_aic_ed[0];

    switch (ed->ux_dcd_aic_ed_state) {
    case UX_DCD_AIC_ED_STATE_DATA_TX:
        USB_AIDXPF(0x1e8318, "d", "DATA Stage IN len = %d\n", usb_get_actual_length(0x80));
        break;
    case UX_DCD_AIC_ED_STATE_DATA_RX:
        USB_AIDXPF(0x069f7a, "d", "DATA Stage Out len = %d\n", usb_get_actual_length(0x0));
        req = &ed->ux_dcd_aic_ed_endpoint->ux_slave_endpoint_transfer_request;
        req->ux_slave_transfer_request_current_data_pointer =
        req->ux_slave_transfer_request_data_pointer;
        status = _ux_device_stack_control_request_process(req);
        if (status)
           USB_AIDXPF(0x08244f, "d", "control request process fail. status =%d\n", status);
        break;
    }
    usb_ep0_setup_irq_enable(au, 1);

    ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_IDLE;
    writeb_relaxed(&aic_udc.regs->ep0cs, UCDM_EP0CS_HSNAK);
}

void usb_setup_handler(void *data)
{
    UDC_Device *au = (UDC_Device *)data;
    UX_SLAVE_TRANSFER *req;
    UX_SLAVE_ENDPOINT *endpoint;
    UX_DCD_AIC *dcd_aic;
    UX_DCD_AIC_ED *ed;
    UX_SLAVE_DCD *dcd;
    UINT status;

    usb_resume(au);
    if (au->irq_type) {
        /* Reset all but not release channel DMA for EP0 */
        usb_reset(au);
        au->irq_type = IRQ_EP0;
        usb_put_autosuspend(au);
        return;
    }

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave->ux_system_slave_dcd;

    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    /* Fetch the address of the physical endpoint. */
    ed = &dcd_aic->ux_dcd_aic_ed[0];

    /* Get the pointer to the transfer request. */
    req = &ed->ux_dcd_aic_ed_endpoint->ux_slave_endpoint_transfer_request;

    /* Copy setup data to transfer request. */
    _ux_utility_memory_copy(req->ux_slave_transfer_request_setup,
                au->setupdata, UX_SETUP_SIZE);

    /* Clear the length of the data received. */
    req->ux_slave_transfer_request_actual_length = 0;

    /* Mark the phase as SETUP. */
    req->ux_slave_transfer_request_type = UX_TRANSFER_PHASE_SETUP;

    /* Mark the transfer as successful. */
    req->ux_slave_transfer_request_completion_code = UX_SUCCESS;

    /* Set the status of the endpoint to not stalled.  */
    ed->ux_dcd_aic_ed_status &= ~UX_DCD_AIC_ED_STATUS_STALLED;

    /* Check if the transaction is IN. */
    if (*req->ux_slave_transfer_request_setup & UX_REQUEST_IN) {
        /* The endpoint is IN.  This is important to memorize the
           direction for the control endpoint
           in case of a STALL. */
        ed->ux_dcd_aic_ed_direction = UX_ENDPOINT_IN;
        ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_DATA_TX;
        /* Call the Control Transfer dispatcher.  */
        status = _ux_device_stack_control_request_process(req);
        if (status) {
            USB_AIDXPF(0x1a8cd5, "d", "usb_setup_handler. status = %d\n", status);
            usb_ep0_setup_irq_enable(au, 1);
        }
    } else {
        /* The endpoint is OUT.  This is important to memorize the
           direction for the control endpoint
           in case of a STALL. */
        ed->ux_dcd_aic_ed_direction = UX_ENDPOINT_OUT;

        /* We are in a OUT transaction. Check if there is
           a data payload. If so, wait for the payload
           to be delivered.  */
        if (*(req->ux_slave_transfer_request_setup + 6) == 0 &&
            *(req->ux_slave_transfer_request_setup + 7) == 0) {

            ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_STATUS_RX;
            usb_ep0_data_callback();
            /* Call the Control Transfer dispatcher. */
            status =_ux_device_stack_control_request_process(req);
            if (status) {
                USB_AIDXPF(0x1a8cd5, "d", "usb_setup_handler. status = %d\n", status);
                usb_ep0_setup_irq_enable(au, 1);
            }
        } else {
            /* Get the pointer to the logical endpoint from the transfer request. */
            endpoint = req->ux_slave_transfer_request_endpoint;

            /* Get the length we expect from the SETUP packet. */
            req->ux_slave_transfer_request_requested_length =
            _ux_utility_short_get(
                req->ux_slave_transfer_request_setup + 6);

            /* Check if we have enough space for the request. */
            if (req->ux_slave_transfer_request_requested_length >
                UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH) {

                /* No space available, stall the endpoint. */
                _ux_dcd_aic_endpoint_stall(dcd_aic, endpoint);

                /* Next phase is a SETUP. */
                ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_IDLE;
                usb_ep0_setup_irq_enable(au, 1);
                usb_put_autosuspend(au);
                return;
            } else {
                /* Reset what we have received so far. */
                req->ux_slave_transfer_request_actual_length = 0;
                /* And reprogram the current buffer address to the beginning of the buffer. */
                req->ux_slave_transfer_request_current_data_pointer =
                    req->ux_slave_transfer_request_data_pointer;
                ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_DATA_RX;
                writeb_relaxed(&aic_udc.regs->ep0Rxbc, 0);
                /* Receive data. */
                usb_EpXDataReceive(endpoint,
                    req->ux_slave_transfer_request_current_data_pointer,
                    req->ux_slave_transfer_request_requested_length);
            }
        }
    }
    usb_put_autosuspend(au);
}

static ISR_RET_E usb_ep0_irq(UDC_Device *au)
{
    UCRegs *regs = au->regs;
    int8_t i;

    /* Get the pointer to the DCD.  */
    /* Get the pointer to the AIC DCD.  */
    /* Fetch the address of the physical endpoint. */

    usb_resume(au);
    writeb_relaxed(&regs->ep0cs, UCDM_EP0CS_CHGSET);
    /*Read Setup packet from USB registers*/
    for (i = 0; i < UX_SETUP_SIZE; i++)
        au->setupdata[i] = readb_relaxed(&regs->setupdat[i]);
    i = readb_relaxed(&regs->ep0cs);
    if (i & UCDM_EP0CS_CHGSET) {
        USB_AIDXPF(0x13ea38, "s", "Setup Flags Chang: Not Error %s\n", " ");
        /*Setup packet has been modified during reading.
         * Probably last Setup Request was SET_ADDRESS that
         * was acknowledgment by hardware
         * In this case function return ECANCELED but it is not error;
         **/
         usb_put_autosuspend(au);
         return NONE_WORKER;
    }
    usb_ep0_setup_irq_enable(au, 0);
    writeb_relaxed(&regs->usbirq, UCDM_USBIR_SUDAV);
    for (i = 0; i < UX_SETUP_SIZE; i++)
        USB_AIDXPF(0x167e4f, "dx", "Setup[%d] = 0x%x\n", i, au->setupdata[i]);
    usb_put_autosuspend(au);
    return CALL_LWORKER;
}

static ISR_RET_E usb_udc_irq_handle(void *privateData)
{
    UDC_Device *au = (UDC_Device*) privateData;
    UCRegs *regs = au->regs;
    uint8_t usbIrqV, usbIrqM;
    ISR_RET_E ret;

    writeb_relaxed(&au->regs->cpuctrl, BIT_0);
    usbIrqV = readb_relaxed(&regs->usbirq);
    usbIrqM = readb_relaxed(&regs->usbien);

    usb_resume(au);
    usbIrqV = usbIrqV & usbIrqM;
    USB_AIDXPF(0x01fd88, "x", "SFR (usbirq & usbien) =  %02x\n", usbIrqV);
    if (usbIrqV & UCDM_USBIR_LPMIR) {
        USB_AIDXPF(0x1b3701, "", "USBIRQ: LPM\n");
        /* Clear the NYET bit */
        uint8_t usbcs = readb_relaxed(&regs->usbcs);
        usbcs &= ~UCDM_USBCS_LPMNYET;
        writeb_relaxed(&regs->usbcs, usbcs);
        writeb_relaxed(&regs->usbirq, UCDM_USBIR_LPMIR);
    }

    if (usbIrqV & UCM_USBIR_URES) {
        USB_AIDXPF(0x1dbc72, "", "USBIRQ: RESET\n");
        /* Clearing interrupt bits */
        writeb_relaxed(&regs->usbirq, UCM_USBIR_URES);
        au->irq_type = IRQ_RESET;
        usb_put_autosuspend(au);
        return CALL_LWORKER;
    }

    /*This interrupt is not handled by firmware because Speed is checking by speedctrl SFR*/
    if (usbIrqV & UCM_USBIR_HSPPED) {
        USB_AIDXPF(0x103b9e, "", "USBIRQ: HighSpeed\n");
        writeb_relaxed(&regs->usbirq, UCM_USBIR_HSPPED);
        usb_enter_high_speed(au);
    }

    if (usbIrqV & UCDM_USBIR_SUDAV) { /* setup packet interrupt */
        USB_AIDXPF(0x025c99, "", "USBIRQ: SUDAV\n");
        ret = usb_ep0_irq(au);
        if (ret) {
            usb_put_autosuspend(au);
            return ret;
        }
    }

    /* SOF interrupt */
    if (usbIrqV & UCM_USBIR_SOF) {
        USB_AIDXPF(0x077697, "", "USBIRQ: SOF\n");
        writeb_relaxed(&regs->usbirq, UCM_USBIR_SOF);
    }

    if (usbIrqV & UCDM_USBIR_SUTOK) {
        USB_AIDXPF(0x19747f, "", "USBIRQ: SUTOK\n", " ");
        writeb_relaxed(&regs->usbirq, UCDM_USBIR_SUTOK);
    }

    USB_AIDXPF(0x044bc3, "", "Forward ISR to CUSBDMA ISR\n", " ");
    usb_dma_isr(&au->dma);
    usb_put_autosuspend(au);

    if (usbIrqV & UCM_USBIR_SUSP) {
            writeb_relaxed(&regs->usbirq, UCM_USBIR_SUSP);
            usb_suspend(au);
            USB_AIDXPF(0x1c3b8c, "", "USBIRQ: SUSPEND\n");
    }
    return NONE_WORKER;
}

static void usb_init_device_ep(UDC_Device * au, uint8_t epNum,
                              uint8_t isInEp)
{
    USB_AIDXPF(0x02bdb5, "ds", "Init Device endpoint%d, %s\n", epNum, isInEp ? "in" : "out");
    if (!epNum)
        return;

    if (isInEp) {
        if (au->cfg.epIN[epNum].startBuf != 0)
            writew_relaxed(&au->regs->txstaddr[epNum - 1].addr,
                    au->cfg.epIN[epNum].startBuf);
        writeb_relaxed(&au->regs->ep[epNum - 1].txcon, 0x0);
    } else {
        if (au->cfg.epOUT[epNum].startBuf != 0)
            writew_relaxed(&au->regs->rxstaddr[epNum - 1].addr,
                    au->cfg.epOUT[epNum].startBuf);
        writeb_relaxed(&au->regs->ep[epNum - 1].rxcon, 0x0);
    }
    au->endpointInList++;
}

static void usb_init_endpoints(UDC_Device *au)
{
    CUSBD_Config * config = &au->cfg;
    uint8_t num;

    for (num = 0; num < USB_ENDPOINT_NUM; num++) {
        if (!num) {
            usb_init_device_ep(au, num, 1); /*EP0 IN*/
            usb_init_device_ep(au, num, 0); /*EP0 OUT*/
            continue;
        }

        if (config->epIN[num].bufferingValue)
            usb_init_device_ep(au, num, 1);

        if (config->epOUT[num].bufferingValue)
            usb_init_device_ep(au, num, 0);
    }
}

static void usb_setup(UDC_Device *au)
{
    usb_init_endpoints(au);

    /*set default Ep0 Max Packet Size*/
    writeb_relaxed(&au->regs->ep0maxpack, 0x40);
    /*disable all interrupt*/
    writew_relaxed(&au->regs->rxien, 0);
    writew_relaxed(&au->regs->txien, 0);

    /*Clear all interrupts*/
    writew_relaxed(&au->regs->rxirq, 0xFFFF);
    writew_relaxed(&au->regs->txirq, 0xFFFF);
    writeb_relaxed(&au->regs->usbirq, 0xEF);

    /*Reset all USB endpoints*/
    writeb_relaxed(&au->regs->endprst, UCM_ENDPRST_IO_TX);
    writeb_relaxed(&au->regs->endprst,
            UCM_ENDPRST_FIFORST | UCM_ENDPRST_TOGRST
            | UCM_ENDPRST_IO_TX);
    writeb_relaxed(&au->regs->endprst,
            UCM_ENDPRST_FIFORST | UCM_ENDPRST_TOGRST);
    au->isReady =1;
}

void usb_start(UDC_Device *au)
{
    UCRegs * regs;
    uint8_t reg;

    if (!au)
        return;

    regs = au->regs;
    /*Set Connect USB controller*/
    reg = readb_relaxed(&regs->usbcs);
    reg &= ~UCM_USBCS_DISCON;
    writeb_relaxed(&regs->usbcs, reg);
    /* writeb_relaxed(&regs->speedctrl, UCM_SPEEDCTRL_HSDISABLE); */
    usb_dma_start(&au->dma);

    reg = readb_relaxed(&regs->usbien);
    reg |= UCM_USBIR_URES | UCDM_USBIR_SUDAV  | UCDM_USBIR_LPMIR;
    reg |= UCM_USBIR_HSPPED | UCM_USBIR_SUSP;
    writeb_relaxed(&regs->usbien, reg);
}

uint32_t usb_get_actual_length(uint8_t endpoint)
{
    uint8_t index = (endpoint << 1) + (endpoint >> 7) ;
    UDC_Device *au = &aic_udc;

    return usb_dma_get_actual_length(&au->dma, au->channel[index]);
}

void usb_data_stage_callback(uint8_t endpoint)
{
    USB_AIDXPF(0x03a76e, "d", "usb_data_stage_callback endpoint = %d\n",endpoint);

    if (endpoint & UX_ENDPOINT_DIRECTION)
        _ux_dcd_aic_DataInStageCallback(endpoint);
    else
        _ux_dcd_aic_DataOutStageCallback(endpoint);
}

void usb_dma_callback(void *privateData, uint8_t epNum, uint8_t epDir)
{
    epNum = epNum | epDir;

    USB_AIDXPF(0x063b20, "dd", "usb_dma_callback dir = %d, ep = %d\n", epDir, epNum);
    if (epNum & 0xf)
        usb_data_stage_callback(epNum);
    else
        usb_ep0_data_callback();
}

static void usb_phy_poweron(UDC_Device *au)
{
#ifdef CONFIG_PMIC_PMU
    if (!au->cfg.vddusb3v1)
        return;
    pmu_enable(au->cfg.vddusb3v1);
#endif
}

static void usb_phy_powerdown(UDC_Device *au)
{
#ifdef CONFIG_PMIC_PMU
    if (!au->cfg.vddusb3v1)
        return;
    pmu_disable(au->cfg.vddusb3v1);
#endif
}

static int usb_power_init(UDC_Device *au)
{
#ifdef CONFIG_PMIC_PMU
    au->cfg.vddusb3v1 = pmu_get("usb3v1", "VDDUSB3V1");
    if (!au->cfg.vddusb3v1) {
        aicx_error(0x1acfeb, "", "get usb3v1 failed!\n");
        return AIC_ENXIO;
    }
#endif
    writel_relaxed((void *)au->cfg.usb_ana_base, 2);
    return AIC_SUCCESS;
}

void usb_disconnect(void)
{
    UDC_Device *au = &aic_udc;
    uint8_t usbcs;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (au->ustatus) {
        writeb_relaxed(&au->regs->cpuctrl, BIT_0);
        usb_suspend_enable(au, 0);
        NVIC_DisableIRQ(configDriver.wakeup_irq);
        NVIC_ClearPendingIRQ(configDriver.wakeup_irq);
        usbcs = readb_relaxed(&au->regs->usbcs);
        usbcs |= UCM_USBCS_SIGRSUME;
        writeb_relaxed(&au->regs->usbcs, usbcs);
        au->ustatus = UDC_RESUME;
    }
    au->power_disable_depth = 0;
    /*Set Disconnect USB controller*/
    usbcs = readb_relaxed(&au->regs->usbcs);
    usbcs |= UCM_USBCS_DISCON;
    writeb_relaxed(&au->regs->usbcs, usbcs);
    AOS_INTERRUPT_RESTORE
}

void usb_connect(void)
{
    UDC_Device *au = &aic_udc;
    uint8_t usbcs;

    usbcs = readb_relaxed(&au->regs->usbcs);
    usbcs &= ~UCM_USBCS_DISCON;
    writeb_relaxed(&au->regs->usbcs, usbcs);
}

void usb_hw_reset()
{
    writel_relaxed((void *)configDriver.rst, configDriver.rsteb);
    aic_udelay(USB_PHY_RESET_UDELAY);
    writel_relaxed((void *)configDriver.rst, 0);
}

void usb_clk_enable()
{
    writel_relaxed((void *)configDriver.eb_base, configDriver.eb);
}

void usb_clk_disable()
{
    writel_relaxed((void *)configDriver.eb_base, 0);
    usb_hw_reset();
}

void usb_reset_phy(UDC_Device *au)
{
    writeb(&au->regs->phyctrl0, 0);
    aic_udelay(USB_PHY_RESET_UDELAY);
    writeb(&au->regs->phyctrl0, UCM_PHYCTRL0_RESET);
    aic_udelay(USB_PHY_RESET_UDELAY);
}

void usb_plugin(void)
{
    UDC_Device *au = &aic_udc;

    if (configDriver.enum_timeout)
        usb_service_enum_timer_active();

    usb_phy_poweron(au);
    usb_reset_phy(au);
    if (configDriver.ref_clk_base)
        writel((void *)configDriver.ref_clk_base, configDriver.ref_clk_eb);
#ifdef USB_PHY_SOURCE_CLK_PARENT_EN
    clk_pll_enable(USB_PHY_SOURCE_CLK_PARENT_EN);
#endif
#ifdef USB_PHY_SOURCE_CLK_EN
    clk_pll_enable(USB_PHY_SOURCE_CLK_EN);
#endif
    writel_relaxed((void *)au->cfg.usb_ana_base, 0);
    usb_setup(au);
    usb_start(au);
}

void usb_plugout(void)
{
    UDC_Device *au = &aic_udc;

    if (configDriver.enum_timeout)
        usb_service_enum_timer_deactive();

    usb_disconnect();
    aos_sleep(6);
    writel_relaxed((void *)au->cfg.usb_ana_base, 2);
    if (configDriver.ref_clk_base)
        writel((void *)configDriver.ref_clk_base, 0);
#ifdef USB_PHY_SOURCE_CLK_EN
    clk_pll_disable(USB_PHY_SOURCE_CLK_EN);
#endif
#ifdef USB_PHY_SOURCE_CLK_PARENT_EN
    clk_pll_disable(USB_PHY_SOURCE_CLK_PARENT_EN);
#endif
    usb_phy_powerdown(au);
    _ux_system_slave->ux_system_slave_remote_wakeup_capability = 0;
}

int usb_direct_enum(void)
{
    return configDriver.direct_enum;
}

static int usb_init(void)
{
    UDC_Device *au = &aic_udc;
    CUSBD_Config* config;

    usb_hw_reset();
    config = &configDriver;
    memset(au, 0, (sizeof(UDC_Device)));
    au->cfg = *config;
    au->regs = (UCRegs*) config->regBase;

    usb_power_init(au);
    /*Creating Dma Controller*/
    au->dma_cfg.regBase = config->regBase + 0x400;
    au->dma_cfg.trbAddr = config->trbAddr;
    au->dma_cfg.trbDmaAddr = config->trbDmaAddr;

    if(config->dmultEnabled) {
        au->dma_cfg.dmaModeRx = 0xFFFF;
        au->dma_cfg.dmaModeTx = 0xFFFF;
    } else {
        au->dma_cfg.dmaModeRx = 0x0000;
        au->dma_cfg.dmaModeTx = 0x0000;
    }

    au->dmaCallback.complete = usb_dma_callback;
    usb_dma_init(&au->dma, &au->dma_cfg, &au->dmaCallback);

    au->mutex = aos_create_mutex("udc");
    AIC_ASSERT(au->mutex);
    _ux_dcd_aic_initialize(au);
    register_irq(config->irq, usb_udc_irq_handle, (void *)au,
            usb_setup_handler, (void *)au);
    register_irq(config->wakeup_irq, usb_wakeup_irq, (void *)au,
        NULL, NULL);
    NVIC_DisableIRQ(configDriver.wakeup_irq);

    return AIC_SUCCESS;
}

DIS_BASIC_DRV_P2_INIT_EXPORT(usb_init);
