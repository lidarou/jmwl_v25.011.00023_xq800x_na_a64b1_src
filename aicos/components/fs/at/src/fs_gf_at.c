/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic fs at Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10ca98
#define LOG_MODULE    AMOD_FS

#include <sys/_default_fcntl.h>
#include <sys/unistd.h>
#include <stdbool.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <alog_api.h>
#include <fs_api.h>
#include <aic_errno.h>
#include <aic_math.h>
#include <aic_at_resp_handle.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_process_common.h>
#include <aic_checksum.h>
#include <aic_serial_recv_send.h>
#include <aic_list.h>
#include <errno.h>
#include <slp_inc.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <aic_convert.h>
#include <cm_at_common.h>
#include <sys/mount.h>
#include <aos_utl.h>
#include <stdio.h>
#include <wdt.h>
#include <timer_inc.h>
#include "fs_at.h"
#include "fs_crc.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define FS_AT_MAX_VOLUME_NAME   10

#ifdef CONFIG_CUSTOMER_CM_AT
#define FS_AT_REEFIX_NAME       "+M"
#ifdef CONFIG_FS_FATFS
#define FS_AT_DEFAULT_VOLUME    "/C/home"
#else
#define FS_AT_DEFAULT_VOLUME    "/C"
#endif
#define FS_AT_MAX_NAME_LEN      128
#else
#define FS_AT_REEFIX_NAME       "+G"
#define FS_AT_DEFAULT_VOLUME    "/C"
#define FS_AT_MAX_NAME_LEN      256
#endif

#define FS_AT_SD_VOLUME         "/D"
#define FS_AT_ROMFS_VOLUME      "/O"
#define FS_AT_EXFS_VOLUME       "/E"

#define FS_AT_MAX_FRAG_SIZE     8192
#define FS_AT_MAX_RW_TEST_SIZE  512

#define FS_AT_ERROR_RESP        "+CME ERROR: "

#define FS_AT_CFG               "AT"""FS_AT_REEFIX_NAME"""FCFG"
#define FS_AT_INFO              "AT"""FS_AT_REEFIX_NAME"""FSINFO"
#define FS_AT_LIST              "AT"""FS_AT_REEFIX_NAME"""FLIST"
#define FS_AT_SIZE              "AT"""FS_AT_REEFIX_NAME"""FSIZE"
#define FS_AT_PUT               "AT"""FS_AT_REEFIX_NAME"""FPUT"
#define FS_AT_GET               "AT"""FS_AT_REEFIX_NAME"""FGET"
#define FS_AT_OPEN              "AT"""FS_AT_REEFIX_NAME"""FOPEN"
#define FS_AT_READ              "AT"""FS_AT_REEFIX_NAME"""FREAD"
#define FS_AT_WRITE             "AT"""FS_AT_REEFIX_NAME"""FWRITE"
#define FS_AT_SYNC              "AT"""FS_AT_REEFIX_NAME"""FSYNC"
#define FS_AT_SEEK              "AT"""FS_AT_REEFIX_NAME"""FSEEK"
#define FS_AT_TRUNC             "AT"""FS_AT_REEFIX_NAME"""FTRUNC"
#define FS_AT_CLOSE             "AT"""FS_AT_REEFIX_NAME"""FCLOSE"
#define FS_AT_DELETE            "AT"""FS_AT_REEFIX_NAME"""FDELETE"
#define FS_AT_MOVE              "AT"""FS_AT_REEFIX_NAME"""FMOVE"
#define FS_AT_CHECK             "AT"""FS_AT_REEFIX_NAME"""FCHECK"
#ifndef CONFIG_CUSTOMER_CM_AT
#define FS_AT_MKDIR             "AT"""FS_AT_REEFIX_NAME"""FMKDIR"
#define FS_AT_RMDIR             "AT"""FS_AT_REEFIX_NAME"""FRMDIR"
#define FS_AT_CD                "AT"""FS_AT_REEFIX_NAME"""FCD"
#define FS_AT_RENAME            "AT"""FS_AT_REEFIX_NAME"""FRENAME"
#define FS_AT_COPY              "AT"""FS_AT_REEFIX_NAME"""FCOPY"
#define FS_AT_FORMAT            "AT"""FS_AT_REEFIX_NAME"""FFORMAT"
#define FS_AT_RWTEST            "AT"""FS_AT_REEFIX_NAME"""FRWTEST"
#endif

#define FS_AT_CFG_RESP          FS_AT_REEFIX_NAME"""FCFG: "
#define FS_AT_INFO_RESP         FS_AT_REEFIX_NAME"""FSINFO: "
#define FS_AT_LIST_RESP         FS_AT_REEFIX_NAME"""FLIST: "
#define FS_AT_SIZE_RESP         FS_AT_REEFIX_NAME"""FSIZE: "
#define FS_AT_PUT_RESP          FS_AT_REEFIX_NAME"""FPUT: "
#define FS_AT_GET_RESP          FS_AT_REEFIX_NAME"""FGET: "
#define FS_AT_OPEN_RESP         FS_AT_REEFIX_NAME"""FOPEN: "
#define FS_AT_READ_RESP         FS_AT_REEFIX_NAME"""FREAD: "
#define FS_AT_WRITE_RESP        FS_AT_REEFIX_NAME"""FWRITE: "
#define FS_AT_SYNC_RESP         FS_AT_REEFIX_NAME"""FSYNC: "
#define FS_AT_SEEK_RESP         FS_AT_REEFIX_NAME"""FSEEK: "
#define FS_AT_TRUNC_RESP        FS_AT_REEFIX_NAME"""FTRUNC: "
#define FS_AT_CLOSE_RESP        FS_AT_REEFIX_NAME"""FCLOSE: "
#define FS_AT_DELETE_RESP       FS_AT_REEFIX_NAME"""FDELETE: "
#define FS_AT_MOVE_RESP         FS_AT_REEFIX_NAME"""FMOVE: "
#define FS_AT_CHECK_RESP        FS_AT_REEFIX_NAME"""FCHECK: "
#ifndef CONFIG_CUSTOMER_CM_AT
#define FS_AT_MKDIR_RESP        FS_AT_REEFIX_NAME"""FMKDIR: "
#define FS_AT_RMDIR_RESP        FS_AT_REEFIX_NAME"""FRMDIR: "
#define FS_AT_CD_RESP           FS_AT_REEFIX_NAME"""FCD: "
#define FS_AT_RENAME_RESP       FS_AT_REEFIX_NAME"""FRENAME: "
#define FS_AT_COPY_RESP         FS_AT_REEFIX_NAME"""FCOPY: "
#define FS_AT_FORMAT_RESP       FS_AT_REEFIX_NAME"""FFORMAT: "
#define FS_AT_RWTEST_RESP       FS_AT_REEFIX_NAME"""FRWTEST: "
#endif

enum FS_AT_ERRCODE {
    FS_AT_EINVAL    = 50,       /* Fs Invalid argument. */
    FS_AT_ERROR     = 1050,     /* Fs error. */
    FS_AT_ENOENT    = 1051,     /* No such file or directory. */
    FS_AT_EEXIST    = 1052,     /* File exists. */
    FS_AT_EFBIG     = 1053,     /* File too large */
    FS_AT_EMFILE    = 1054,     /* Too many open files */
    FS_AT_EBUSY     = 1055,     /* file had opened. */
    FS_AT_EOPEN     = 1056,     /* Open file error */
    FS_AT_EWRITE    = 1057,     /* Write file error */
    FS_AT_EREAD     = 1058,     /* Read file error */
    FS_AT_EROFS     = 1059,     /* Read-only file system */
    FS_AT_EBADF     = 1060,     /* Bad file number */
    FS_AT_EDEL      = 1061,     /* file delete error */
    FS_AT_ELS       = 1062,     /* file list error */
    FS_AT_ENOSPC    = 1063,     /* No space left on device */
    FS_AT_ETIME     = 1064,     /* timeout occurred */
};

enum FS_AT_INPUT_FORMAT {
    FS_AT_INPUT_ASCII           = 0,
    FS_AT_INPUT_HEX             = 1,
    FS_AT_INPUT_ESCAPE_CHAR     = 2,
    FS_AT_INPUT_MAX
};

enum FS_AT_OUTPUT_FORMAT {
    FS_AT_OUTPUT_ASCII      = 0,
    FS_AT_OUTPUT_HEX        = 1,
    FS_AT_OUTPUT_MAX
};

enum FS_AT_VERIFY_ALG {
    FS_AT_VERIFY_MD5        = 0,
    FS_AT_VERIFY_SHA        = 1,
    FS_AT_VERIFY_SHA256     = 2,
    FS_AT_VERIFY_CRC        = 3,
    FS_AT_VERIFY_MAX
};

enum FS_AT_RWTEST_EVENT {
    FS_AT_RWTEST_START      = BIT_0,
    FS_AT_RWTEST_STOP       = BIT_1,
};

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
struct fs_at_cfg {
    u16 timeout;            /* unit: s*/
    u32 frag_size;
    u32 frag_interval;      /* unit: ms*/
    u8 input_format;
    u8 output_format;
    u32 rwtest_length;
    u8 rwtest_path[FS_AT_MAX_NAME_LEN];
    u32 rwtest_interval;    /* unit: ms*/
    boolean rwtest_start;
    boolean ack;
};

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
static aos_timer_ptr fs_at_timer;
static boolean fs_at_timeout_flag = false;
static u32 fs_at_conn_id;

static struct fs_at_cfg s_fs_at_cfg =
{
    .timeout = 10,
    .frag_size = 0,
    .frag_interval = 0,
    .input_format = 0,
    .output_format = 0,
    .rwtest_length = FS_AT_MAX_RW_TEST_SIZE,
    .rwtest_path = FS_AT_DEFAULT_VOLUME,
    .rwtest_interval = 500,
    .rwtest_start = false,
    .ack = false,
};

LIST_HEAD(fs_at_open_list);

aos_event_ptr fs_at_rwtest_event;

char s_fs_at_volume_name[][6] = {"/C", "C:", "/D", "D:", "/O", "O:", "/E", "E:"};

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_at_is_rootpath - path is root path or not.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:
 *
 */
