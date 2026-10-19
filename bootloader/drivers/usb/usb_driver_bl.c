/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <aic_errno.h>
#include <initcall.h>
#include <io.h>
#include <aic_bitmap.h>
#include <cache.h>
#include <usb/cdc_acm_cfg.h>
#include <timer.h>
#include <usb/usb_hw_cfg.h>
#include <usb/usb_dma.h>
#include <usb/usb_device.h>
#include <usb/cusb_ch9.h>

#define USB_PHY_RESET_UDELAY            2000
#define USB_TIMEOUT_VAL     -1

int usb_data_transfer_complete(CH9_UsbEndpointDescriptor *ep, uint32_t timeout);

struct usb_descriptor {
    CH9_UsbDeviceDescriptor device_desc;
    CH9_UsbConfigurationDescriptor cfg_desc;
    CH9_UsbInterfaceDescriptor interface_desc;
    CH9_UsbEndpointDescriptor ep_desc[2];
};
struct usb_descriptor *usb_desc;

UDC_Device aic_udc;
CH9_UsbEndpointDescriptor ep0_desc;

#define EP0_OUT_DIR     0
#define EP0_IN_DIR     1

int usb_EpXDataReceive(CH9_UsbEndpointDescriptor *ep,
                             uint8_t *buffer, uint32_t length)
{
    uint32_t num =ep->bEndpointAddress & 0xf;
    UDC_Device *au = &aic_udc;
    uint32_t channelStatus, index;
    uint32_t chMaxLen;
    uint8_t epType =0;

    index = num << 1;
    chMaxLen = usb_dma_get_max_length(&au->dma, au->channel[index]);
    epType = ep->bmAttributes & UX_MASK_ENDPOINT_TYPE;

    channelStatus = usb_dma_get_channel_status(&au->dma, au->channel[index]);
    if((channelStatus >= CUSBDMA_STATUS_BUSY && !au->cfg.dmultEnabled) ||
        (channelStatus >= CUSBDMA_STATUS_BUSY
        &&  (epType != UX_ISOCHRONOUS_ENDPOINT)))	{
        USB_PRINTF("outdata transfer for ep%d is pending now\n", num);
        return AIC_EIO;
    }

    dcache_inv_by_range((int32_t)buffer, length);
    usb_dma_channel_program(&au->dma, au->channel[index],
            ep->wMaxPacketSize, (uint32_t)buffer, length);
    return 0;
}

int usb_EpXDataSend(CH9_UsbEndpointDescriptor *ep,
                            uint8_t *buffer, uint32_t length)
{
    uint32_t num =ep->bEndpointAddress & 0xf;
    uint32_t chMaxLen, index , channelStatus;
    UDC_Device *au = &aic_udc;
    uint8_t epType;

    index = (num << 1) + 1;
    chMaxLen = usb_dma_get_max_length(&au->dma, au->channel[index]);
    /*start critical section*/
    epType = ep->bmAttributes & UX_MASK_ENDPOINT_TYPE;

    channelStatus = usb_dma_get_channel_status(&au->dma, au->channel[index]);
    if((channelStatus >= CUSBDMA_STATUS_BUSY && !au->cfg.dmultEnabled) ||
        (channelStatus >= CUSBDMA_STATUS_BUSY &&
        (epType != UX_ISOCHRONOUS_ENDPOINT)))	{
        USB_PRINTF("indata transfer for ep%d  is pending now, channelStatus = 0x%x\n",
            num, channelStatus);
        return AIC_EIO;
    }

    USB_PRINTF("packetSize 0x%x\n", ep->wMaxPacketSize);
    dcache_clean_by_range((int32_t)buffer, length);
    usb_dma_channel_program(&au->dma, au->channel[index],
            ep->wMaxPacketSize, (uint32_t)buffer, length);
    return 0;
}

static void
usb_abort_endpoint(UDC_Device *au, uint8_t num, uint8_t isInEp)
 {
    uint32_t index;
    USB_PRINTF("Abort Device endpoint: %d, isInEp = %d\n", num, isInEp);

    index = (num << 1) + isInEp;
    /*For endpoint different than 0 firmware has to release DMA Channel*/
    if (au->channel[index])
        usb_dma_channel_abort(&au->dma, au->channel[index]);
}

