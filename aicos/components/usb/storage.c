/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d7c9f
#define LOG_MODULE    AMOD_USB

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <cache.h>
#include <fs_api.h>
#include <sys/mount.h>
#include <stdint.h>
#include <string.h>
#include <usb_service.h>
#include <usb/msc_cfg.h>
#include <usb_componets.h>
#include <ux_api.h>
#include <ux_device_stack.h>
#include <ux_device_class_storage.h>

int usb_storage_disk_open(u32 index)
{
    struct UX_STORAGE *storage = &mass_storage.storage[index];
    int ret;

    if (storage->mount_info.mount)
        umount(storage->name);

    ret = storage->ops.open(storage->name, &storage->handle);
    if (ret) {
       aicx_error(0x1121da, "sd", "usb_storage_flash_open %s fail! ret = %d",
         storage->name, ret);
       return AIC_ENODEV;
    }

    storage->ops.get_sector_size(storage->handle, &storage->block_len);
    storage->ops.get_sector_num(storage->handle, &storage->last_lba);

    return AIC_SUCCESS;
}

int usb_storage_disk_close(u32 index)
{
    struct UX_STORAGE *storage = &mass_storage.storage[index];
    int ret;

    if (!storage->handle) {
       aicx_error(0x115c4d, "", "Close file before open file!");
       return AIC_ENODEV;
    }

    ret = storage->ops.close(storage->handle);

    AICX_ASSERT_IFX(ret == AIC_SUCCESS,0x1c9620, "d", "usb_storage_flash_close ret(%d) err!", ret);
    if (storage->mount_info.mount)
        mount( storage->mount_info.devpath, storage->mount_info.target,
            storage->mount_info.fstype, storage->mount_info.flags, NULL);

    storage->handle = 0;

    return AIC_SUCCESS;
}


 UINT usb_storage_app_media_status(VOID *storage, ULONG lun, ULONG media_id,
                                        ULONG *media_status)
{
    return AIC_SUCCESS;
}

 UINT usb_storage_app_media_read(VOID *storage, ULONG lun,
                                           UCHAR *data_pointer,
                                           ULONG number_blocks, ULONG lba,
                                           ULONG *media_status)
{
    struct UX_STORAGE *ux_storage = &mass_storage.storage[lun];
    int ret;

    if (!ux_storage->handle) {
       aicx_error(0x1d7e7c, "s", "Read: disk(%s) not exsit!", ux_storage->name);
       return AIC_SUCCESS;
    }

    ret = ux_storage->ops.read(ux_storage->handle, lba, number_blocks, data_pointer);
    if (ret) {
        aicx_error(0x03a975, "xxd", "usb_storage_app_media_read(%x) sec_num(0x%x) ret(%d) err!",
            lba, number_blocks, ret);
        return ret;
    }

    return AIC_SUCCESS;
}

UINT usb_storage_app_media_write(VOID *storage, ULONG lun,
                                          UCHAR *data_pointer,
                                          ULONG number_blocks, ULONG lba,
                                          ULONG *media_status)
{
    struct UX_STORAGE *ux_storage = &mass_storage.storage[lun];
    int ret;

    if (!ux_storage->handle) {
       aicx_error(0x0b5ee6, "s", "Write: disk(%s) not exsit!", ux_storage->name);
       return AIC_SUCCESS;
    }

    ret = ux_storage->ops.write(ux_storage->handle, lba, number_blocks, data_pointer);
    if (ret) {
        aicx_error(0x162095, "xxd", "usb_storage_app_media_write(%x) sec_num(0x%x) ret(%d) err!",
                lba, number_blocks, ret);
        return ret;
    }

    return AIC_SUCCESS;
}

static void usb_storage_update_parameter(struct UX_STORAGE * ux_storage,
                                                      void *data, u32 index)
{
    UX_SLAVE_CLASS_STORAGE *storage = (UX_SLAVE_CLASS_STORAGE *)data;
    UX_SLAVE_CLASS_STORAGE_LUN *lun;

    if (index >= UX_MAX_SLAVE_LUN) {
        aicx_error(0x0840ad, "d", "index (%d）too large than UX_MAX_SLAVE_LUN\n", index);
        return;
    }

    lun = &storage->ux_slave_class_storage_lun[index];
    /* Check block length size. */
    if (ux_storage->block_len > UX_SLAVE_CLASS_STORAGE_BUFFER_SIZE) {
        aicx_error(0x1ccfbb, "d", "usb storage block_len (%d）too large\n", ux_storage->block_len);
        return;
    }

    lun->ux_slave_class_storage_media_last_lba = ux_storage->last_lba;
    lun->ux_slave_class_storage_media_block_length =ux_storage->block_len;
}