boolean fs_at_is_rootpath(char *path)
{
    u16 volume_len, volume_cnt, i;
    u16 path_len = strlen(path);

    volume_len = strlen(s_fs_at_volume_name[0]);
    volume_cnt = sizeof(s_fs_at_volume_name) / volume_len;

    if (path_len >= 2) {
        for (i = 0; i < volume_cnt; i++) {
            if (!memcmp(path, s_fs_at_volume_name[i], strlen(s_fs_at_volume_name[i]))) {
                if ((path_len == volume_len) || (path[volume_len] == '/')) {
                    return true;
                }
            }
        }
    }

    return false;
}

/**
 * fs_at_fullpath_get - get full path.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:
 *
 */
char *fs_at_fullpath_get(char *path)
{
    char *full_path, *ptr;
    u16 path_len;

    path_len = FS_AT_MAX_NAME_LEN + 10;
    if (fs_at_is_rootpath(path)) {
        return path;
    } else if (path[0] == '/') {
        full_path = aos_zalloc(path_len);
        snprintf(full_path, path_len, "%s%s",
                FS_AT_DEFAULT_VOLUME, path);
        return full_path;
    } else {
        full_path = aos_zalloc(path_len);
        ptr = getcwd(full_path, path_len);
        if (ptr == NULL) {
            snprintf(full_path, path_len, "%s/%s",
                    FS_AT_DEFAULT_VOLUME, path);
        } else {
            snprintf(full_path + strlen(full_path),
                    path_len - strlen(full_path), "/%s",
                    path);
        }
    }

    return full_path;
}

/**
 * fs_at_dir_lastseppos - get directory last separator'/' position.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:
 *  directory length;
 *
 */
int fs_at_dir_lastseppos_get(char *path)
{
    int pos, cnt;

    /* /C /C/ C: C:/ */
    if (strlen(path) <= 3)
        return 0;

    pos = 0;
    cnt = 0;
    while (path[cnt]) {
        if (path[cnt] == '/') {
            pos = cnt;
        }
        cnt++;
    }

    /* /C/txt.file C:/txt.file */
    if (pos <= 2)
        return 0;

    return pos;
}

/**
 * fs_map_err_ret - map to fs at error return.
 *
 * Parameter:
 *  @errcode: input error code.
 *
 * Return:
 *
 */
static int fs_at_err_map(int errcode)
{
    int ret;

    switch (errcode) {
        case AIC_ENOENT:
            ret  = FS_AT_ENOENT;
            break;
        case AIC_EEXIST:
            ret = FS_AT_EEXIST;
            break;
        case AIC_EFBIG:
            ret = FS_AT_EFBIG;
            break;
        case AIC_EMFILE:
            ret = FS_AT_EMFILE;
            break;
        case AIC_EBADF:
            ret = FS_AT_EBADF;
            break;
        case AIC_EROFS:
            ret = FS_AT_EROFS;
            break;
        case AIC_ENOSPC:
            ret = FS_AT_ENOSPC;
            break;
        case AIC_ETIME:
            ret = FS_AT_ETIME;
            break;
        case AIC_EINVAL:
            ret = FS_AT_EINVAL;
            break;
        case AIC_EBUSY:
            ret = FS_AT_EBUSY;
            break;
        default:
            ret = FS_AT_ERROR;
            break;
    }

    return ret;
}

/**
 * fs_at_timeout - fs at receive data timeout.
 *
 * Parameter:
 *  @input: input param..
 *
 * Return:
 *
 */
static void fs_at_timeout(void *input)
{
    u8 *ptr;

    fs_at_timeout_flag = true;

    ptr = aos_dma_malloc(12);
    strcpy(ptr, "+++");
    aic_serial_recv_callback(fs_at_conn_id, ptr, strlen(ptr), NULL);
}

/**
 * fs_at_loopfind_callback - loop find file callback.
 *
 * Parameter:
 * @find_info: find information.
 * @param: input callback param.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
static int fs_at_loopfind_callback(struct fs_loopfind_info *find_info)
{
    FS_AT_CMD *at_cmd = (FS_AT_CMD *)find_info->param;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 *result_line = NULL, *dir_path;
    int ret = AIC_SUCCESS;
    struct stat buf;

#ifdef CONFIG_CUSTOMER_CM_AT
    dir_path = find_info->fullpath + strlen(FS_AT_DEFAULT_VOLUME);
#else
    dir_path = find_info->fullpath;
#endif

    if (!memcmp(at_cmd->cmd, FS_AT_LIST_RESP, strlen(FS_AT_LIST_RESP)) &&
        strlen(at_cmd->cmd) == strlen(FS_AT_LIST_RESP)) {
        AIC_ASSERT(at_cmd->ptr);
        result_line = at_cmd->ptr;
        find_info->found_cnt++;

        if (find_info->dent->d_type & DT_DIR) {
            snprintf(result_line, NAME_MAX,
                    FS_AT_LIST_RESP"""\"%s\"",
                    dir_path);
        } else {
            ret = stat(find_info->fullpath, &buf);
            if (ret != 0) {
                ret = -errno;
                goto find_fail;
            }

            at_cmd->file_size += buf.st_size;
            snprintf(result_line, NAME_MAX,
                    FS_AT_LIST_RESP"""\"%s\",%ld",
                    dir_path, buf.st_size);
        }

        result_line[strlen(result_line)] = '\0';

        line_pointers[0] = (ulong)result_line;
        aic_fill_transmit_cmd_resp(at_cmd->at_req, line_pointers, 1);
    }

find_fail:
    return ret;
}

/**
 * fs_at_path_is_open - query path is open or not.
 *
 * Parameter:
 *  @path: at path.
 *
 * Return:
 *  1 is open, other not open.
 */
static boolean fs_at_path_is_open(char *path)
{
    struct list_head *p = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    boolean find = false;

    if (!list_empty(&fs_at_open_list)) {
        p = fs_at_open_list.next;
        while (p != &fs_at_open_list) {
            next = p->next;
            pEntry = list_entry(p, FS_AT_OPEN_FILE, node);
            if (strcmp(path, pEntry->name) == 0) {
                find = true;
                break;
            }
            p = next;
        }
    }

    return find;
}

/**
 * fs_at_fd_is_open - query fd is open or not.
 *
 * Parameter:
 *  @at_req: at request.
 *  @free: free fd.
 *
 * Return:
 *  1 is open, other not open.
 */
static boolean fs_at_fd_is_open(int fd, boolean free)
{
    struct list_head *p = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    boolean find = false;

    if (!list_empty(&fs_at_open_list)) {
        p = fs_at_open_list.next;
        while (p != &fs_at_open_list) {
            next = p->next;
            pEntry = list_entry(p, FS_AT_OPEN_FILE, node);
            if (pEntry->fd == fd) {
                find = true;
                if (free) {
                    list_del(&pEntry->node);
                    aos_free(pEntry->name);
                    aos_free(pEntry);
                }
                break;
            }
            p = next;
        }
    }

    return find;
}

/**
 * fs_at_cfg_cmd - response =?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_cfg_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u8 cnt = 0;

    line_pointers[cnt++] = (ulong)FS_AT_CFG_RESP"""\"timeout\",(1-120)";
    line_pointers[cnt++] = (ulong)FS_AT_CFG_RESP"""\"encoding\",(0-2),(0-1)";
    line_pointers[cnt++] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, cnt);
}

/**
 * fs_at_cfg_set_cmd - set fs at config.
 * format:
 *  "timeout"[,<timeout>]
 *  "encoding"[,<input_format>[,<output_format>]]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_cfg_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, timeout, input_format, output_format;
    static char *name = NULL;
    u16 line_count;
    boolean valid;
    int errcode;

    do {
        errcode = AIC_SUCCESS;
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_CFG"""=",
                                                 &count);
        if (!paras_arr || count < 1 || count > 3) {
            errcode = AIC_EINVAL;
            break;
        }

        name = (u8 *)paras_arr[0];

        if ((strlen("timeout") == strlen(name)) &&
            !memcmp("timeout", name, strlen(name))) {
            if (count == 1) {
                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                        FS_AT_CFG_RESP"""\"timeout\",%d",
                        s_fs_at_cfg.timeout);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
            } else if (count == 2) {
                para = (u8 *)paras_arr[1];
                valid = aic_is_valid_integer_para(para, strlen(para));
                if (!valid) {
                    errcode = AIC_EINVAL;
                    break;
                }
                timeout = atoi((u8 *)paras_arr[1]);
                if (timeout < 1 || timeout > 120) {
                    errcode = AIC_EINVAL;
                } else {
                    s_fs_at_cfg.timeout = timeout;
                    line_pointers[0] = (ulong)AT_RESULT_OK;
                    line_count = 1;
                }
            } else {
                errcode = AIC_EINVAL;
            }
        } else if ((strlen("encoding") == strlen(name)) &&
            !memcmp("encoding", name, strlen(name))) {
            if (count == 1) {
                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         FS_AT_CFG_RESP"""\"encoding\",%d,%d",
                         s_fs_at_cfg.input_format, s_fs_at_cfg.output_format);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
            } else if (count == 2) {
                para = (u8 *)paras_arr[1];
                valid = aic_is_valid_integer_para(para, strlen(para));
                if (!valid) {
                    errcode = AIC_EINVAL;
                    break;
                }
                input_format = atoi((u8 *)paras_arr[1]);
                if (input_format >= FS_AT_INPUT_MAX) {
                    errcode = AIC_EINVAL;
                } else {
                    s_fs_at_cfg.input_format = input_format;
                    line_pointers[0] = (ulong)AT_RESULT_OK;
                    line_count = 1;
                }
            } else {
                para = (u8 *)paras_arr[1];
                valid = aic_is_valid_integer_para(para, strlen(para));
                if (!valid) {
                    errcode = AIC_EINVAL;
                    break;
                }
                input_format = atoi((u8 *)paras_arr[1]);

                para = (u8 *)paras_arr[2];
                valid = aic_is_valid_integer_para(para, strlen(para));
                if (!valid) {
                    errcode = AIC_EINVAL;
                    break;
                }
                output_format = atoi((u8 *)paras_arr[2]);
                if ((input_format >= FS_AT_INPUT_MAX) ||
                    (output_format >= FS_AT_OUTPUT_MAX)) {
                    errcode = AIC_EINVAL;
                } else {
                    s_fs_at_cfg.input_format = input_format;
                    s_fs_at_cfg.output_format = output_format;
                    line_pointers[0] = (ulong)AT_RESULT_OK;
                    line_count = 1;
                }
            }
        } else {
            errcode = AIC_EINVAL;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d", fs_at_err_map(errcode));
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_info_set_cmd - get memory space information.
 * format:
 *  NULL: get default volume space.
 *  <volume>: get volume space.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_info_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u64 free_size, total_size;
    u16 count, line_count;
    int errcode;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                FS_AT_INFO"""=",
                                                &count);
        if ((count != 0) && (count != 1)) {
            errcode = AIC_EINVAL;
            break;
        }

        if (count == 0) {
            name = FS_AT_DEFAULT_VOLUME;
        } else {
            name = (char *)paras_arr[0];
        }

        errcode = fs_getdiskfreespace(name, &free_size, &total_size);
        if (errcode < 0)
            errcode = -errno;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        if (total_size > 0x7FFFFFFF) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    FS_AT_INFO_RESP"""%lld,%lld", total_size, free_size);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                    FS_AT_INFO_RESP"""%ld,%ld", (long int)total_size, (long int)free_size);
        }
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_list_set_cmd - list path file information.
 * format:
 *  NULL: list default volume file information.
 *  <path>: list path file information.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_list_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    struct fs_loopfind_info *findinfo;
    FS_AT_CMD *at_cmd = NULL;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_LIST"""=",
                                                 &count);
        if (count == 0) {
            tmpname = FS_AT_DEFAULT_VOLUME;
        } else if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        } else {
            tmpname = (char *)paras_arr[0];
        }

        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN) {
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        at_cmd = aos_zalloc(sizeof(FS_AT_CMD));
        at_cmd->at_req = at_req;
        at_cmd->cmd = FS_AT_LIST_RESP;
        at_cmd->ptr = aos_zalloc(NAME_MAX);
        AIC_ASSERT(at_cmd->ptr);

        if ((name[strlen(name) - 1] == '*') && (name[strlen(name) - 2] == '*')) {
            at_cmd->prefix = true;
        } else {
            at_cmd->prefix = false;
        }

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);
        findinfo->findtype = FS_FIND_CURDIR;
        findinfo->dir = name;
        findinfo->callback = (u32)fs_at_loopfind_callback;
        findinfo->param = (void *)at_cmd;
        findinfo->found_cnt = 0;
        errcode = fs_loopfind(name, findinfo);

        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
        } else if (findinfo->found_cnt == 0)  {
            errcode = FS_AT_ENOENT;
        }

        aos_free(findinfo);
        aos_free(at_cmd->ptr);
        aos_free(at_cmd);
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_size_set_cmd - get file size.
 * format:
 *  <fname>: get file size.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_size_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    u16 count, line_count;
    struct stat st;
    int errcode;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_SIZE"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        errcode = stat(name, &st);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_SIZE_RESP"""%lu", st.st_size);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * at_get_para_array - get parameter array.
 *
 * Parameter:
 *
 * Return:
 *  array, fail return NULL.
 */