static void
usb_release_endpoint(UDC_Device *au, uint8_t num, uint8_t isInEp)
 {
    uint32_t index;
    USB_PRINTF("Abort Device endpoint: %d, isInEp = %d\n", num, isInEp);

    index = (num << 1) + isInEp;
    /*For endpoint different than 0 firmware has to release DMA Channel*/
    if (au->channel[index]) {
        usb_dma_channel_release(&au->dma, au->channel[index]);
        au->channel[index] = NULL;
    }
}

static int usb_ep0_enable(UDC_Device *au, uint8_t num, uint8_t isInEp)
{
    uint8_t chan_index = (num << 1) + isInEp;
    UCRegs * regs = au->regs;
    uint16_t tmp;

    if (!isInEp) {
        tmp = readw(&regs->rxien);
        /*disable out irq for Ep*/
        tmp &= ~(1 << num);
        writew(&regs->rxien, tmp);
        writeb(&regs->ep0fifoctrl,
            UCM_FIFOCTRL_IO_TX | UCM_FIFOCTRL_FIFOAUTO);
    } else {
        tmp = readw(&regs->txien);
        /*disable out irq for Ep*/
        tmp &= ~(1 << num);
        writew(&regs->txien, tmp);
        writeb(&regs->ep0fifoctrl, UCM_FIFOCTRL_FIFOAUTO );
    }

    au->channel[chan_index] = usb_dma_channel_alloc(&au->dma,
            isInEp, num, 0);
    if(!au->channel[chan_index] ) {
        USB_PRINTF("Error: Dma Channel not allocated\n");
        return AIC_EIO;
    }
    writeb(&regs->ep0maxpack, 0x40);
    return AIC_SUCCESS;
}

static int usb_epX_enable(UDC_Device *au, uint8_t num,
                        CH9_UsbEndpointDescriptor *epInit,
                        uint8_t isInEp)
{
    uint8_t chan_index = (num << 1) + isInEp;
    uint16_t tmp = epInit->wMaxPacketSize;
    UCRegs *regs = au->regs;
    uint32_t payload;
    uint8_t type;

    if (!(tmp & 0x07ff))
        return AIC_EINVAL;

    payload = tmp & 0x07ff;
    switch (epInit->bmAttributes) {
    case UX_BULK_ENDPOINT:
        type = UCM_CON_TYPE_BULK;
        break;
    case UX_INTERRUPT_ENDPOINT:
        type = UCM_CON_TYPE_INT;
        break;
    default:
        type = epInit->bmAttributes;
    }

    tmp &= 0x07ff;
    if (isInEp) {
        if (tmp > au->cfg.epIN[num].maxPacketSize)
            return AIC_EINVAL;

        writew(&regs->txmaxpack[num - 1], tmp);
        writeb(&regs->ep[num - 1].txcon,
                UCM_CON_VAL | type
                | (au->cfg.epIN[num].bufferingValue - 1));

        writeb(&regs->fifoctrl,
                (UCM_FIFOCTRL_FIFOAUTO | UCM_FIFOCTRL_IO_TX | num));

        /*Reset Endpoint (fifo and toggle bit*/
        writeb(&regs->endprst, UCM_ENDPRST_IO_TX | num);
        writeb(&regs->endprst,
                UCM_ENDPRST_IO_TX | num | UCM_ENDPRST_FIFORST
                | UCM_ENDPRST_TOGRST);
    } else {
        if (tmp > au->cfg.epOUT[num].maxPacketSize)
        	return AIC_EINVAL;

        writew(&regs->rxmaxpack[num - 1], tmp);
        writeb(&regs->ep[num - 1].rxcon,
                UCM_CON_VAL | type
                | (au->cfg.epIN[num].bufferingValue - 1));

        writeb(&au->regs->fifoctrl,
                (UCM_FIFOCTRL_FIFOAUTO | num));

        /*Reset Endpoint (fifo and toggle bit*/
        writeb(&regs->endprst, num);
        writeb(&regs->endprst, num |
            UCM_ENDPRST_FIFORST | UCM_ENDPRST_TOGRST);
    }

    au->channel[chan_index] = usb_dma_channel_alloc(&au->dma,
            isInEp, num, 0);

    return AIC_SUCCESS;
}

