/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <fmd.h>
#include "console/console.h"
#include "initcall.h"
#include "platform/boot.h"
#include "platform/platform.h"
#include <hlzma.h>
#include <fota.h>

#define EXTEND_LZMA_DEC_OFFSET  (24) /* sizeof(EXT_IMG_HEADER) + 4*/
#define LOAD_ADDR_OFFSET        (16)
#define SEC_CHECk_FAILED        (-5566)

static int boot_normal_get_image_addr(char *part, u32 *part_addr)
{
    u32 handle;
    int ret;

    ret = fmd_open(part, &handle);
    if (ret < 0) {
        log_print("open partiton failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_get_physical_address(handle, 0, part_addr);
    if (ret < 0) {
        log_print("get physical_address failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_close(handle);
    if (ret < 0) {
        log_print("close partiton failed, errno(%d)!\n", ret);
        return ret;
    }

    return 0;
}

#ifdef CONFIG_COMPRESS_AP_EXTEND
int decompress_ap_extend(void)
{
    u32 load_addr;
    u32 parti_addr;
    int ret;

    log_print("decompress ap extend img.\n");

    ret = boot_normal_get_image_addr("ap_extend", &parti_addr);
    if (ret < 0)
        return ret;

    load_addr = *((u32*)(parti_addr + 16));

    log_print("decompress to load addr:0x%x!\n", load_addr);

    ret = hlzma_dec_polling(parti_addr + EXTEND_LZMA_DEC_OFFSET,
                                  load_addr);
    if (ret < 0) {
        log_print("decompress fail, errno(%d)!\n", ret);
        return ret;
    }

    return 0;
}
#endif

static int boot_normal_fmd_enter_read_mode(void)
{
    u32 handle;
    int ret;

    log_print("set fmd enter read mode!\n");

    /* any parttition is ok. */
    ret = fmd_open("ap", &handle);
    if (ret < 0) {
        log_print("open partiton failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_enter_read_mode(handle);
    if (ret < 0) {
        log_print("enter read mode failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_close(handle);
    if (ret < 0) {
        log_print("close partiton failed, errno(%d)!\n", ret);
        return ret;
    }

    return ret;
}

static int boot_normal_secboot_check_image(char *part)
{
    struct secure_boot *sec_boot = &platform.secure_boot;
    u32 img_addr;
    int ret;

    log_print("secboot check part %s.\n", part);

    ret = boot_normal_get_image_addr(part, &img_addr);
    if (ret)
        return ret;

    platform.next_img = (uint8_t *)img_addr;
    if (sec_boot->secure_boot_enable) {
        ret = sec_boot->secboot_authorization(&platform);
        if (ret) {
            log_print("secboot authorization %s fail, ret=%d!\n", part, ret);
            return SEC_CHECk_FAILED;
        }
    } else {
#ifdef CONFIG_SECURE_BOOT
        platform.next_img = platform.next_img + IMAGE_HEADER_SIZE;
#endif
    }

    return ret;
}

int boot_normal_mode_enter(void)
{
    int ret;

    /* before check, fmd must must enter read mode. */
    boot_normal_fmd_enter_read_mode();

#ifdef CONFIG_COMPRESS_AP_EXTEND
    /* check compressed ap_extend image */
    decompress_ap_extend();
#else
    /*
     * check extend image, only check SEC_CHECk_FAILED return,
     * other fail may it does not have ap_extend image.
     */
    if (fota_is_normal_mode()) {
        ret = boot_normal_secboot_check_image("ap_extend");
        if (SEC_CHECk_FAILED == ret)
            return ret;
    }
#endif

    /* check ap image. */
    ret = boot_normal_secboot_check_image("ap");
    if (ret)
        return ret;

    /* quit bootloader, bl aicos. */
    boot_platform_quit((struct boot_arm_vector_table *)platform.next_img);

    return 0;
}

static int boot_platform_init(void)
{
    /* Initialize stack limit register */
    uint32_t msp_stack_bottom = (uint32_t)__stack_start__;

    __set_MSPLIM(msp_stack_bottom);

    platform_boot_mode_enterpoint_register(boot_normal_mode_enter,
                    NORMAL_BOOT);
    platform.boot_mode = NORMAL_BOOT;

    return 0;
}

driver_init(boot_platform_init);
