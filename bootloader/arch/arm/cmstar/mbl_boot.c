/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include "cache.h"
#include "console/console.h"
#include <fmd.h>
#include <hlzma.h>
#include <psram_mem_config.h>
#include "initcall.h"
#include "platform/boot.h"
#include "platform/platform.h"

#define BL_IMG_NUM          2
#define FOTA_MAGIC_NUMBER   0xaae7557e
#define LZMA_DEC_OFFSET     4

#define FOTA_PTNAME         FOTA_EXTIMG_PARTION_NAME
#define FOTA_BK_OFFSET      FOTA_DIFF_PAC_MAX_SIZE

int boot_normal_mode_enter(void)
{
    u32 *fota_flag_ptr, flash_next_img_addr, handle, fota_offset, psize;
    struct secure_boot *sec_boot = &platform.secure_boot;
    char *bl_name[BL_IMG_NUM] = {"bl", FOTA_PTNAME};
    struct image_header *header;
    int offset = 0;
    int bl_size = 0;
    int ret, i;

    for (i = 0; i < BL_IMG_NUM; i++) {
        ret = fmd_open(bl_name[i], &handle);
        if (ret < 0) {
            log_print("open partiton failed, errno(%d)!\n", ret);
            return ret;
        }

        if (strcmp(bl_name[i], FOTA_PTNAME) == 0)
            offset = FOTA_BK_OFFSET;
        else
            offset = 0;

        ret = fmd_enter_read_mode(handle);
        if (ret < 0) {
            log_print("enter read mode failed, errno(%d)!\n", ret);
            return ret;
        }

        log_print("next_img = 0x%x, offset:0x%x\n", platform.next_img, offset);
        platform.next_img = platform.next_img - IMAGE_HEADER_SIZE;
        if (platform.next_img_compressed) {
            ret = fmd_get_physical_address(handle, offset, &flash_next_img_addr);
            if (ret) {
                log_print("fmd_get_physical_address failed, errno(%d)!\n", ret);
                return ret;
            }
            dcache_invcc_all();
            ret = hlzma_dec_polling(flash_next_img_addr + LZMA_DEC_OFFSET,
                                  (u32)platform.next_img);
        } else {
            ret = fmd_get_partition_size(handle, &psize);
            if (ret) {
                log_print("fmd_get_partition_size failed, errno(%d)!\n", ret);
                return ret;
            }
            if (offset)
                psize -= offset;
            if (bl_size == 0)
                bl_size = psize;
            if (bl_size < psize)
                psize = bl_size;
            ret = fmd_read(handle, offset, psize, (u8 *)platform.next_img);
        }
        if (ret < 0) {
            log_print("next img read fail, errno(%d)!\n", ret);
            return ret;
        }
        ret = fmd_close(handle);
        if (ret < 0) {
            log_print("close partiton failed, errno(%d)!\n", ret);
            return ret;
        }

        header = (struct image_header *)platform.next_img;
        fota_offset = header->key_sign_offset + header->key_sign_size;
        fota_flag_ptr = (u32 *)(platform.next_img + fota_offset);
        if (*fota_flag_ptr == FOTA_MAGIC_NUMBER)
            break;

        platform.next_img = platform.next_img + IMAGE_HEADER_SIZE;
        log_print("fota_magic_number(0x%x), %s image boot fail\n", *fota_flag_ptr, bl_name[i]);
    }

    if (sec_boot->secure_boot_enable) {
        ret = sec_boot->secboot_authorization(&platform);
        if (ret) {
            log_print("secboot authorization fail!\n");
            return ret;
        }
    } else {
        platform.next_img = platform.next_img + IMAGE_HEADER_SIZE;
    }

    dcache_clean_all();
    icache_inv_all();
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

