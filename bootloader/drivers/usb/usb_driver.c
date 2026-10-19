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
#include <usb/usb_hw_cfg.h>
#include <usb/usb_dma.h>
#include <usb/usb_device.h>
#include <usb/cusb_ch9.h>

struct usb_descriptor {
    CH9_UsbDeviceDescriptor device_desc;
    CH9_UsbConfigurationDescriptor cfg_desc;
    CH9_UsbInterfaceDescriptor interface_desc;
    CH9_UsbEndpointDescriptor ep_desc[2];
};
struct usb_descriptor *usb_desc;

UDC_Device aic_udc;
CH9_UsbEndpointDescriptor ep0_desc;

void usb_data_transfer_complete(void);

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

    USB_PRINTF("USB Stop Activity %s\n");

    for (i = 0; i < (USB_ENDPOINT_NUM >> 1); i++) {
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
    USB_PRINTF("USB Disable Device Activity %s\n");
}

int usb_setup_handler(uint8_t *setupdata)
{
    CH9_UsbSetup *ctrl = (CH9_UsbSetup *)setupdata;
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
                            default:
                                return -1;
                        }
                    } else {
                        if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) ==
                            CH9_USB_REQ_RECIPIENT_DEVICE) {
                        switch (ctrl->wValue >> 8) {
                            case CH9_USB_DT_DEVICE:
                                length = sizeof(usb_desc->device_desc);
                                length = ctrl->wLength > length ? length : ctrl->wLength;
                                ret = usb_EpXDataSend(&ep0_desc, (uint8_t *)&usb_desc->device_desc, length);
                                usb_data_transfer_complete();
                                break;

                            case CH9_USB_DT_CONFIGURATION:
                                length = sizeof(usb_desc) - 18;
                                length = ctrl->wLength > length ? length : ctrl->wLength;
                                ret = usb_EpXDataSend(&ep0_desc, (uint8_t *)&usb_desc->cfg_desc, length);
                                usb_data_transfer_complete();
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
                                usb_EpXDataSend(&ep0_desc, (uint8_t *)strDesc, length);
                                usb_data_transfer_complete();
                            }
                                break;
                            default:
                                USB_PRINTF("parse packet fail \n");
                                return -1;

                        }//switch
                    }//if
                 }
                 break;

                case CH9_USB_REQ_SET_CONFIGURATION:
                    usb_endpoint_create(&aic_udc, &usb_desc->ep_desc[0]);
                    usb_endpoint_create(&aic_udc, &usb_desc->ep_desc[1]);
                    aic_udc.isReady = 1;
                    return 0;

                case CH9_USB_REQ_GET_CONFIGURATION:
                    break;

                default:
                    USB_PRINTF("Error_2\n");
                    return -1; //return error
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

    usb_setup_handler(setupdata);
    return AIC_SUCCESS;
}

static int usb_udc_irq_handle(UDC_Device *au)
{
    UCRegs *regs = au->regs;
    uint8_t usbIrqV, usbIrqM;

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
        usb_ep0_irq(au);
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

void usb_data_transfer_complete(void)
{
    while (!aic_udc.complete)
        usb_udc_irq_handle(&aic_udc);
    aic_udc.complete = 0;
}

uint32_t usb_data_read(uint8_t *buffer, uint32_t length)
{
    if (usb_EpXDataReceive(&usb_desc->ep_desc[1], buffer, length))
        return 0;
    usb_data_transfer_complete();
    return aic_udc.TransferLength;
}

uint32_t usb_data_write(uint8_t *buffer, uint32_t length)
{
    if (usb_EpXDataSend(&usb_desc->ep_desc[0], buffer, length))
        return 0;
    usb_data_transfer_complete();
    return aic_udc.TransferLength;
}

static void usb_setup(UDC_Device *au)
{
    USB_PRINTF("Prepareing Device for operation\n");

    usb_endpoint_create(&aic_udc, &ep0_desc);
    ep0_desc.bEndpointAddress = 0x80;
    usb_endpoint_create(&aic_udc, &ep0_desc);
}

static void usb_start(UDC_Device *au)
{
    usb_dma_start(&au->dma);
}

void usb_clk_enable(void)
{
    writel((void *)configDriver.eb_base, configDriver.eb);
}

void usb_enum_port(UDC_Device *au)
{
    while (!au->isReady)
        usb_udc_irq_handle(au);
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

static int usb_init(void)
{
    UDC_Device *au = &aic_udc;
    CUSBD_Config* config;

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
    au->dmaCallback.complete = usb_dma_callback;
    usb_dma_init(&au->dma, &au->dma_cfg, &au->dmaCallback);

    usb_setup(au);
    usb_start(au);

    usb_endpoint_create(&aic_udc, &usb_desc->ep_desc[0]);
    usb_endpoint_create(&aic_udc, &usb_desc->ep_desc[1]);
    au->isReady = 1;

    return AIC_SUCCESS;
}

service_init_late(usb_init);