void usb_endpoint_create(UDC_Device *au,
                            CH9_UsbEndpointDescriptor *epInit)
{
   uint8_t isInEp = epInit->bEndpointAddress & UX_ENDPOINT_DIRECTION ? 1 : 0;
   uint8_t num = epInit->bEndpointAddress & 0xf;

   if (epInit->bEndpointAddress & 0x0f)
     usb_epX_enable(au, num, epInit, isInEp);
   else
     usb_ep0_enable(au, num, isInEp);
}

static void usb_stop_activity(UDC_Device *au)
{
    uint8_t i;

    USB_PRINTF("USB Stop Activity\n");

    for (i = 0; i < (USB_ENDPOINT_NUM / 2); i++) {
        if (!i) {
            usb_abort_endpoint(au, i, 0);
            usb_abort_endpoint(au, i, 1);
            continue;
        }
        usb_release_endpoint(au, i, 0);
        usb_release_endpoint(au, i, 1);
    }
}

static void usb_reset(UDC_Device *au)
{
    usb_stop_activity(au);
    USB_PRINTF("USB Disable Device Activity\n");
}

int usb_dma_transfer_complete(uint32_t timeout, uint32_t dir)
{
    UDC_Device *au = &aic_udc;
    uint32_t time, index;

    index = !!dir;
    if (((int)timeout) > 0)
        time = aic_get_tick() + timeout;

    while ((!aic_udc.complete)) {
        usb_dma_isr(&aic_udc.dma);
        if ((((int)timeout) > 0) && (time <= aic_get_tick())) {
            aic_udc.complete = 0;
            usb_dma_channel_abort(&au->dma, au->channel[index]);
            return AIC_EIO;
        }
    }
    aic_udc.complete = 0;

    return AIC_SUCCESS;
}

int usb_setup_handler(uint8_t *setupdata)
{
    CH9_UsbSetup *ctrl = (CH9_UsbSetup *)setupdata;
    uint8_t buf[2] = {0 , 0};
    uint32_t length;
    int ret;

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK){
        case CH9_USB_REQ_TYPE_STANDARD:
            switch (ctrl->bRequest) {
                case CH9_USB_REQ_GET_DESCRIPTOR:
                    if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK)
                        == CH9_USB_REQ_RECIPIENT_INTERFACE) {
                        switch (ctrl->wValue >> 8) {
                            case 0x22:
                                break;
                            case 0x21:
                                break;
                        }
                    } else {
                        if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) ==
                            CH9_USB_REQ_RECIPIENT_DEVICE) {
                        switch (ctrl->wValue >> 8) {
                            case CH9_USB_DT_DEVICE:
                                length = sizeof(usb_desc->device_desc);
                                length = ctrl->wLength > length ? length : ctrl->wLength;
                                ret = usb_EpXDataSend(&ep0_desc, (uint8_t *)&usb_desc->device_desc, length);
                                if (ret)
                                   return ret;
                                ret = usb_dma_transfer_complete(USB_TIMEOUT_VAL, EP0_IN_DIR);
                                if (ret)
                                   return ret;
                                break;

                            case CH9_USB_DT_CONFIGURATION:
                                length = sizeof(*usb_desc) - 18;
                                length = ctrl->wLength > length ? length : ctrl->wLength;
                                ret = usb_EpXDataSend(&ep0_desc, (uint8_t *)&usb_desc->cfg_desc, length);
                                if (ret)
                                   return ret;
                                ret = usb_dma_transfer_complete(USB_TIMEOUT_VAL, EP0_IN_DIR);
                                if (ret)
                                   return ret;
                                break;

                            case CH9_USB_DT_STRING:
                            {
                                uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                                char *strDesc;
                                USB_PRINTF("StringDesc:\n");
                                switch (descIndex) {
                                    case 0:
                                        strDesc = (char *)string_framework0;
                                        length = strDesc[0];
                                        USB_PRINTF("language\n", ' ');
                                        break;

                                    case 1:
                                        strDesc = (char *)string_framework1;
                                        length = strDesc[0];
                                        USB_PRINTF("vendor\n");
                                        break;

                                    case 2:
                                        strDesc = (char *)string_framework2;
                                        length = strDesc[0];
                                        USB_PRINTF("product\n");
                                        break;

                                    default: return -1;
                                }
                                ret = usb_EpXDataSend(&ep0_desc, (uint8_t *)strDesc, length);
                                if (ret)
                                    return ret;
                                ret = usb_dma_transfer_complete(USB_TIMEOUT_VAL, EP0_IN_DIR);
                                if (ret)
                                    return ret;
                            }
                                break;
                            default:
                                USB_PRINTF("usb controller can not support this setup\n");
                                writeb(&aic_udc.regs->ep0cs, UCDM_EP0CS_DSTALL);
                                return 0;

                        }//switch
                    }//if
                 }
                 break;

                case CH9_USB_REQ_SET_CONFIGURATION:
                    usb_endpoint_create(&aic_udc, &usb_desc->ep_desc[0]);
                    usb_endpoint_create(&aic_udc, &usb_desc->ep_desc[1]);
                    writeb(&aic_udc.regs->ep0cs, UCDM_EP0CS_HSNAK);
                    aic_udc.reset = 1;
                    aic_udc.isReady = 1;
                    return 0;

                case CH9_USB_REQ_GET_CONFIGURATION:
                    break;

                case CH9_USB_REQ_GET_STATUS:
                    ret = usb_EpXDataSend(&ep0_desc, (uint8_t *)buf, 2);
                    if (ret)
                        return ret;
                    ret = usb_dma_transfer_complete(USB_TIMEOUT_VAL, EP0_IN_DIR);
                    if (ret)
                        return ret;
                    break;
            }
            break;

        case CH9_USB_REQ_TYPE_CLASS:
            break;
    }

    return 0;
}