static ulong *at_get_para_array(u8 *raw_str, u8 *start_str, u16 *paras_cnt,
                              u16 max_cnt, u16 data_index)
{
    u16 raw_str_len, para_str_len;
    u8 cnt = 0, i;
    ulong *str_arr = AIC_NULL;
    u8 *str_pointer[30] = {0};
    u8 *paras_ptr;
    u8 *work_ptr;
    u8 *found;
    int offset_len = 0;
    u8 at_end_flag_len = 0;

    if (!raw_str || !start_str)
        return AIC_NULL;

    if (strlen(raw_str) <= strlen(start_str) || !strcasestr(raw_str, start_str)) {
        aicx_info(0x111bed, "", "[MF] split paras error, no paras\n");
        *paras_cnt = 0;
        return AIC_NULL;
    }

    raw_str_len = strlen(raw_str);
    if (!is_cr_lf_sign(raw_str[raw_str_len - 1])) {
        aicx_info(0x07ff47, "", "[MF] not found the AT cmd end\n");
        *paras_cnt = 0;
        return AIC_NULL;
    } else
        at_end_flag_len ++;

    if (is_cr_lf_sign(raw_str[raw_str_len - 2]))
        at_end_flag_len ++;

    paras_ptr = aos_zalloc(raw_str_len + 1);

    strncpy(paras_ptr, raw_str + strlen(start_str),
            (raw_str_len - at_end_flag_len - strlen(start_str)));
    para_str_len = strlen(paras_ptr);

    work_ptr = paras_ptr;
    do {
        if (data_index && cnt == data_index - 1) {
            str_pointer[cnt] = aos_zalloc(strlen(work_ptr) + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            if (work_ptr[0] == '"') {
                //the quotation marks(") should be included both the begin & end
                if (work_ptr[strlen(work_ptr) - 1] != '"') {
                    aicx_error(0x07608a, "", "[MF] not found other quotation in last data\n");
                    *paras_cnt = 0;
                    goto SPLIT_ERROR;
                }
                strncpy(str_pointer[cnt], work_ptr + 1, strlen(work_ptr) - 2);
            } else {
                goto SPLIT_ERROR;
            }
            cnt++;
            break;
        }

        found = strchr(work_ptr, ',');
        if (found) {
            offset_len = para_str_len - strlen(found);
            para_str_len = para_str_len - offset_len - 1;

            str_pointer[cnt] = aos_zalloc(offset_len + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            //parameter with quotes("), remove commas(,)
            if (work_ptr[0] == '"') {
                //the quotation marks(") should be included both the begin & end
                if (work_ptr[offset_len - 1] != '"') {
                    aicx_error(0x0eeaad, "", "[MF] not found other quotation\n");
                    *paras_cnt = 0;
                    goto SPLIT_ERROR;
                }
                strncpy(str_pointer[cnt], work_ptr + 1, offset_len - 2);
            } else {
                strncpy(str_pointer[cnt], work_ptr, offset_len);
            }

            work_ptr = work_ptr + offset_len + 1;
            found++;
            cnt++;
            if (strncmp(found, ",", 1) == 0) {
                /* Two continuous ',',that means there is an empty para. */
                str_pointer[cnt] = aos_zalloc(2);
                if (!str_pointer[cnt])
                    goto SPLIT_ERROR;
                strncpy((u8 *)str_pointer[cnt], "", 1);
                cnt++;
                found++;
                para_str_len--;
                work_ptr++;
            }
        } else {
            str_pointer[cnt] = aos_zalloc(strlen(work_ptr) + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            if (work_ptr[0] == '"') {
                //the quotation marks(") should be included both the begin & end
                if (work_ptr[strlen(work_ptr) - 1] != '"') {
                    aicx_info(0x001d91, "", "[MF] not fount other quotation 333\n");
                    *paras_cnt = 0;
                    goto SPLIT_ERROR;
                }

                strncpy(str_pointer[cnt], work_ptr + 1, strlen(work_ptr) - 2);
            } else {
                strncpy(str_pointer[cnt], work_ptr, strlen(work_ptr));
            }

            cnt++;
            break;
        }
    } while (cnt < max_cnt);

    if (cnt > 0) {
        str_arr = (ulong *)aos_malloc(sizeof(ulong) * cnt);
        if (!str_arr) {
            aicx_error(0x10cd73, "", "[MF]: malloc str_arr fail\n");
            goto SPLIT_ERROR;
        }

        for (i = 0; i < cnt; i++) {
            str_arr[i] = (ulong)str_pointer[i];
            aicx_info(0x009305, "ds", "[MF] str_arr[%d] is %s\n", i, str_arr[i]);
        }
    }
    *paras_cnt = cnt;
    aos_free(paras_ptr);

    return str_arr;

SPLIT_ERROR:
    *paras_cnt = 0;

    aos_free(paras_ptr);
    for (i = 0; i < cnt; i++) {
        aos_free(str_pointer[i]);
    }

    return AIC_NULL;
}

/**
 * fs_at_put_set_cmd - upload a file to storage.
 * format:
 *  <fname>,<fsize>[,<data>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_put_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL, *ptr = NULL, *para;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    static u32 filesize, recvlen, offset, frag_size;
    static char *filename = NULL, *recvdata = NULL;
    ssize_t rsize, wsize, datalen;
    u64 free_bytes, total_bytes;
    u16 count = 0, line_count;
    boolean valid, convert;
    char *tmpname = NULL;
    int dirpos, errcode;
    struct statfs stfs;
    struct stat buf;
    static int fd;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;

    do {
        errcode = AIC_SUCCESS;
        if (at_req->at_mode == AT_CMD_MODE) {
            fd = -1;
            recvlen = 0;
            offset = 0;
            paras_arr = at_get_para_array(at_req->at_req,
                                        FS_AT_PUT"""=",
                                        &count, 3, 3);
            if (!paras_arr || count < 1 || count > 3) {
                errcode = AIC_EINVAL;
                break;
            }

            tmpname = (char *)paras_arr[0];
            if (strlen(tmpname) > FS_AT_MAX_NAME_LEN) {
                errcode = AIC_EINVAL;
                break;
            }

            if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
                errcode = FS_AT_EBUSY;
                break;
            }

            filename = fs_at_fullpath_get(tmpname);
            if (filename && ((u32)tmpname == (u32)filename)) {
                filename = aos_zalloc(FS_AT_MAX_NAME_LEN + 4);
                aos_memcpy(filename, tmpname, strlen(tmpname));
            }

            filesize = 0;
            if (count >= 2) {
                para = (u8 *)paras_arr[1];
                if (strlen(para)) {
                    valid = aic_is_valid_integer_para(para, strlen(para));
                    if (!valid) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    filesize = atoi((u8 *)paras_arr[1]);
                } else {
                    filesize = 0;
                }
            }

            dirpos = fs_at_dir_lastseppos_get(filename);
            if (dirpos) {
                filename[dirpos] = '\0';
                errcode = mkdir(filename, O_RDWR);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }
                filename[dirpos] = '/';
            }

            errcode = statfs(filename, &stfs);
            if (errcode != AIC_SUCCESS) {
                errcode = -errno;
                break;
            }

            if (stfs.f_bfree == 0) {
                errcode = AIC_ENOSPC;
                break;
            }

            errcode = stat(filename, &buf);
            if (errcode == 0) {
                errcode = unlink(filename);
                if (errcode != 0) {
                    errcode = -errno;
                    break;
                }
            } else {
                errcode = AIC_SUCCESS;
            }

            fd = open(filename, O_RDWR | O_CREAT);
            if (fd < 0) {
                errcode = -errno;
                break;
            }

            if (count == 3) {
                errcode = fs_getdiskfreespace(filename, &free_bytes, &total_bytes);
                if (errcode < 0) {
                    errcode = -errno;
                    break;
                }

                data = (u8 *)paras_arr[2];

                if (s_fs_at_cfg.input_format == FS_AT_INPUT_HEX) {
                    if (strlen(data) % 2) {
                        /* invalid hex length */
                        errcode = AIC_EINVAL;
                        break;
                    }

                    ptr = aos_zalloc(strlen(data) + 4);
                    AIC_ASSERT(ptr);

                    convert = string2hex(data, ptr, strlen(data));
                    if (!convert) {
                        aos_free(ptr);
                        errcode = AIC_ENOEXEC;
                        break;
                    }

                    datalen = strlen(data) / 2;
                    if (filesize && (filesize != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    wsize = datalen > free_bytes ? free_bytes : datalen;

                    if (wsize) {
                        rsize = write(fd, ptr, wsize);
                    } else {
                        rsize = 0;
                    }

                    aos_free(ptr);
                } else if (s_fs_at_cfg.input_format == FS_AT_INPUT_ESCAPE_CHAR) {
                    datalen = strlen(data);
                    if (filesize && (filesize != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }

                    ptr = aos_zalloc(strlen(data) + 4);
                    AIC_ASSERT(ptr);

                    if (datalen) {
                        aic_escape_data_normal(ptr, data,
                                            (int)datalen,
                                            (int *)&wsize);
                        wsize = wsize > free_bytes ? free_bytes : wsize;
                        if (wsize) {
                            rsize = write(fd, ptr, wsize);
                        } else {
                            rsize = 0;
                        }
                    } else {
                        wsize = rsize = 0;
                    }
                    aos_free(ptr);
                } else {
                    datalen = strlen(data);
                    if (filesize && (filesize != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    wsize = datalen > free_bytes ? free_bytes : datalen;

                    if (wsize) {
                        rsize = write(fd, data, wsize);
                    } else {
                        rsize = 0;
                    }
                }
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                goto MFPUT_DONE;
            } else {
                frag_size = filesize;
                if (filesize) {
                    recvdata = aos_zalloc_relaxed(filesize);
                }

                if (recvdata == NULL) {
                    recvdata = aos_zalloc(FS_AT_MAX_FRAG_SIZE);
                    frag_size = FS_AT_MAX_FRAG_SIZE;
                }
            }

            aos_change_timer(fs_at_timer,
                            s_fs_at_cfg.timeout * 1000,
                            false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            aic_get_wakelock();
            at_req->at_mode = AT_DATA_MODE;
            line_pointers[0] = (ulong)">";
            line_count = 1;
        } else if (at_req->at_upstream_data != AIC_NULL &&
                   at_req->at_mode == AT_DATA_MODE) {
#ifdef CONFIG_CUSTOMER_CM_AT
            if (fs_at_timeout_flag) {
#else
            if (fs_at_timeout_flag ||
                strncmp(at_req->at_upstream_data, "+++", 3) == 0) {
#endif
MFPUT_DONE:
                if (offset) {
                    rsize = write(fd, recvdata, offset);
                    offset = 0;
                    if (rsize < 0) {
                        errcode = -errno;
                        break;
                    }
                }

                errcode = fsync(fd);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }

                errcode = fstat(fd, &buf);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }

                errcode = close(fd);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }
                fd = -1;

                at_req->at_mode = AT_CMD_MODE;

                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         FS_AT_PUT_RESP"""%ld", buf.st_size);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                aos_free(filename);
                filename = NULL;
                if (recvdata) {
                    aos_free(recvdata);
                    recvdata = NULL;
                }
                aic_put_wakelock();
                break;
            }

            rsize = 0;
            data = at_req->at_upstream_data;
            datalen = at_req->upstream_data_len;
            recvlen += datalen;
            if ((filesize > 0) && (recvlen > filesize)) {
                wsize = datalen - (recvlen - filesize);
            } else {
                wsize = datalen;
            }

            errcode = fs_getdiskfreespace(filename, &free_bytes, &total_bytes);
            if (errcode < 0) {
                errcode = -errno;
                break;
            }
            wsize = wsize > (free_bytes - offset) ? (free_bytes - offset) : wsize;

            if (wsize >= frag_size) {
                if (offset) {
                    rsize = write(fd, recvdata, offset);
                    offset = 0;
                    if (rsize < 0) {
                        errcode = -errno;
                        break;
                    }
                }

                rsize = write(fd, data, wsize);
                offset = 0;
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }

                #ifdef CONFIG_DRIVER_WDT
                aic_watchdog_ping(NULL);
                #endif
            } else if ((offset + wsize) == frag_size) {
                aos_memcpy(recvdata + offset, data, wsize);
                offset += wsize;
                rsize = write(fd, recvdata, offset);
                offset = 0;
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }

                #ifdef CONFIG_DRIVER_WDT
                aic_watchdog_ping(NULL);
                #endif
            } else if ((offset + wsize) > frag_size) {
                rsize = write(fd, recvdata, offset);
                offset = 0;
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                aos_memcpy(recvdata + offset, data, wsize);
                offset += wsize;

                #ifdef CONFIG_DRIVER_WDT
                aic_watchdog_ping(NULL);
                #endif
            } else if (wsize) {
                aos_memcpy(recvdata + offset, data, wsize);
                offset += wsize;
            }

            if ((filesize && (recvlen >= filesize))) {
                goto MFPUT_DONE;
            }

            aos_change_timer(fs_at_timer, s_fs_at_cfg.timeout * 1000, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            errcode = AIC_SUCCESS;
        } else {
            errcode = AIC_EPERM;
            break;
        }

    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;

        if (at_req->at_mode != AT_CMD_MODE) {
            at_req->at_mode = AT_CMD_MODE;
        }

        if (fd >= 0) {
            close(fd);
            fd = -1;
        }

        if (filename) {
            aos_free(filename);
            filename = NULL;
        }

        if (recvdata) {
            aos_free(recvdata);
            recvdata = NULL;
        }
        aic_put_wakelock();
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);

    if (line_count) {
        errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
    }

    return errcode;
}

/**
 * fs_at_get_set_cmd - download a file from storage.
 * format:
 *  <fname>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_get_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 *result_line = NULL, *data = NULL, *ptr = NULL;
    char *filename = NULL, *tmpname = NULL;
    int fd = -1, errcode, i;
    u32 rspsize, frag_size;
    u16 count, line_count;
    struct stat st;
    ssize_t rsize;
    char tmp[5];

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                FS_AT_GET"""=",
                                                &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        filename = fs_at_fullpath_get(tmpname);
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            errcode = -errno;
            break;
        }

        errcode = fstat(fd, &st);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }

        if (!st.st_size) {
            if (result_line == NULL) {
                rspsize = 100;
                result_line = aos_zalloc(100);
                AIC_ASSERT(result_line);
            }
            snprintf(result_line, rspsize, FS_AT_GET_RESP"""%ld,", st.st_size);
            line_pointers[0] = (ulong)result_line;
            aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            errcode = AIC_SUCCESS;
            break;
        }

        frag_size = s_fs_at_cfg.frag_size ? s_fs_at_cfg.frag_size : st.st_size;

        data = aos_zalloc(frag_size + 4);
        if (data == NULL) {
            errcode = AIC_ENOMEM;
            break;
        }

        if (s_fs_at_cfg.output_format == FS_AT_OUTPUT_HEX) {
            ptr = aos_zalloc(frag_size * 2 + 4);
            if (ptr == NULL) {
                errcode = AIC_ENOMEM;
                break;
            }
        }

        if (s_fs_at_cfg.output_format == FS_AT_OUTPUT_HEX) {
            rspsize = frag_size * 2 + 100;
        } else {
            rspsize = frag_size + 100;
        }
        result_line = aos_zalloc(rspsize);
        AIC_ASSERT(result_line);

        aos_memset(tmp, 0x0, sizeof(tmp));
        while (st.st_size) {
            rsize = st.st_size > frag_size ? frag_size : st.st_size;

            aos_memset(data, 0x0, frag_size + 4);
            rsize = read(fd, data, rsize);
            if (rsize < 0) {
                errcode = -errno;
                break;
            }

            if (s_fs_at_cfg.output_format == FS_AT_OUTPUT_HEX) {
                hex2string(data, ptr, rsize);
                snprintf(result_line, rspsize, FS_AT_GET_RESP"""%d,%s", rsize, ptr);
                line_pointers[0] = (ulong)result_line;
                aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            } else {
                snprintf(result_line, rspsize, FS_AT_GET_RESP"""%d,", rsize);
                i = strlen(result_line);
                aos_memcpy(result_line + i, data, rsize);
                aic_transmit_at_result(at_req->serial_port_id, result_line, i + rsize);
            }

            st.st_size -= rsize;
            if (s_fs_at_cfg.frag_interval)
                aos_sleep(s_fs_at_cfg.frag_interval);
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        if (result_line == NULL) {
            rspsize = 100;
            result_line = aos_zalloc(100);
            AIC_ASSERT(result_line);
        }
        snprintf(result_line, rspsize, FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if (fd >= 0)
        fs_close(fd);

    if (data)
        aos_free(data);

    if (ptr)
        aos_free(ptr);

    if (filename && ((u32)tmpname != (u32)filename)) {
        aos_free(filename);
    }

    aic_free_paras_mem(paras_arr, count);
    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    aos_free(result_line);
    return errcode;
}

/**
 * fs_at_open_query_cmd - query open list.
 * format:
 *  =?
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_open_query_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct list_head *pos = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    u8 *result_line = NULL;
    int errcode;
    u16 rsplen;

    rsplen = NAME_MAX + 50;
    result_line = aos_zalloc(rsplen);
    AIC_ASSERT(result_line);

    errcode = AIC_SUCCESS;

    list_for_each_safe(pos, next, &fs_at_open_list) {
        pEntry = list_entry(pos, FS_AT_OPEN_FILE, node);
        snprintf(result_line, rsplen,
                 "%s,%u,%d", pEntry->name, pEntry->fd, pEntry->mode);
        aicx_info(0x1cdf2e, "s", "%s.\n", result_line);
        result_line[strlen(result_line)] = '\0';
        line_pointers[0] = (ulong)result_line;
        errcode = aic_fill_transmit_cmd_resp(at_req,
                                            line_pointers,
                                            1);
        if (errcode != AIC_SUCCESS)
            break;
    }

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    errcode = aic_fill_transmit_cmd_resp(at_req,
                                        line_pointers,
                                        1);

    if (result_line)
        aos_free(result_line);

    return errcode;
}

/**
 * fs_at_open_set_cmd - open a file.
 * format:
 *  <fname>,<mode>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_open_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    char *name = NULL, *tmpname = NULL;
    FS_AT_OPEN_FILE *open_file;
    u16 count, line_count;
    int fd, flags, mode;
    int errcode, dirpos;
    boolean valid;

    do {
        errcode = AIC_SUCCESS;

        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                FS_AT_OPEN"""=",
                                                &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        para = (u8 *)paras_arr[1];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }
        mode = atoi((u8 *)paras_arr[1]);

        switch (mode) {
            case 0:
                flags = O_RDONLY;
                break;
            case 1:
                flags = O_WRONLY | O_TRUNC | O_CREAT;
                break;
            case 2:
                flags = O_WRONLY | O_APPEND | O_CREAT;
                break;
            case 3:
                flags = O_RDWR | O_TRUNC | O_CREAT;
                break;
            case 4:
                flags = O_RDWR | O_APPEND | O_CREAT;
                break;
            default:
                flags = O_RDONLY;
                break;
        }

        if (mode > 4) {
            errcode = AIC_EINVAL;
            break;
        }

        if (flags & O_CREAT) {
            dirpos = fs_at_dir_lastseppos_get(name);
            if (dirpos) {
                name[dirpos] = '\0';
                errcode = mkdir(name, O_RDWR);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }
                name[dirpos] = '/';
            }
        }

        valid = fs_at_path_is_open(name);
        if (valid) {
            errcode = FS_AT_EBUSY;
            break;
        }

        fd = open(name, flags);
        if (fd < 0) {
            errcode = -errno;
            break;
        }

        errcode = fsync(fd);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }

        open_file = aos_zalloc(sizeof(FS_AT_OPEN_FILE));
        open_file->fd = fd;
        open_file->name = aos_zalloc(strlen(name) + 4);
        aos_memcpy(open_file->name, name, strlen(name));
        open_file->mode = mode;
        list_add(&open_file->node, &fs_at_open_list);

        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, FS_AT_OPEN_RESP"""%u", fd);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_read_set_cmd - read data from file.
 * format:
 *  <fhandle>[,<length>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_read_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 *result_line = NULL, *data = NULL, *ptr = NULL, *para;
    ssize_t rsize, tsize, asize;
    u32 rspsize, frag_size;
    u16 count, line_count;
    int fd, errcode, i;
    struct stat st;
    boolean valid;
    ulong length;
    char tmp[5];
    off_t pos;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                FS_AT_READ"""=",
                                                &count);
        if (!paras_arr || count < 1 || count > 2) {
            errcode = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }
        fd = atoi((u8 *)paras_arr[0]);
        if ((fd < 0) || (fd > 0xFFFF)) {
            errcode = AIC_EINVAL;
            break;
        }
        if (!fs_at_fd_is_open(fd, false)) {
            errcode = AIC_EBADF;
            break;
        }

        if (count == 2) {
            para = (u8 *)paras_arr[1];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            length = atoi((u8 *)paras_arr[1]);
        } else {
            length = 0xFFFFFFFF;
        }

        errcode = fstat(fd, &st);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }

        if (st.st_size == 0) {
            errcode = FS_AT_ERROR;
            break;
        }

        frag_size = s_fs_at_cfg.frag_size ? s_fs_at_cfg.frag_size : st.st_size;

        if (s_fs_at_cfg.output_format == FS_AT_OUTPUT_HEX) {
            rspsize = frag_size * 2 + 100;
        } else {
            rspsize = frag_size + 100;
        }
        result_line = aos_zalloc(rspsize);
        AIC_ASSERT(result_line);

        data = aos_zalloc(frag_size + 4);
        AIC_ASSERT(data);

        pos = fs_getpos(fd);

        if (st.st_size >= pos) {
            st.st_size -= pos;
            if (length == 0xFFFFFFFF) {
                length = st.st_size;
            } else if (length > st.st_size) {
                errcode = FS_AT_EREAD;
                break;
            }
        } else {
            rsize = read(fd, data, frag_size);
            if (rsize < 0) {
                errcode = -errno;
                if (errcode == AIC_EACCES)
                    errcode = FS_AT_EREAD;
                break;
            }
            // snprintf(result_line, rspsize, FS_AT_READ_RESP"""%d,%ld,0,0,0", fd, length == 0xFFFFFFFF ? 0 : length);
            // line_pointers[0] = (ulong)result_line;
            // line_count = 1;
            // aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
            errcode = AIC_SUCCESS;
            break;
        }

        if (s_fs_at_cfg.output_format == FS_AT_OUTPUT_HEX) {
            ptr = aos_zalloc(frag_size * 2 + 4);
            AIC_ASSERT(ptr);
        }

        asize = 0;
        tsize = length;
        aos_memset(tmp, 0x0, sizeof(tmp));
        while (st.st_size && length) {
            aos_memset(data, 0x0, frag_size + 4);
            rsize = read(fd, data, frag_size > length ? length : frag_size);
            if (rsize < 0) {
                errcode = -errno;
                if (errcode == AIC_EACCES)
                    errcode = FS_AT_EREAD;
                break;
            }

            if (rsize == 0) {
                errcode = AIC_SUCCESS;
                break;
            }

            length -= rsize;
            asize += rsize;
            st.st_size -= rsize;

            if (s_fs_at_cfg.output_format == FS_AT_OUTPUT_HEX) {
                hex2string(data, ptr, rsize);
                snprintf(result_line, rspsize, FS_AT_READ_RESP"""%d,%d,%d,%d,%s",
                         fd, tsize, asize, rsize, ptr);
                line_pointers[0] = (ulong)result_line;
                errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            } else {
                snprintf(result_line, rspsize, FS_AT_READ_RESP"""%d,%d,%d,%d,",
                         fd, tsize, asize, rsize);
                i = strlen(result_line);
                aos_memcpy(result_line + i, data, rsize);
                errcode = aic_transmit_at_result(at_req->serial_port_id, result_line, i + rsize);
            }

            if (errcode != AIC_SUCCESS) {
                aicx_error(0x14276a, "d", "resp at fail, errcode=%d.\n", errcode);
                break;
            }
            if (s_fs_at_cfg.frag_interval)
                aos_sleep(s_fs_at_cfg.frag_interval);
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        if (result_line == NULL) {
            rspsize = 100;
            result_line = aos_zalloc(rspsize);
            AIC_ASSERT(result_line);
        }
        snprintf(result_line, rspsize,
                FS_AT_ERROR_RESP"""%d",
                errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if(data)
        aos_free(data);

    if(ptr)
        aos_free(ptr);

    aic_free_paras_mem(paras_arr, count);
    errcode =  aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    aos_free(result_line);
    return errcode;
}

/**
 * fs_at_write_set_cmd - write data to file.
 * format:
 *  <fhandle>,<length>[,<data>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_write_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL, *ptr = NULL, *para;
    static u32 length, recvlen, wlength, offset, frag_size;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct list_head *p = NULL, *next;
    static FS_AT_OPEN_FILE *pEntry;
    ssize_t rsize, wsize, datalen;
    static char *recvdata = NULL;
    u64 free_bytes, total_bytes;
    u16 count = 0, line_count;
    ulong *paras_arr = NULL;
    boolean valid, convert;
    boolean find = false;
    static int fd;
    int errcode;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;
    errcode = AIC_SUCCESS;

    do {
        if (at_req->at_mode == AT_CMD_MODE) {
            fd = -1;
            recvlen = 0;
            wlength = 0;
            offset = 0;
            paras_arr = at_get_para_array(at_req->at_req,
                                        FS_AT_WRITE"""=",
                                        &count, 3, 3);
            if (!paras_arr || count < 1 || count > 3) {
                errcode = AIC_EINVAL;
                break;
            }

            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            fd = atoi((u8 *)paras_arr[0]);
            if ((fd < 0) || (fd > 0xFFFF)) {
                errcode = AIC_EINVAL;
                break;
            }

            find = false;
            if (!list_empty(&fs_at_open_list)) {
                p = fs_at_open_list.next;
                while (p != &fs_at_open_list) {
                    next = p->next;
                    pEntry = list_entry(p, FS_AT_OPEN_FILE, node);
                    if (pEntry->fd == fd) {
                        find = true;
                        break;
                    }
                    p = next;
                }
            }
            if (!find) {
                errcode = AIC_EBADF;
                break;
            }

            length = 0;
            if (count >= 2) {
                para = (u8 *)paras_arr[1];
                if (strlen(para)) {
                    valid = aic_is_valid_integer_para(para, strlen(para));
                    if (!valid) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    length = atoi((u8 *)paras_arr[1]);
                }
            }

            errcode = fs_getdiskfreespace(pEntry->name, &free_bytes, &total_bytes);
            if (errcode < 0) {
                break;
            }

            if (free_bytes == 0) {
                errcode = AIC_ENOSPC;
                break;
            }

            if (count == 3) {
                data = (u8 *)paras_arr[2];

                if (s_fs_at_cfg.input_format == FS_AT_INPUT_HEX) {
                    if (strlen(data) % 2) {
                        errcode = AIC_EINVAL;
                        break;
                    }

                    ptr = aos_zalloc(strlen(data) + 4);
                    AIC_ASSERT(ptr);

                    convert = string2hex(data, ptr, strlen(data));
                    if (!convert) {
                        aos_free(ptr);
                        errcode = AIC_ENOEXEC;
                        break;
                    }

                    datalen = strlen(data) / 2;
                    if (length && (length != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    wsize = datalen > free_bytes ? free_bytes : datalen;

                    if (wsize) {
                        rsize = write(fd, ptr, wsize);
                    } else {
                        rsize = 0;
                    }
                    aos_free(ptr);
                } else if (s_fs_at_cfg.input_format == FS_AT_INPUT_ESCAPE_CHAR) {
                    datalen = strlen(data);
                    if (length && (length != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }

                    ptr = aos_zalloc(strlen(data) + 4);
                    AIC_ASSERT(ptr);

                    aic_escape_data_normal(ptr, data, (int)datalen, (int *)&wsize);
                    wsize = wsize > free_bytes ? free_bytes : wsize;
                    if (wsize) {
                        rsize = write(fd, ptr, wsize);
                    } else {
                        rsize = 0;
                    }
                    aos_free(ptr);
                } else {
                    datalen = strlen(data);
                    if (length && (length != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    wsize = datalen > free_bytes ? free_bytes : datalen;
                    if (wsize) {
                        rsize = write(fd, data, wsize);
                    } else {
                        rsize = 0;
                    }
                }
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                wlength = rsize;
                goto MFWRITE_DONE;
            } else {
                frag_size = length;
                if (length) {
                    recvdata = aos_zalloc_relaxed(length);
                }

                if (recvdata == NULL) {
                    recvdata = aos_zalloc(FS_AT_MAX_FRAG_SIZE);
                    frag_size = FS_AT_MAX_FRAG_SIZE;
                }
            }

            aos_change_timer(fs_at_timer, s_fs_at_cfg.timeout * 1000, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            aic_get_wakelock();
            at_req->at_mode = AT_DATA_MODE;
            line_pointers[0] = (ulong)">";
            line_count = 1;
            errcode = AIC_SUCCESS;
        } else if (at_req->at_upstream_data != AIC_NULL &&
                   at_req->at_mode == AT_DATA_MODE) {
#ifdef CONFIG_CUSTOMER_CM_AT
            if (fs_at_timeout_flag) {
#else
            if (fs_at_timeout_flag ||
                strncmp(at_req->at_upstream_data, "+++", 3) == 0) {
#endif
MFWRITE_DONE:
                if (offset) {
                    rsize = write(fd, recvdata, offset);
                    offset = 0;
                    if (rsize < 0) {
                        errcode = -errno;
                        break;
                    }
                    wlength += rsize;
                }

                errcode = fsync(fd);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }

                if (recvdata) {
                    aos_free(recvdata);
                    recvdata = NULL;
                }

                at_req->at_mode = AT_CMD_MODE;
        
                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         FS_AT_WRITE_RESP"""%d,%d", fd, wlength);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                errcode = AIC_SUCCESS;
                aic_put_wakelock();
                break;
            }

            rsize = 0;
            data = at_req->at_upstream_data;
            datalen = at_req->upstream_data_len;
            recvlen += datalen;
            if ((length > 0) && (recvlen > length)) {
                wsize = datalen - (recvlen - length);
            } else {
                wsize = datalen;
            }

            errcode = fs_getdiskfreespace(pEntry->name,
                                        &free_bytes, &total_bytes);
            if (errcode < 0) {
                break;
            }
            wsize = wsize > (free_bytes - offset) ? (free_bytes - offset) : wsize;

            if (wsize >= frag_size) {
                if (offset) {
                    rsize = write(fd, recvdata, offset);
                    offset = 0;
                    if (rsize < 0) {
                        errcode = -errno;
                        break;
                    }
                    wlength += rsize;
                }

                rsize = write(fd, data, wsize);
                offset = 0;
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                wlength += rsize;

                #ifdef CONFIG_DRIVER_WDT
                aic_watchdog_ping(NULL);
                #endif
            } else if ((offset + wsize) == frag_size) {
                aos_memcpy(recvdata + offset, data, wsize);
                offset += wsize;
                rsize = write(fd, recvdata, offset);
                offset = 0;
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                wlength += rsize;

                #ifdef CONFIG_DRIVER_WDT
                aic_watchdog_ping(NULL);
                #endif
            } else if ((offset + wsize) > frag_size) {
                rsize = write(fd, recvdata, offset);
                offset = 0;
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                wlength += rsize;
                aos_memcpy(recvdata + offset, data, wsize);
                offset += wsize;

                #ifdef CONFIG_DRIVER_WDT
                aic_watchdog_ping(NULL);
                #endif
            } else if (wsize) {
                aos_memcpy(recvdata + offset, data, wsize);
                offset += wsize;
            }

            if ((length && (recvlen >= length))) {
                goto MFWRITE_DONE;
            }

            aos_change_timer(fs_at_timer,
                            s_fs_at_cfg.timeout * 1000, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);
        } else {
            errcode = AIC_EPERM;
            break;
        }

    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;

        if (at_req->at_mode != AT_CMD_MODE) {
            at_req->at_mode = AT_CMD_MODE;
        }

        if (recvdata) {
            aos_free(recvdata);
            recvdata = NULL;
        }

        aic_put_wakelock();
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);

    if (line_count) {
        errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
    }

    return errcode;
}

/**
 * fs_at_sync_set_cmd - sync file data.
 * format:
 *  <fhandle>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_sync_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode;
    int fd;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_SYNC"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }
        fd = atoi((u8 *)paras_arr[0]);
        if ((fd < 0) || (fd > 0xFFFF)) {
            errcode = AIC_EINVAL;
            break;
        }
        if (!fs_at_fd_is_open(fd, false)) {
            errcode = AIC_EBADF;
            break;
        }

        errcode = fsync(fd);
        if (errcode < 0) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_seek_set_cmd - seek file.
 * format:
 *  <fhandle>[,<offset>,<fromwhere>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_seek_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode, fd, fromwhere;
    long roffset;
    off_t offset;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_SEEK"""=",
                                                 &count);
        if (!paras_arr || (count != 1 && count != 3)) {
            errcode = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }

        fd = atoi(para);
        if ((fd < 0) || (fd > 0xFFFF)) {
            errcode = AIC_EINVAL;
            break;
        }
        if (!fs_at_fd_is_open(fd, false)) {
            errcode = AIC_EBADF;
            break;
        }

        if (count == 3) {
            para = (u8 *)paras_arr[1];
#if 0
            if (para[0] == '-') {
                valid = aic_is_valid_integer_para(para + 1, strlen(para) - 1);
            } else {
                valid = aic_is_valid_integer_para(para, strlen(para));
            }
#else
            valid = aic_is_valid_integer_para(para, strlen(para));
#endif
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            offset = atoi(para);
            if (offset < 0 || offset > 65535) {
                errcode = AIC_EINVAL;
                break;
            }

            para = (u8 *)paras_arr[2];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            fromwhere = atoi(para);
            if (fromwhere < 0 || fromwhere > 2) {
                errcode = AIC_EINVAL;
                break;
            }
        } else {
            offset = 0;
            fromwhere = SEEK_CUR;
        }

        roffset = lseek(fd, offset, fromwhere);
        if (roffset < 0) {
            errcode = -errno;
            break;
        }
        errcode = AIC_SUCCESS;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_count = 0;
        if (count == 1) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_SEEK_RESP"""%d,%ld", fd, roffset);
            line_pointers[line_count] = (ulong)result_line;
            line_count++;
        }
        line_pointers[line_count] = (ulong)AT_RESULT_OK;
        line_count++;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_trunc_set_cmd - trunc file.
 * format:
 *  <fhandle>,<length>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_trunc_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode, fd;
    off_t length;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_TRUNC"""=",
                                                 &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }
        fd = atoi((u8 *)paras_arr[0]);
        if ((fd < 0) || (fd > 0xFFFF)) {
            errcode = AIC_EINVAL;
            break;
        }
        if (!fs_at_fd_is_open(fd, false)) {
            errcode = AIC_EBADF;
            break;
        }

        para = (u8 *)paras_arr[1];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }
        length = atoi((u8 *)paras_arr[1]);

        errcode = ftruncate(fd, length);
        if (errcode < 0)
            errcode = -errno;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d", errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_close_set_cmd - close file.
 * format:
 *  <fhandle>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_close_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode, fd;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_CLOSE"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            errcode = AIC_EINVAL;
            break;
        }
        fd = atoi((u8 *)paras_arr[0]);
        if ((fd < 0) || (fd > 0xFFFF)) {
            errcode = AIC_EINVAL;
            break;
        }
        if (!fs_at_fd_is_open(fd, true)) {
            errcode = AIC_EBADF;
            break;
        }

        errcode = close(fd);
        if (errcode < 0)
            errcode = -errno;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d", errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_delete_set_cmd - delete file in storage.
 * format:
 *  <fhandle>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_delete_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct fs_loopfind_info *findinfo = NULL;
    char *name = NULL, *tmpname = NULL;
    u16 count, line_count;
    int errcode;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_DELETE"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);
        findinfo->findtype = FS_FIND_ALLDIR;
        if (tmpname[strlen(tmpname) - 1] == '/') {
            findinfo->deltype = FS_DEL_ALL;
            name[strlen(name) - 1] = '\0';
        } else {
            findinfo->deltype = FS_DEL_FILE;
        }
        findinfo->dir = name;
        errcode = fs_loopfind(name, findinfo);

        if (errcode < 0) {
            errcode = -errno;
            break;
        }

        if ((findinfo->deltype == FS_DEL_ALL) &&
            (strcmp(name, FS_AT_DEFAULT_VOLUME) == 0) &&
            (strlen(FS_AT_DEFAULT_VOLUME) > 3)) {
            errcode = mkdir(FS_AT_DEFAULT_VOLUME, O_RDWR);
        }

        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    if (findinfo)
        aos_free(findinfo);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_move_set_cmd - move source directory or file to destination directory or file.
 * format:
 *  <old_fname>,<new_fname>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_move_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *old_fname = NULL, *tmp_old_fname = NULL;
    char *new_fname = NULL, *tmp_new_fname = NULL;
    u16 count, line_count;
    int errcode, dirpos;
    struct stat st;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_MOVE"""=",
                                                 &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        tmp_old_fname = (char *)paras_arr[0];
        if (strlen(tmp_old_fname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_old_fname) == 0) ||
            ((strlen(tmp_old_fname) == 1) && (tmp_old_fname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        old_fname = fs_at_fullpath_get(tmp_old_fname);

        tmp_new_fname = (char *)paras_arr[1];
        if (strlen(tmp_new_fname) > FS_AT_MAX_NAME_LEN) {
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_new_fname) == 0) ||
            ((strlen(tmp_new_fname) == 1) && (tmp_new_fname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        new_fname = fs_at_fullpath_get(tmp_new_fname);
        errcode = stat(old_fname, &st);
        if (errcode != 0) {
            errcode = -errno;
            break;
        }

        dirpos = fs_at_dir_lastseppos_get(new_fname);
        if (dirpos) {
            new_fname[dirpos] = '\0';
            errcode = mkdir(new_fname, O_RDWR);
            if (errcode != AIC_SUCCESS) {
                errcode = -errno;
                break;
            }
            new_fname[dirpos] = '/';
        }

        errcode = fs_move(old_fname, new_fname);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                FS_AT_ERROR_RESP"""%d",
                errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if ((old_fname) && ((u32)tmp_old_fname != (u32)old_fname)) {
        aos_free(old_fname);
    }

    if ((new_fname) && ((u32)tmp_new_fname != (u32)new_fname)) {
        aos_free(new_fname);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_check_set_cmd - check file.
 * format:
 *  <fname>[,<verify_alg>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_check_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, tmp[32];
    u16 count, line_count, verify_alg, crc16;
    char *filename = NULL, *tmpname = NULL;
    int errcode = 0, fd = -1;
    char *data = NULL;
    struct stat st;
    ssize_t rsize;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_CHECK"""=",
                                                 &count);
        if (!paras_arr || ((count != 1) && (count != 2))) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        filename = fs_at_fullpath_get(tmpname);

        if (count == 2)
            verify_alg = atoi((u8 *)paras_arr[1]);
        else
            verify_alg = FS_AT_VERIFY_MD5;

        if (verify_alg >= FS_AT_VERIFY_MAX) {
            errcode = AIC_EINVAL;
            break;
        }

        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            errcode = -errno;
            break;
        }

        errcode = fstat(fd, &st);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }

        data = aos_zalloc(st.st_size);
        if (data == NULL) {
            errcode = AIC_ENOMEM;
            break;
        }

        rsize = read(fd, data, st.st_size);
        if (rsize < 0) {
            errcode = -errno;
            break;
        }

        errcode = close(fd);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
        fd = -1;

        aos_memset(tmp, 0x0, sizeof(tmp));

        /* todo check data. */
        switch (verify_alg) {
#ifdef CONFIG_CRYPTO_MBEDTLS
            case FS_AT_VERIFY_MD5:
                {
                    u8 check[64] = {0};
#ifdef CONFIG_CRYPTO_MBEDTLS_3
                    errcode = mbedtls_md5(data, rsize, check);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
                    mbedtls_md5(data, rsize, check);
#endif
                    if (errcode < 0) {
                        errcode = AIC_ENOPROTOOPT;
                        break;
                    } else {
                        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                                 FS_AT_CHECK_RESP);
                        for (u8 i = 0; i < 16; i++) {
                            snprintf(tmp, sizeof(tmp), "%02X", check[i]);
                            strcat(result_line, tmp);
                        }
                        line_pointers[0] = (ulong)result_line;
                    }
                }
                break;
            case FS_AT_VERIFY_SHA:
                {
                    u8 check[64] = {0};
#ifdef CONFIG_CRYPTO_MBEDTLS_3
                    errcode = mbedtls_sha1(data, rsize, check);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
                    mbedtls_sha1(data, rsize, check);
#endif
                    if (errcode < 0) {
                        errcode = AIC_ENOPROTOOPT;
                        break;
                    } else {
                        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                                 FS_AT_CHECK_RESP);
                        for (u8 i = 0; i < 20; i++) {
                            snprintf(tmp, sizeof(tmp), "%02X", check[i]);
                            strcat(result_line, tmp);
                        }
                        line_pointers[0] = (ulong)result_line;
                    }
                }
                break;
            case FS_AT_VERIFY_SHA256:
                {
                    u8 check[64] = {0};
#ifdef CONFIG_CRYPTO_MBEDTLS_3
                    errcode = mbedtls_sha256(data, rsize, check, 0);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
                    mbedtls_sha256(data, rsize, check, 0);
#endif
                    if (errcode < 0) {
                        errcode = AIC_ENOPROTOOPT;
                        break;
                    } else {
                        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                                 FS_AT_CHECK_RESP);
                        for (u8 i = 0; i < 32; i++) {
                            snprintf(tmp, sizeof(tmp), "%02X", check[i]);
                            strcat(result_line, tmp);
                        }
                        line_pointers[0] = (ulong)result_line;
                    }
                }
                break;
#endif
            case FS_AT_VERIFY_CRC:
                {
                    crc16 = fs_crc_16(data, rsize);
                    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                             FS_AT_CHECK_RESP"""%02X", crc16);
                    line_pointers[0] = (ulong)result_line;
                }
                break;
            default:
                errcode = AIC_EINVAL;
                break;
        }
    } while(0);

    if (fd >= 0)
        close(fd);

    if (data)
        aos_free(data);

    if (filename && ((u32)tmpname != (u32)filename)) {
        aos_free(filename);
    }

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                FS_AT_ERROR_RESP"""%d",
                errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

#ifndef CONFIG_CUSTOMER_CM_AT
/**
 * fs_at_rename_set_cmd - rename file.
 * format:
 *  <old_fname>,<new_fname>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_rename_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *old_fname = NULL, *tmp_old_fname = NULL;
    char *new_fname = NULL, *tmp_new_fname = NULL;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_RENAME"""=",
                                                 &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        tmp_old_fname = (char *)paras_arr[0];
        if (strlen(tmp_old_fname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_old_fname) == 0) ||
            ((strlen(tmp_old_fname) == 1) && (tmp_old_fname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        old_fname = fs_at_fullpath_get(tmp_old_fname);

        tmp_new_fname = (char *)paras_arr[1];
        if (strlen(tmp_new_fname) > FS_AT_MAX_NAME_LEN) {
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_new_fname) == 0) ||
            ((strlen(tmp_new_fname) == 1) && (tmp_new_fname[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        new_fname = fs_at_fullpath_get(tmp_new_fname);

        errcode = rename(old_fname, new_fname);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
        FS_AT_ERROR_RESP"""%d",
                errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if ((old_fname) && ((u32)tmp_old_fname != (u32)old_fname)) {
        aos_free(old_fname);
    }

    if ((new_fname) && ((u32)tmp_new_fname != (u32)new_fname)) {
        aos_free(new_fname);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_copy_set_cmd -
 *  copy source directory or file to destination directory or file.
 * format:
 *  <src_path>,<dst_path>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_copy_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *src_path = NULL, *tmp_src_path = NULL;
    char *dst_path = NULL, *tmp_dst_path = NULL;
    u16 count, line_count;
    int errcode;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_COPY"""=",
                                                 &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        tmp_src_path = (char *)paras_arr[0];
        if (strlen(tmp_src_path) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_src_path) == 0) ||
            ((strlen(tmp_src_path) == 1) && (tmp_src_path[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        src_path = fs_at_fullpath_get(tmp_src_path);

        tmp_dst_path = (char *)paras_arr[1];
        if (strlen(tmp_dst_path) > FS_AT_MAX_NAME_LEN) {
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_dst_path) == 0) ||
            ((strlen(tmp_dst_path) == 1) && (tmp_dst_path[0] == '/'))) {
            errcode = FS_AT_EBUSY;
            break;
        }

        dst_path = fs_at_fullpath_get(tmp_dst_path);

        errcode = fs_copy(src_path, dst_path);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if ((src_path) && ((u32)tmp_src_path != (u32)src_path)) {
        aos_free(src_path);
    }

    if ((dst_path) && ((u32)tmp_dst_path != (u32)dst_path)) {
        aos_free(dst_path);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_mkdir_set_cmd - create directory.
 * format:
 *  <dir>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mkdir_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_MKDIR"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        errcode = mkdir(name, O_RDWR | O_CREAT);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        aicx_info(0x1c12d5, "sd", "%s mkdir error=%d.\n", name, errcode);
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_rmdir_set_cmd - remove directory or file.
 * format:
 *  <dir>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_rmdir_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    struct fs_loopfind_info *findinfo;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_RMDIR"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);
        findinfo->findtype = FS_FIND_ALLDIR;
        findinfo->deltype = FS_DEL_ALL;
        findinfo->dir = name;
        errcode = fs_loopfind(name, findinfo);
        aos_free(findinfo);

        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_cd_query_cmd - Get current work directory.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_cd_query_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    char *dir_path = NULL, *ptr;
    int errcode;
    u16 count;

    dir_path = aos_zalloc(NAME_MAX);
    AIC_ASSERT(dir_path);

    ptr = getcwd(dir_path, NAME_MAX);

    if (!ptr) {
        errcode = -errno;
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        count = 1;
    } else {
        line_pointers[0] = (ulong)dir_path;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        count = 2;
    }

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, count);

    if (dir_path)
        aos_free(dir_path);

    return errcode;
}

/**
 * fs_at_cd_set_cmd - set current work directory.
 * format:
 *  <curr_dir>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_cd_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_CD"""=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_AT_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_AT_EBUSY;
            break;
        }

        name = fs_at_fullpath_get(tmpname);

        errcode = chdir(name);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_format_set_cmd - format volume.
 * format:
 *  <volume>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_format_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct fs_mount_info mntinfo;
    struct statfs buf;
    char *volume;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 FS_AT_FORMAT"""=",
                                                 &count);
        if ((count != 0) && (count != 1)) {
            errcode = AIC_EINVAL;
            break;
        }

        if (count == 0) {
            volume = FS_AT_DEFAULT_VOLUME;
        } else {
            volume = (char *)paras_arr[0];
        }

        if (strlen(volume) > NAME_MAX) {
            errcode = AIC_ENAMETOOLONG;
            break;
        }

        errcode = fs_getmntinfo(volume, &mntinfo);
        if (errcode != AIC_SUCCESS) {
            break;
        }

        if (mntinfo.mount == 0) {
            errcode = AIC_ENOENT;
            break;
        }

        errcode = mkfs(mntinfo.fstype, mntinfo.target);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }

        errcode = statfs(mntinfo.target, &buf);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 errcode < 0 ? fs_at_err_map(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_rwtest_task - fs read&write test task.
 *
 * Parameter:
 *  @length: read write length.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
static void fs_at_rwtest_task(void *length)
{
    u32 test_length = (u32)(uintptr_t)length, return_bit;
    static char test_name[FS_AT_MAX_NAME_LEN];
    u16 file_id = 0, file_id_for_del = 0;
    u64 free_bytes = 0, total_bytes = 0;
    static uint32_t test_loop = 0;
    int fd = -1, err = -1;
    u8 test_val = 0x0;
    aos_status status;
    off_t offset;

    void *wbuf = aos_malloc(test_length);
    void *rbuf = aos_malloc(test_length);
    AOS_ASSERT(wbuf && rbuf);

    while (1) {
        if (!s_fs_at_cfg.rwtest_start) {
            aos_set_event(fs_at_rwtest_event, FS_AT_RWTEST_STOP, AOS_OR);
            status = aos_get_event(fs_at_rwtest_event,
                                    FS_AT_RWTEST_START,
                                    AOS_OR_CLEAR,
                                    &return_bit,
                                    AOS_WAIT_FOREVER);
            AIC_ASSERT(AIC_SUCCESS == status);
        }

        snprintf(test_name, FS_AT_MAX_NAME_LEN,
                "%s/file_%u",
                s_fs_at_cfg.rwtest_path, file_id++);
        fd = open(test_name, O_RDWR | O_TRUNC | O_CREAT);
        if (fd < 0) {
            aicx_error(0x018517, "sd", "open (%s) failed ret: %d.\n", test_name, fd);
            break;
        }

        aos_memset(wbuf, test_val++, test_length);
        err = write(fd, wbuf, test_length);
        if ((err < 0) || (err != test_length)) {
            aicx_error(0x0e4210, "dd", "write (%d) failed ret: %d.\n", test_length, err);
            break;
        }

        offset = lseek(fd, 0, FS_SEEK_BEGIN);
        if (offset != 0) {
            aicx_error(0x0c7f76, "d", "lseek begin failed ret: %d.\n", offset);
            break;
        }

        err = read(fd, rbuf, test_length);
        if ((err < 0) || (err != test_length)) {
            aicx_error(0x16d581, "dd", "read (%d) failed ret: %d.\n", test_length, err);
            break;
        }

        if (memcmp(wbuf, rbuf, test_length)) {
            aicx_error(0x0bb7e9, "", "rbuf wbuf compare failed!\n");
            break;
        }

        close(fd);
        fd = -1;

        err = fs_getdiskfreespace(s_fs_at_cfg.rwtest_path, &free_bytes, &total_bytes);
        if (err != 0) {
            err = -errno;
            aicx_error(0x0ba14d, "d", "get disk space failed ret: %d.\n", err);
            break;
        }

        if ((free_bytes <= (test_length + 4096)) || ((file_id - file_id_for_del) > 6)) {
            struct fs_loopfind_info *findinfo;
            snprintf(test_name, FS_AT_MAX_NAME_LEN,
                        "%s", s_fs_at_cfg.rwtest_path);
            findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->deltype = FS_DEL_ALL;
            findinfo->dir = test_name;
            fs_loopfind(test_name, findinfo);
            aos_free(findinfo);

            err = mkdir(test_name, O_RDWR | O_CREAT);
            if (err) {
                aicx_info(0x0ae9e1, "sd", "mkdir %s err(%d).\n", test_name, err);
                break;
            }
            file_id_for_del = file_id;
        }

        if (++test_loop % 100 == 0) {
            aicx_info(0x17f990, "u", "LOOP [%u] fs_rwtest", test_loop);
        }

        aos_sleep(s_fs_at_cfg.rwtest_interval);
    }

    if (fd >= 0)
        close(fd);

    if (wbuf)
        aos_free(wbuf);

    if (rbuf)
        aos_free(rbuf);
}

/**
 * fs_at_format_set_cmd - fs read&write test.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_rwtest_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *volume, path[FS_AT_MAX_NAME_LEN], *para;
    u64 free_bytes = 0, total_bytes = 0;
    struct fs_loopfind_info *findinfo;
    int errcode = AIC_SUCCESS, start;
    u32 length, interval, return_bit;
    static boolean init = 0;
    aos_status status;
    boolean valid;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                FS_AT_RWTEST"""=",
                                                &count);
        if (count > 4) {
            errcode = AIC_EINVAL;
            break;
        }

        length = s_fs_at_cfg.rwtest_length;
        interval = s_fs_at_cfg.rwtest_interval;
        if (count == 0) {
            start = 1;
            volume = FS_AT_DEFAULT_VOLUME;
        } else if (count == 1) {
            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            start = atoi((u8 *)paras_arr[0]);

            volume = FS_AT_DEFAULT_VOLUME;
        } else if (count == 2) {
            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            start = atoi((u8 *)paras_arr[0]);

            volume = (char *)paras_arr[1];
        } else if (count == 3) {
            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            start = atoi((u8 *)paras_arr[0]);

            volume = (char *)paras_arr[1];

            para = (u8 *)paras_arr[2];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            interval = atoi((u8 *)paras_arr[2]);
        } else {
            para = (u8 *)paras_arr[0];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            start = atoi((u8 *)paras_arr[0]);

            volume = (char *)paras_arr[1];

            para = (u8 *)paras_arr[2];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            interval = atoi((u8 *)paras_arr[2]);

            para = (u8 *)paras_arr[3];
            valid = aic_is_valid_integer_para(para, strlen(para));
            if (!valid) {
                errcode = AIC_EINVAL;
                break;
            }
            length = atoi((u8 *)paras_arr[3]);
        }

        if ((start != 0) && (start != 1)) {
            errcode = AIC_EINVAL;
            break;
        }

        if (start) {
            if (s_fs_at_cfg.rwtest_start) {
                break;
            } else {
                if (init) {
                    s_fs_at_cfg.rwtest_start = true;
                    aos_set_event(fs_at_rwtest_event,
                                FS_AT_RWTEST_START,
                                AOS_OR);
                    break;
                }
            }
        } else {
            if (s_fs_at_cfg.rwtest_start) {
                s_fs_at_cfg.rwtest_start = false;
                status = aos_get_event(fs_at_rwtest_event,
                                        FS_AT_RWTEST_STOP,
                                        AOS_OR_CLEAR,
                                        &return_bit,
                                        AOS_WAIT_FOREVER);
                AIC_ASSERT(AIC_SUCCESS == status);
            }
            break;
        }

#ifdef CONFIG_FS_FATFS
        snprintf(path, FS_AT_MAX_NAME_LEN, "%s/home/test", volume);
#else
        snprintf(path, FS_AT_MAX_NAME_LEN, "%s/test", volume);
#endif

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        findinfo->findtype = FS_FIND_ALLDIR;
        findinfo->deltype = FS_DEL_ALL;
        findinfo->dir = path;
        fs_loopfind(path, findinfo);
        aos_free(findinfo);

        errcode = mkdir(path, O_RDWR | O_CREAT);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }

        errcode = fs_getdiskfreespace(volume, &free_bytes, &total_bytes);
        if (errcode || free_bytes < (length + 4096 * 2)) { 
            aicx_error(0x0b478b, "duu", "fs_at_rwtest[%d] not enougth space[%u/%u]",
                        errcode, free_bytes, total_bytes);
            if (errcode != AIC_SUCCESS) {
                errcode = -errno;
            } else {
                errcode = AIC_ENOSPC;
            }
            break;
        }

        aos_memset(s_fs_at_cfg.rwtest_path, 0x0, FS_AT_MAX_NAME_LEN);
        snprintf(s_fs_at_cfg.rwtest_path, FS_AT_MAX_NAME_LEN, "%s", path);
        s_fs_at_cfg.rwtest_length = length;
        s_fs_at_cfg.rwtest_interval = interval;

        aos_thread_id test_tid = aos_create_thread_simply("fsrwtst",
                                                        fs_at_rwtest_task,
                                                            (void *)length,
                                                            4096,
                                                            253);
        if (test_tid == AOS_INVALID_THREAD_ID) {
            aicx_error(0x0adb27, "", "fs_at_rwtest failed to create test thread");
            errcode = AIC_ENOMEM;
            break;
        }

        fs_at_rwtest_event = aos_create_event("fsrwtst");
        AIC_ASSERT(fs_at_rwtest_event);

        init = true;
        s_fs_at_cfg.rwtest_start = true;
    } while (0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 FS_AT_ERROR_RESP"""%d",
                 fs_at_err_map(errcode));
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}
#endif

/*
 *  Add fs AT handle here.
 */
const static AT_CMD_HANDLE fs_at_handles[] =
{
    {FS_AT_CFG, fs_at_cfg_cmd, NULL, fs_at_cfg_set_cmd, NULL},
    {FS_AT_INFO, NULL, NULL, fs_at_info_set_cmd, NULL},
    {FS_AT_LIST, NULL, NULL, fs_at_list_set_cmd, NULL},
    {FS_AT_SIZE, NULL, NULL, fs_at_size_set_cmd, NULL},
    {FS_AT_PUT, NULL, NULL, fs_at_put_set_cmd, NULL},
    {FS_AT_GET, NULL, NULL, fs_at_get_set_cmd, NULL},
    {FS_AT_OPEN, NULL, fs_at_open_query_cmd, fs_at_open_set_cmd, NULL},
    {FS_AT_READ, NULL, NULL, fs_at_read_set_cmd, NULL},
    {FS_AT_WRITE, NULL, NULL, fs_at_write_set_cmd, NULL},
    {FS_AT_SYNC, NULL, NULL, fs_at_sync_set_cmd, NULL},
    {FS_AT_SEEK, NULL, NULL, fs_at_seek_set_cmd, NULL},
    {FS_AT_TRUNC, NULL, NULL, fs_at_trunc_set_cmd, NULL},
    {FS_AT_CLOSE, NULL, NULL, fs_at_close_set_cmd, NULL},
    {FS_AT_DELETE, NULL, NULL, fs_at_delete_set_cmd, NULL},
    {FS_AT_MOVE, NULL, NULL, fs_at_move_set_cmd, NULL},
    {FS_AT_CHECK, NULL, NULL, fs_at_check_set_cmd, NULL},
#ifndef CONFIG_CUSTOMER_CM_AT
    {FS_AT_MKDIR, NULL, NULL, fs_at_mkdir_set_cmd, NULL},
    {FS_AT_RMDIR, NULL, NULL, fs_at_rmdir_set_cmd, NULL},
    {FS_AT_CD, NULL, fs_at_cd_query_cmd, fs_at_cd_set_cmd, NULL},
    {FS_AT_RENAME, NULL, NULL, fs_at_rename_set_cmd, NULL},
    {FS_AT_COPY, NULL, NULL, fs_at_copy_set_cmd, NULL},
    {FS_AT_FORMAT, NULL, NULL, fs_at_format_set_cmd, NULL},
    {FS_AT_RWTEST, NULL, NULL, fs_at_rwtest_set_cmd, NULL},
#endif
    {AIC_NULL},
};

/**
 * fs_at_cmd_init - fs at cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int fs_at_cmd_init(void)
{
    list_head_init(&fs_at_open_list);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fs_at_handles);

    fs_at_timer = aos_create_timer("fsat", fs_at_timeout,
                                   NULL, FS_AT_TIMEOUT,
                                   AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(fs_at_timer != AOS_NULL);

    return AIC_SUCCESS;
}

AIC_COMP_P2_INIT_EXPORT(fs_at_cmd_init);

