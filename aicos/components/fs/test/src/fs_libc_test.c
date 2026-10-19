/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs test operations.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x179a73
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#ifndef WIN32
#include <stdio.h>
#else
#include "fs_win32.h"
#endif
#include "fs_test.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FS_TEST_TASK_STACK_SIZE    4096
#define FS_TEST_TASK_PRIORITY      32
#define FS_TEST_QUEUE_NUM          10

#define FS_RAM_TEST_DIR_NAME       "RAM:/"

#define FS_FLASH_TEST_DIR_NAME     "C:/"

#define FS_SD_TEST_DIR_NAME        "D:/"

#define FS_WIN32_TEST_DIR_NAME_C   "C:/"
#define FS_WIN32_TEST_DIR_NAME_D   "D:/"

#define FS_TEST_BUF_LEN             100
#define FS_TEST_BUF                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"

#define FS_TEST_NUM                 10

#define FS_MAX_NAME_LEN             256

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_thread_id fs_libc_test_thread_id;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_libc_test_entry - fs libc test task entry.
 *
 * Parameter:
 * @param: task input param.
 *
 * Return:
 */
void fs_libc_test_entry(void *param)
{
    char *dst_buf, *src_buf, *file_name, *dev_file_name[FS_TEST_NUM] = {0};
    char *volume_name[FS_TEST_NUM] = {0};
    u32 i ,len, fs_num = 0, count;
    size_t rsize;
    FILE *stream;
    int ret;

#ifdef CONFIG_FS_DEV_RAM
    volume_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(volume_name[fs_num], "RAM:", strlen("RAM:"));
    dev_file_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(dev_file_name[fs_num], FS_RAM_TEST_DIR_NAME,
               strlen(FS_RAM_TEST_DIR_NAME));
    ++fs_num;
#endif

#ifdef CONFIG_FS_DEV_FLASH
    volume_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(volume_name[fs_num], "C:", strlen("C:"));
    dev_file_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(dev_file_name[fs_num], FS_FLASH_TEST_DIR_NAME,
               strlen(FS_FLASH_TEST_DIR_NAME));
    ++fs_num;
#endif

#ifdef CONFIG_DRIVER_SDMMC
    volume_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(volume_name[fs_num], "D:", strlen("D:"));
    dev_file_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(dev_file_name[fs_num], FS_SD_TEST_DIR_NAME,
               strlen(FS_SD_TEST_DIR_NAME));
    ++fs_num;

#endif

#ifdef CONFIG_FS_DEV_WIN32
    volume_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(volume_name[fs_num], "C:", strlen("C:"));
    dev_file_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(dev_file_name[fs_num], FS_WIN32_TEST_DIR_NAME_C,
               strlen(FS_WIN32_TEST_DIR_NAME_C));
    ++fs_num;

    volume_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(volume_name[fs_num], "D:", strlen("D:"));
    dev_file_name[fs_num] = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(dev_file_name[fs_num], FS_WIN32_TEST_DIR_NAME_D,
               strlen(FS_WIN32_TEST_DIR_NAME_D));
    ++fs_num;
#endif

    dst_buf = aos_zalloc(FS_TEST_BUF_LEN);
    src_buf = aos_zalloc(FS_TEST_BUF_LEN);
    file_name = aos_zalloc(FS_MAX_NAME_LEN);
    aos_memcpy(src_buf, FS_TEST_BUF, strlen(FS_TEST_BUF));

    while(1) {
        for (count = 0; count < fs_num; count++) {
            if (dev_file_name[count]) {
                len = strlen(dev_file_name[count]);
            } else {
                continue;
            }

            i = 0;
            while(i < 1) {
                aos_memset(file_name, 0, FS_MAX_NAME_LEN);
                snprintf(file_name, FS_MAX_NAME_LEN,
                         "%s", dev_file_name[count]);
                aos_memcpy(file_name + strlen(file_name), volume_name[count],
                           strlen(volume_name[count]) - 1);
                snprintf(file_name + strlen(file_name),
                         FS_MAX_NAME_LEN - strlen(file_name),
                         "_test%d.txt", i);
                /* Open the test file.  */
                stream = fopen(file_name, "a+");
                /* Check the file open status.  */
                AOS_ASSERT(ferror(stream) == AIC_SUCCESS);

                ret = fseek(stream, 0, 0);
                AOS_ASSERT(ret == 0);

                rsize = fwrite(src_buf, 1, strlen(src_buf), stream);
                AOS_ASSERT(rsize == strlen(src_buf));

                ret = fflush(stream);
                AOS_ASSERT(ret == 0);

                /* Seek to the end of the test file.  */
                ret = fseek(stream, 0, 2);
                AOS_ASSERT(ret == 0);

                aos_memset(dst_buf, 0x0, FS_TEST_BUF_LEN);
                rsize = fread(dst_buf, 1, FS_TEST_BUF_LEN, stream);

                ret = fseek(stream, 0, 0);
                AOS_ASSERT(ret == 0);

                rsize = fread(dst_buf, 1, FS_TEST_BUF_LEN, stream);
                AOS_ASSERT(rsize == strlen(dst_buf));

                AOS_ASSERT(!memcmp(src_buf, dst_buf, strlen(dst_buf)));

                ret = fclose(stream);
                AOS_ASSERT(ret == AIC_SUCCESS);

                i++;
            }
        }
        aos_sleep(500);
    }
}

/**
 * fs_libc_test_init - fs libc test init.
 *
 * Parameter:
 *
 * Return:
 */
int fs_libc_test_init(void)
{
    fs_libc_test_thread_id = aos_create_thread("fs_libc_test_task",
                                                fs_libc_test_entry, AIC_NULL,
                                                FS_TEST_TASK_STACK_SIZE,
                                                0, AIC_NULL,
                                                FS_TEST_TASK_PRIORITY,
                                                AOS_AUTO_START);
    AOS_ASSERT(AOS_INVALID_THREAD_ID != fs_libc_test_thread_id);

    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_P1_INIT_EXPORT(fs_libc_test_init);