void usb_storage_instance_activeate(void *data)
{
    u32 index;

    mass_storage.class_storage = (UX_SLAVE_CLASS_STORAGE *)(data);
    for (index = 0; index < mass_storage.disk_num; index++) {
        usb_storage_disk_open(index);
        usb_storage_update_parameter(&mass_storage.storage[index], data, index);
    }

    aicx_error(0x06e40f, "", "usb_storage_instance_activeate \n");
}

void usb_storage_instance_deactivate(void *data)
{
    u32 index;

    for (index = 0; index < mass_storage.disk_num; index++)
        usb_storage_disk_close(index);

    aicx_error(0x12068b, "", "usb_storage_instance_deactivate \n");
}

void usb_storage_lun_init(UX_SLAVE_CLASS_STORAGE_LUN *lun, u32 index)
{
    lun->ux_slave_class_storage_media_last_lba =mass_storage.storage[index].last_lba;
    lun->ux_slave_class_storage_media_block_length =mass_storage.storage[index].block_len;
    lun->ux_slave_class_storage_media_type = 0;
    lun->ux_slave_class_storage_media_removable_flag = 0x80;
    lun->ux_slave_class_storage_media_read = usb_storage_app_media_read;
    lun->ux_slave_class_storage_media_write = usb_storage_app_media_write;
    lun->ux_slave_class_storage_media_status = usb_storage_app_media_status;
}

void usb_storage_fs_mount_hook(struct fs_mount_info *mntinfo, void *param)
{
    //if (!mntinfo->mount)
        //usb_service_enum_manage(LOG_AT_DIAG);
}

int usb_storage_uninit(void)
{
    uint32_t status;

    if (!mass_storage.initialization)
        return AIC_SUCCESS;
    fs_unregister_mount_hook(usb_storage_fs_mount_hook);
    status = _ux_device_stack_class_unregister(
                    _ux_system_slave_class_storage_name,
                    ux_device_class_storage_entry);
    if (status != UX_SUCCESS) {
        aicx_error(0x1919a5, "d", "storage class unregister. status = %d\n", status);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

int usb_storage_get_disk_mount_info(void)
{
    int ret, i;

    for (i = 0; i < USB_STORAGE_LUN_NUM; i++) {
        ret = fs_getmntinfo(mass_storage.storage[i].fs_name, &mass_storage.storage[i].mount_info);
        if (ret)
            aicx_error(0x132164, "sd", "Get disk(%s). ret = %d\n", mass_storage.storage[i].fs_name, ret);
    }

    return AOS_SUCCESS;
}

struct UX_STORAGE *usb_storage_get_sdmmc_storage(char *name)
{
    char *str;
    int i;

    for (i = 0; i < USB_STORAGE_LUN_NUM; i++) {
        str = strstr(mass_storage.storage[i].name, name);
        if (str)
            return &mass_storage.storage[i];
    }

    return NULL;
}

int usb_storage_init(void)
{
    UX_SLAVE_CLASS_STORAGE_PARAMETER parameter;
    ULONG number_lun = USB_STORAGE_LUN_NUM;
    struct UX_STORAGE *sdmmc_storage;
    uint32_t i, status;
    int ret;

    ret = usb_storage_get_disk_mount_info();
    if (ret)
        return ret;

    sdmmc_storage = usb_storage_get_sdmmc_storage("D");
    if (sdmmc_storage && !sdmmc_storage->mount_info.mount)
        number_lun = number_lun - 1;
    if (!number_lun)
        return AIC_SUCCESS;

    fs_register_mount_hook(usb_storage_fs_mount_hook, NULL);
    ux_utility_memory_set(&parameter, 0, sizeof(parameter));
    parameter.ux_slave_class_storage_instance_activate =
        usb_storage_instance_activeate;
    parameter.ux_slave_class_storage_instance_deactivate =
        usb_storage_instance_deactivate;
    parameter.ux_slave_class_storage_parameter_number_lun = number_lun;

    for (i = 0; i < parameter.ux_slave_class_storage_parameter_number_lun; i++)
        usb_storage_lun_init(&parameter.ux_slave_class_storage_parameter_lun[i], i);
    /* Initialize the device rndis class. This class owns both interfaces. */
    status = ux_device_stack_class_register(_ux_system_slave_class_storage_name,
        ux_device_class_storage_entry, 1,0, &parameter);
    aicx_error(0x0cc89e, "x", "ux_device_stack_class_register ret =0x%x.\n", status);
    mass_storage.initialization = 1;

    return status;
}

struct usb_class_instance storage_instance = {
    .init = usb_storage_init,
    .uninit =usb_storage_uninit,
    .status = 1,
    .name = "usb mass storage",
};

int usb_storage_register(void)
{
    usb_service_register_instance(MSC, storage_instance);
    return AIC_SUCCESS;
}
DIS_BASIC_COMP_P1_INIT_EXPORT(usb_storage_register);
