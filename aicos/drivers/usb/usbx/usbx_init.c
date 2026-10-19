/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15efac
#define LOG_MODULE    AMOD_USB

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <cache.h>
#include <ux_api.h>
#include <usb/usb_cfg.h>
#include <usb_componets.h>
#include <ux_device_stack.h>

struct usb_desc {
    UCHAR *hs_descriptor[COMBO_MAX];
    UCHAR *fs_descriptor[COMBO_MAX];
    ULONG desc_size[COMBO_MAX];
};

struct aic_usbx {
    struct usb_desc descriptor;
    ULONG *memory_buffer;
    UCHAR *hs_desc;
    UCHAR *fs_desc;
    ULONG desc_size;
};

struct aic_usbx aic_usbx = {
    .descriptor.hs_descriptor = {hs_vcom_desc, hs_rndis_desc, hs_ecm_desc, hs_msc_desc},
    .descriptor.fs_descriptor = {fs_vcom_desc, fs_rndis_desc, fs_ecm_desc, fs_msc_desc},
    .descriptor.desc_size = {sizeof(hs_vcom_desc), sizeof(hs_rndis_desc),
                      sizeof(hs_ecm_desc), sizeof(hs_msc_desc),},
    .memory_buffer = NULL,
 };

 UEC usbx_get_combo_default(void)
 {
     return UX_DEFAULT_ENUM;
 }

void usbx_descriptor_build(UEC combo)
{
    aic_usbx.hs_desc = aic_usbx.descriptor.hs_descriptor[combo];
    aic_usbx.fs_desc = aic_usbx.descriptor.fs_descriptor[combo];
    aic_usbx.desc_size = aic_usbx.descriptor.desc_size[combo];
}

int usbx_uninit(void)
{
    uint32_t status;

    aicx_error(0x0e23cc, "", "usbx_uninit \n");

    status = ux_device_stack_uninitialize();
    if (status) {
      aicx_error(0x159c27, "x", "ux_device_stack_uninitialize status == 0x%x\n", status);
      return AIC_EIO;
    }

    return AIC_SUCCESS;
}

int usbx_device_stack_init(void)
{
    uint32_t status;

        /* The code below is required for installing the device portion of USBX */
    status = _ux_device_stack_initialize(aic_usbx.hs_desc, aic_usbx.desc_size,
                                aic_usbx.fs_desc, aic_usbx.desc_size,
                                string_framework,
                                sizeof(string_framework),
                                language_id_framework,
                                sizeof(language_id_framework),
                                UX_NULL);

    return status != UX_SUCCESS ? AIC_EIO : AIC_SUCCESS;
}

int usbx_init(void)
{
    uint32_t status;

    aic_usbx.descriptor.desc_size[0] = sizeof(hs_vcom_desc);
    aic_usbx.descriptor.desc_size[1] = sizeof(hs_rndis_desc);
    aic_usbx.descriptor.desc_size[2] = sizeof(hs_ecm_desc);
    aic_usbx.descriptor.desc_size[3] = sizeof(hs_msc_desc);

    /* Initialize USBX Memory.  */
    aic_usbx.memory_buffer = aos_dma_malloc(UX_MEMORY_SIZE);
    AIC_ASSERT(aic_usbx.memory_buffer);

    status = ux_system_initialize(aic_usbx.memory_buffer, UX_MEMORY_SIZE,
                                UX_NULL, 0);
    if (status != UX_SUCCESS)
        return AIC_EIO;

    return AIC_SUCCESS;
}

DIS_BASIC_DRV_P1_INIT_EXPORT(usbx_init);