static int usb_ep0_irq(UDC_Device *au)
{
    uint8_t setupdata[UX_SETUP_SIZE];
    UCRegs *regs = au->regs;
    int8_t i;

    writeb(&regs->ep0cs, UCDM_EP0CS_CHGSET);

    /*Read Setup packet from USB registers*/
    for (i = 0; i < UX_SETUP_SIZE; i++)
        setupdata[i] = readb(&regs->setupdat[i]);

    i = readb(&regs->ep0cs);
    if (i & UCDM_EP0CS_CHGSET) {
        USB_PRINTF("Setup Flags Chang: Not Error %s\n", " ");
        /*Setup packet has been modified during reading.
         * Probably last Setup Request was SET_ADDRESS that
         * was acknowledgment by hardware
         * In this case function return ECANCELED but it is not error;
         **/
        return AIC_SUCCESS;
    }
    writeb(&regs->usbirq, UCDM_USBIR_SUDAV);

    for (i = 0; i < UX_SETUP_SIZE; i++)
        USB_PRINTF("setupdata[%d]: 0x%x\n", i, setupdata[i]);

    return usb_setup_handler(setupdata);
}

static int usb_udc_irq_handle(UDC_Device *au)
{
    UCRegs *regs = au->regs;
    uint8_t usbIrqV, usbIrqM;
    int ret;

    usbIrqV = readb(&regs->usbirq);
    usbIrqM = readb(&regs->usbien);

    usbIrqV = usbIrqV & usbIrqM;
    if (usbIrqV & UCDM_USBIR_LPMIR) {
        USB_PRINTF("USBIRQ: LPM\n");
        /* Clear the NYET bit */
        uint8_t usbcs = readb(&regs->usbcs);
        usbcs &= ~UCDM_USBCS_LPMNYET;
        writeb(&regs->usbcs, usbcs);
        writeb(&regs->usbirq, UCDM_USBIR_LPMIR);
    }

    if (usbIrqV & UCM_USBIR_URES) {
        USB_PRINTF("USBIRQ: RESET\n");
        /* Clearing interrupt bits */
        writeb(&regs->usbirq, UCM_USBIR_URES);
        /*Reset all but not release channel DMA for EP0*/
        usb_reset(au);
    }

    /*This interrupt is not handled by firmware because Speed is checking by speedctrl SFR*/
    if (usbIrqV & UCM_USBIR_HSPPED) {
        USB_PRINTF("USBIRQ: HighSpeed\n");
        writeb(&regs->usbirq, UCM_USBIR_HSPPED);
    }

    if (usbIrqV & UCDM_USBIR_SUDAV) { /* setup packet interrupt */
        USB_PRINTF("USBIRQ: SUDAV\n");
        ret = usb_ep0_irq(au);
        if (ret)
           return ret;
    }

    /* SOF interrupt */
    if (usbIrqV & UCM_USBIR_SOF) {
        USB_PRINTF("USBIRQ: SOF\n");
        writeb(&regs->usbirq, UCM_USBIR_SOF);
    }

    if (usbIrqV & UCDM_USBIR_SUTOK) {
        USB_PRINTF("USBIRQ: SUTOK\n", " ");
        writeb(&regs->usbirq, UCDM_USBIR_SUTOK);
    }

    if (usbIrqV & UCM_USBIR_SUSP) {
        writeb(&regs->usbirq, UCM_USBIR_SUSP);
        USB_PRINTF("USBIRQ: SUSPEND\n");
    }

    usb_dma_isr(&au->dma);

    return 0;
}

