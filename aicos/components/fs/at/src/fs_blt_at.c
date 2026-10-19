/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic fs blt at Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x056f1f
#define LOG_MODULE    AMOD_FS

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <aic_at_cmd_handle.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syslimits.h>
#include <errno.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define SD_TEST_FILE_NAME           "sd_blt.txt"

#define SD_TEST_BUF_LEN             100
#define SD_TEST_BUF                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"

#define SD_MOUNT_TARGET_NAME        "/D"

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_at_sd_test_cmd - sd test.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_sd_test_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    char read_buf[SD_TEST_BUF_LEN], *name = NULL;
    int errcode, fd = -1, ret;
    ssize_t rsize;

    do {
        errcode = AIC_SUCCESS;
        name = aos_zalloc(NAME_MAX);
        if (!name) {
            errcode = AIC_ENOMEM;
            break;
        }

        snprintf(name, NAME_MAX, "%s/%s",
                 SD_MOUNT_TARGET_NAME, SD_TEST_FILE_NAME);

        fd = open(name, O_RDWR | O_TRUNC | O_CREAT);
        if (fd < 0) {
            errcode = errno;
            break;
        }

        rsize = write(fd, SD_TEST_BUF, strlen(SD_TEST_BUF));
        if (rsize != strlen(SD_TEST_BUF)) {
            errcode = errno;
            break;
        }

        ret = lseek(fd, 0, SEEK_SET);
        if (ret != AIC_SUCCESS) {
            errcode = errno;
            break;
        }

        rsize = read(fd, read_buf, strlen(SD_TEST_BUF));
        if (rsize != strlen(SD_TEST_BUF)) {
            errcode = errno;
            break;
        }

        if (memcmp(read_buf, SD_TEST_BUF, strlen(SD_TEST_BUF))) {
            errcode = AIC_ENOEXEC;
            break;
        }

        ret = close(fd);
        if (ret != AIC_SUCCESS) {
            errcode = errno;
            break;
        }
        fd = -1;

        ret = unlink(name);
        if (ret != AIC_SUCCESS) {
            errcode = errno;
            break;
        }
    } while(0);

    if (fd >= 0)
        close(fd);

    if (name)
        aos_free(name);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/*
 *  Add fs blt AT handle here.
 */
const static AT_CMD_HANDLE fs_blt_at_handles[] =
{
    {"AT^SDTEST", NULL, NULL, fs_at_sd_test_cmd, NULL},
    {AIC_NULL},
};

/**
 * fs_blt_at_cmd_init - fs blt at cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int fs_blt_at_cmd_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fs_blt_at_handles);

    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_P2_INIT_EXPORT(fs_blt_at_cmd_init);

