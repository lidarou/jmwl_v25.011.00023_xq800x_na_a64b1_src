/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the flash management device driver test case.
 */

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <cache.h>
#include <console/console.h>
#include <initcall.h>
#include <fmd.h>
#include <fmdcore.h>
#include <snor.h>

#define fmdlog_info     log_print
#define fmdlog_error    log_print

#define TEST_PATTERN_COUNT      (9)
u32 test_fix_pattern[TEST_PATTERN_COUNT] = {
    0x12345678, 0xFEFEFEFE, 0xEFEFEFEF, 0xFFFFEEEE,
    0xEEEEFFFF, 0x5A5A5A5A, 0xA5A5A5A5, 0x5555AAAA,
    0xAAAA5555
};

static void generate_test_fix_pattern(u32 *buffer, u32 length, u32 pattern)
{
    u32 i;

    for (i = 0; i < length; i++)
        buffer[i] = pattern;
}

#ifdef CONFIG_SNOR_SIGNAL_TEST
static void fmd_snor_signal_test(struct fmd_dev *dev, u32 ofs, u32 size, u32 map_addr)
{
    u32 buf[512/4], check[512/4], pattern;
    u32 i, j, k, erase_start, offset, loop_count;

    for (loop_count = 0; loop_count < 1; loop_count++) {
        pattern = test_fix_pattern[loop_count];
        generate_test_fix_pattern(buf, 512/4, pattern);

        fmdlog_info("signal pattern(0x%x) test start!\n", pattern);

        for (i = 0; i < (size / SIZE_4K); i++) {
            erase_start = ofs + i * SIZE_4K;
            dev->_erase(dev, erase_start, SIZE_4K);

            for (j = 0; j < (SIZE_4K / 512); j++) {
                offset = erase_start + (j * 512);

                dev->_read(dev, offset, 512, (u8 *)check);

                for (k = 0; k < 512 / 4; k += 4) {
                    if (0xFFFFFFFF != check[k]) {
                        fmdlog_error("erase check off=0x%x failed, val(0x%x)\n",
                                     (offset + k * 4), check[k]);
                        while (1);
                    }
                }

                memset(check, 0x0, 512);

                dev->_write(dev, offset, 512, (u8 *)buf);

                /* memory address read */
                *((volatile u32 *)(map_addr + ofs + 0x004));
                *((volatile u32 *)(map_addr + ofs + 0x044));
                *((volatile u32 *)(map_addr + ofs + 0x08C));
                *((volatile u32 *)(map_addr + ofs + 0x108));
                *((volatile u32 *)(map_addr + ofs + 0x188));
                *((volatile u32 *)(map_addr + ofs + 0x208));
                *((volatile u32 *)(map_addr + ofs + 0x288));
                *((volatile u32 *)(map_addr + ofs + 0x308));
                *((volatile u32 *)(map_addr + ofs + 0x388));
                dcache_inv_by_range(map_addr + ofs, 0x2000);

                memset(check, 0x0, 512);
            }
        }

        memset(buf, 0x0, 512);

        fmdlog_info("signal pattern(0x%x) test success!\n", pattern);
    }
}
#else
static void fmd_snor_stress_test(struct fmd_dev *dev, u32 ofs, u32 size)
{
    u32 buf[512/4], check[512/4], pattern;
    u32 i, j, k, erase_start, offset, loop_count;

    for (loop_count = 0; loop_count < TEST_PATTERN_COUNT; loop_count++) {
        pattern = test_fix_pattern[loop_count];
        generate_test_fix_pattern(buf, 512/4, pattern);

        fmdlog_info("stress pattern(0x%x) test start!\n", pattern);

        for (i = 0; i < (size / SIZE_4K); i++) {
            erase_start = ofs + i * SIZE_4K;
            dev->_erase(dev, erase_start, SIZE_4K);

            for (j = 0; j < (SIZE_4K / 512); j++) {
                offset = erase_start + (j * 512);

                dev->_read(dev, offset, 512, (u8 *)check);

                for (k = 0; k < 512 / 4; k += 4) {
                    if (0xFFFFFFFF != check[k]) {
                        fmdlog_error("erase check off=0x%x failed, val(0x%x)\n",
                                     (offset + k * 4), check[k]);
                        while (1);
                    }
                }

                memset(check, 0x0, 512);

                dev->_write(dev, offset, 512, (u8 *)buf);

                dev->_read(dev, offset, 512, (u8 *)check);

                for (k = 0; k < 512 / 4; k += 4) {
                    if (pattern != check[k]) {
                        fmdlog_error("write check off=0x%x failed, val(0x%x) err(0x%x)\n",
                                     (offset + k * 4), pattern, check[k]);
                        while (1);
                    }
                }

                memset(check, 0x0, 512);
            }
        }

        memset(buf, 0x0, 512);

        fmdlog_info("stress pattern(0x%x) test success!\n", pattern);
    }
}
#endif

int fmd_test_entry(void)
{
    u32 handle, bl_ofs, bl_size, test_ofs, test_size;
    struct fmd_dev *dev;
    struct snor_dev *snor;
    int ret;

    dev = (struct fmd_dev *)fmd_get_dev_handle(CONFIG_FMD_TEST_DEV);
    if (dev == NULL) {
        fmdlog_error("get dev handle error!\n");
        while (1);
    }

    if (CONFIG_FMD_TEST_DEV == 0) {
        ret = fmd_open("bl", &handle);
        if (ret < 0) {
            fmdlog_error("open bl partiton failed, errno(%d)!\n", ret);
            while (1);
        }

        ret = fmd_get_partition_size(handle, &bl_size);
        if (ret < 0) {
            fmdlog_error("get bl size failed, errno(%d)!\n", ret);
            while (1);
        }

        ret = fmd_get_partition_ofs(handle, &bl_ofs);
        if (ret < 0) {
            fmdlog_error("get bl ofs failed, errno(%d)!\n", ret);
            while (1);
        }

        ret = fmd_close(handle);
        if (ret < 0) {
            fmdlog_error("close bl failed, errno(%d)!\n", ret);
            while (1);
        }

        /* test after bl partition */
        test_ofs = bl_ofs + bl_size;
        test_size = dev->size - test_ofs;
    } else {
        test_ofs = 0;
        test_size = dev->size;
    }

    snor = fmd_to_snor(dev);
    fmdlog_info("spi nor flash id %x%x\n", snor->mid, snor->info->did);

#ifdef CONFIG_SNOR_SIGNAL_TEST
    u32 base_addr;

    base_addr = dev->_get_mem_base(dev);

    fmd_snor_signal_test(dev, test_ofs, test_size, base_addr);

    while (1);
#else
    u32 loop = 0;
    while (1) {
        fmdlog_info("stress test loop %d\n", loop);

        fmd_snor_stress_test(dev, test_ofs, test_size);

        loop++;
    }
#endif

    return 0;
}