uint32_t usb_get_actual_length(uint8_t endpoint)
{
    uint8_t index = (endpoint << 1) + (endpoint >> 7) ;
    UDC_Device *au = &aic_udc;

    return usb_dma_get_actual_length(&au->dma, au->channel[index]);
}

void usb_ep0_data_callback(uint8_t endpoint, uint8_t direction)
{
    writeb(&aic_udc.regs->ep0cs, UCDM_EP0CS_HSNAK);
}

void usb_dma_callback(uint8_t epNum, uint8_t epDir)
{
    if (!epNum)
        usb_ep0_data_callback(epNum, epDir);

    aic_udc.complete = 1;
    aic_udc.TransferLength = usb_get_actual_length(epNum | epDir);
}

int usb_data_transfer_complete(CH9_UsbEndpointDescriptor *ep, uint32_t timeout)
{
    uint32_t num =ep->bEndpointAddress & 0xf;
    uint64_t time, index;

    index = num << 1;
    if (((int)timeout) > 0)
        time = aic_get_tick() + timeout;

    while ((!aic_udc.complete)) {
        usb_udc_irq_handle(&aic_udc);
        if ((((int)timeout) > 0) && (time <= aic_get_tick())) {
            aic_udc.complete = 0;
            return AIC_EIO;
        }

        if (aic_udc.reset)
            return AIC_EIO;
    }
    aic_udc.complete = 0;

    return AIC_SUCCESS;
}

int usb_get_status(void)
{
    return aic_udc.isReady;
}

void usb_clk_enable(void)
{
    writel((void *)configDriver.eb_base, configDriver.eb);
}

void usb_clk_disable(void)
{
    writel((void *)configDriver.eb_base, 0);
}

static int usb_enum_port(UDC_Device *au, uint32_t timeout)
{
    uint32_t time = aic_get_tick() + timeout;
    int ret;

    while (!au->isReady) {
        ret = usb_udc_irq_handle(au);
        if (ret)
            return AIC_EIO;
        if ((((int)timeout) > 0) && (time <= aic_get_tick()))
            return AIC_EIO;
    }
    return AIC_SUCCESS;
}

int usb_start(uint32_t timeout)
{
    UDC_Device *au = &aic_udc;
    UCRegs * regs;
    uint8_t reg;
    int ret;

    usb_clk_enable();
    USB_PRINTF("Destroy Device Controller driver");
    regs = au->regs;

    reg = readb(&regs->usbien);
    reg |= UCM_USBIR_URES | UCDM_USBIR_SUDAV | UCDM_USBIR_LPMIR;
    writeb(&regs->usbien, reg);

    /*Set Connect USB controller*/
    reg = readb(&regs->usbcs);
    reg &= ~UCM_USBCS_DISCON;
    writeb(&regs->usbcs, reg);
    ret = usb_dma_start(&au->dma);
    if (ret)
        return ret;
    return usb_enum_port(au, timeout);
}

uint32_t usb_data_read(uint8_t *buffer, uint32_t length, uint32_t timeout)
{
    int ret;
    if (aic_udc.reset)
        aic_udc.reset = 0;

    if (usb_EpXDataReceive(&usb_desc->ep_desc[1], buffer, length))
        return 0;
    ret = usb_data_transfer_complete(&usb_desc->ep_desc[1], timeout);
    if (ret)
       aic_udc.TransferLength = 0;
    return aic_udc.TransferLength;
}

uint32_t usb_data_write(uint8_t *buffer, uint32_t length, uint32_t timeout)
{
    uint32_t TransferLength;
    int ret;

    if (aic_udc.reset)
        aic_udc.reset = 0;
    if (usb_EpXDataSend(&usb_desc->ep_desc[0], buffer, length))
        return 0;
    ret = usb_data_transfer_complete(&usb_desc->ep_desc[0], timeout);
    if (ret)
       return 0;

    TransferLength =  aic_udc.TransferLength;
    if (!(length % usb_desc->ep_desc[0].wMaxPacketSize)) {
        if (usb_EpXDataSend(&usb_desc->ep_desc[0], buffer, 0))
            return 0;
        ret = usb_data_transfer_complete(&usb_desc->ep_desc[0], timeout);
    }

    if (ret)
       TransferLength = 0;
    return TransferLength;
}

static void usb_init_device_ep(UDC_Device * au, uint8_t epNum,
                              uint8_t isInEp)
{
    USB_PRINTF("Init Device endpoint%d, %s\n", epNum, isInEp ? "in" : "out");
    if (!epNum)
        return;

    if (isInEp) {
        if (au->cfg.epIN[epNum].startBuf != 0)
            writew(&au->regs->txstaddr[epNum - 1].addr,
                    au->cfg.epIN[epNum].startBuf);
        writeb(&au->regs->ep[epNum - 1].txcon, 0x0);
    } else {
        if (au->cfg.epOUT[epNum].startBuf != 0)
            writew(&au->regs->rxstaddr[epNum - 1].addr,
                    au->cfg.epOUT[epNum].startBuf);
        writeb(&au->regs->ep[epNum - 1].rxcon, 0x0);
    }
}

static void usb_init_endpoints(UDC_Device *au)
{
    CUSBD_Config * config = &au->cfg;
    uint8_t num;

    for (num = 0; num < (USB_ENDPOINT_NUM / 2); num++) {
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
    USB_PRINTF("Prepareing Device for operation\n");
    usb_init_endpoints(au);

    /*set default Ep0 Max Packet Size*/
    writeb(&au->regs->ep0maxpack, 0x40);
    /*disable all interrupt*/
    writew(&au->regs->rxien, 0);
    writew(&au->regs->txien, 0);

    /*Clear all interrupts*/
    writew(&au->regs->rxirq, 0xFFFF);
    writew(&au->regs->txirq, 0xFFFF);
    writeb(&au->regs->usbirq, 0xEF);

    /*Reset all USB endpoints*/
    writeb(&au->regs->endprst, UCM_ENDPRST_IO_TX);
    writeb(&au->regs->endprst,
            UCM_ENDPRST_FIFORST | UCM_ENDPRST_TOGRST
            | UCM_ENDPRST_IO_TX);
    writeb(&au->regs->endprst,
            UCM_ENDPRST_FIFORST | UCM_ENDPRST_TOGRST);
    usb_endpoint_create(&aic_udc, &ep0_desc);
    ep0_desc.bEndpointAddress = 0x80;
    usb_endpoint_create(&aic_udc, &ep0_desc);
}
void usb_parse_descriptor(void)
{
    usb_desc = (struct usb_descriptor *)usb_cdc_descriptor;
    ep0_desc.bLength = 9;
    ep0_desc.bDescriptorType = 0x5;
    ep0_desc.bEndpointAddress = 0;
    ep0_desc.bmAttributes = 0;
    ep0_desc.wMaxPacketSize = usb_desc->device_desc.bMaxPacketSize0;
}

void usb_reset_phy(UDC_Device *au)
{
    writeb(&au->regs->phyctrl0, UCM_PHYCTRL0_RESET);
    aic_udelay(USB_PHY_RESET_UDELAY);
}

int usb_init(void)
{
    UDC_Device *au = &aic_udc;
    CUSBD_Config* config;
    uint8_t usbcs;
    int ret;

    config = &configDriver;
    usb_parse_descriptor();
    memset(au, 0, (sizeof(UDC_Device)));
    au->cfg = *config;
    au->regs = (UCRegs*) config->regBase;

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

    usb_clk_enable();
    usb_reset_phy(au);
    au->dmaCallback.complete = usb_dma_callback;
    ret = usb_dma_init(&au->dma, &au->dma_cfg, &au->dmaCallback);

    /*Set Disconnect USB controller*/
    usbcs = readb(&au->regs->usbcs);
    usbcs |= UCM_USBCS_DISCON | UCDM_USBCS_LPMNYET;
    writeb(&au->regs->usbcs, usbcs);
    usb_setup(au);
    usb_clk_disable();

    return ret;
}

early_init(usb_init);
