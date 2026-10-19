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
#define AIC_THIS_FILE    0x1942e4
#define LOG_MODULE    AMOD_FS

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <fs_api.h>
#include <aic_errno.h>
#include <aic_math.h>
#include <aic_at_resp_handle.h>
#include <aic_at_cmd_handle.h>
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
#include "fs_at.h"
#include "fs_crc.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define FS_CUSTOM_MAX_NAME_LEN      (128)
#define FS_CUSTOME_DEFAULT_VULUME   "/C/home"

enum FS_MF_ERRCODE {
    FS_MF_EINVAL    = 50,       /* Fs Invalid argument. */
    FS_MF_ERROR     = 1050,     /* Fs error. */
    FS_MF_ENOENT    = 1051,     /* No such file or directory. */
    FS_MF_EEXIST    = 1052,     /* File exists. */
    FS_MF_EFBIG     = 1053,     /* File too large */
    FS_MF_EMFILE    = 1054,     /* Too many open files */
    FS_MF_EBUSY     = 1055,     /* file had opened. */
    FS_MF_EOPEN     = 1056,     /* Open file error */
    FS_MF_EWRITE    = 1057,     /* Write file error */
    FS_MF_EREAD     = 1058,     /* Read file error */
    FS_MF_EROFS     = 1059,     /* Read-only file system */
    FS_MF_EBADF     = 1060,     /* Bad file number */
    FS_MF_EDEL      = 1061,     /* file delete error */
    FS_MF_ELS       = 1062,     /* file list error */
    FS_MF_ENOSPC    = 1063,     /* No space left on device */
    FS_MF_ETIME     = 1064,     /* timeout occurred */
};

enum FS_MF_INPUT_FORMAT {
    FS_MF_INPUT_ASCII           = 0,
    FS_MF_INPUT_HEX             = 1,
    FS_MF_INPUT_ESCAPE_CHAR     = 2,
    FS_MF_INPUT_MAX
};

enum FS_MF_OUTPUT_FORMAT {
    FS_MF_OUTPUT_ASCII      = 0,
    FS_MF_OUTPUT_HEX        = 1,
    FS_MF_OUTPUT_MAX
};

enum FS_MF_VERIFY_ALG {
    FS_MF_VERIFY_MD5        = 0,
    FS_MF_VERIFY_SHA        = 1,
    FS_MF_VERIFY_SHA256     = 2,
    FS_MF_VERIFY_CRC        = 3,
    FS_MF_VERIFY_MAX
};

/**---------------------------------------------------------------------------*
 ** Private Types
 **---------------------------------------------------------------------------*/
struct mfcfg {
    u16 timeout;            /* unit: s*/
    u32 frag_size;
    u32 frag_interval;      /* unit: ms*/
    u8 input_format;
    u8 output_format;
};

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
static aos_timer_ptr fs_at_timer;
static boolean fs_at_timeout_flag = false;
static u32 fs_at_conn_id;
static struct list_head fs_at_open_list;

static struct mfcfg s_mfcfg =
{
    .timeout = 10,
    .frag_size = 0,
    .frag_interval = 0,
    .input_format = 0,
    .output_format = 0
};

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * fs_mfat_fullpath_get - get full path.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:
 *
 */
char *fs_mfat_fullpath_get(char *path)
{
    char *full_path;

    if (path[0] == '/') {
        if ((path[1] == 'C' || path[1] == 'D') && ((strlen(path) == 2) || (path[2] == '/'))) {
            return path;
        }
        full_path = aos_zalloc(FS_CUSTOM_MAX_NAME_LEN + 4);
        snprintf(full_path, FS_CUSTOM_MAX_NAME_LEN + 4, "%s%s",
                FS_CUSTOME_DEFAULT_VULUME, path);
        return full_path;
    }

    full_path = aos_zalloc(FS_CUSTOM_MAX_NAME_LEN + 4);
    snprintf(full_path, FS_CUSTOM_MAX_NAME_LEN + 4, "%s/%s",
            FS_CUSTOME_DEFAULT_VULUME, path);

    return full_path;
}

/**
 * fs_mfat_dir_lastpos - get directory path length from path.
 *
 * Parameter:
 *  @path: input path.
 *
 * Return:
 *  directory length;
 *
 */
int fs_mfat_dir_lastpos(char *path)
{
    int pos, cnt;

    pos = 0;
    cnt = 0;
    while (path[cnt]) {
        if (path[cnt] == '/') {
            pos = cnt;
        }
        cnt++;
    }

    if (!memcmp("/C", path, pos) || !memcmp("/D", path, pos))
        return 0;

    return pos;
}

/**
 * fs_map_mf_ret - map to mf return.
 *
 * Parameter:
 *  @errcode: input error code.
 *
 * Return:
 *
 */
static int fs_map_mf_ret(int errcode)
{
    int ret;

    switch (errcode) {
        case AIC_ENOENT:
            ret  = FS_MF_ENOENT;
            break;
        case AIC_EEXIST:
            ret = FS_MF_EEXIST;
            break;
        case AIC_EFBIG:
            ret = FS_MF_EFBIG;
            break;
        case AIC_EMFILE:
            ret = FS_MF_EMFILE;
            break;
        case AIC_EBADF:
            ret = FS_MF_EBADF;
            break;
        case AIC_EROFS:
            ret = FS_MF_EROFS;
            break;
        case AIC_ENOSPC:
            ret = FS_MF_ENOSPC;
            break;
        case AIC_ETIME:
            ret = FS_MF_ETIME;
            break;
        case AIC_EINVAL:
            ret = FS_MF_EINVAL;
            break;
        default:
            ret = FS_MF_ERROR;
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
    u8 *result_line = NULL;
    int ret = AIC_SUCCESS;
    struct stat buf;

    if (!memcmp(at_cmd->cmd, "MFLIST", strlen("MFLIST")) &&
               strlen(at_cmd->cmd) == strlen("MFLIST")) {
        AIC_ASSERT(at_cmd->ptr);
        result_line = at_cmd->ptr;
        find_info->found_cnt++;

        if (find_info->dent->d_type & DT_DIR) {
            if (!at_cmd->prefix &&
                (!memcmp(find_info->fullpath, "C:/", strlen("C:/")) ||
                !memcmp(find_info->fullpath, "/C/", strlen("/C/")))) {
                snprintf(result_line, NAME_MAX, "+MFLIST: \"%s\"",
                        find_info->fullpath + strlen(FS_CUSTOME_DEFAULT_VULUME));
            } else {
                snprintf(result_line, NAME_MAX, "+MFLIST: \"%s\"",
                        find_info->fullpath);
            }
        } else {
            ret = stat(find_info->fullpath, &buf);
            if (ret != 0) {
                ret = errno;
                goto find_fail;
            }

            at_cmd->file_size += buf.st_size;
            if (!at_cmd->prefix &&
                (!memcmp(find_info->fullpath, "C:/", strlen("C:/")) ||
                !memcmp(find_info->fullpath, "/C/", strlen("/C/")))) {
                snprintf(result_line, NAME_MAX, "+MFLIST: \"%s\",%ld",
                        find_info->fullpath + strlen(FS_CUSTOME_DEFAULT_VULUME), buf.st_size);
            } else {
                snprintf(result_line, NAME_MAX, "+MFLIST: \"%s\",%ld",
                        find_info->fullpath, buf.st_size);
            }
        }

        result_line[strlen(result_line)] = '\0';

        line_pointers[0] = (ulong)result_line;
        aic_fill_transmit_cmd_resp(at_cmd->at_req, line_pointers, 1);
    }

find_fail:
    return ret;
}

/**
 * fs_at_mfcfg_cmd - response AT+MFCFG=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfcfg_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+MFCFG: \"timeout\",(1-120)";
    line_pointers[1] = (ulong)"+MFCFG: \"encoding\",(0-2),(0-1)";
    line_pointers[2] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 3);
}

/**
 * fs_at_mfcfg_set_cmd - response AT+MFCFG.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfcfg_set_cmd(AT_REQ_RESP *at_req)
{
    u16 count, timeout, input_format, output_format;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    static char *name = NULL;
    u16 line_count;
    int errcode;
    boolean valid;

    do {
        errcode = AIC_SUCCESS;
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFCFG=", &count);
        if (!paras_arr || count < 1 || count > 3) {
            errcode = AIC_EINVAL;
            break;
        }

        name = (u8 *)paras_arr[0];

        if ((strlen("timeout") == strlen(name)) &&
            !memcmp("timeout", name, strlen(name))) {
            if (count == 1) {
                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         "+MFCFG: \"timeout\",%d", s_mfcfg.timeout);
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
                    s_mfcfg.timeout = timeout;
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
                         "+MFCFG: \"encoding\",%d,%d",
                         s_mfcfg.input_format, s_mfcfg.output_format);
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
                if (input_format >= FS_MF_INPUT_MAX) {
                    errcode = AIC_EINVAL;
                } else {
                    s_mfcfg.input_format = input_format;
                    line_pointers[0] = (ulong)AT_RESULT_OK;
                    line_count = 1;
                }
            } else if (count == 3) {
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
                if ((input_format >= FS_MF_INPUT_MAX) ||
                    (output_format >= FS_MF_OUTPUT_MAX)) {
                    errcode = AIC_EINVAL;
                } else {
                    s_mfcfg.input_format = input_format;
                    s_mfcfg.output_format = output_format;
                    line_pointers[0] = (ulong)AT_RESULT_OK;
                    line_count = 1;
                }
            } else {
                errcode = AIC_EINVAL;
            }
        } else {
            errcode = AIC_EINVAL;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qflds_set_cmd - Gets memory space information.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfsinfo_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u64 free_size, total_size;
    u16 count, line_count;
    int errcode;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFSINFO=", &count);
        if (count != 0) {
            errcode = AIC_EINVAL;
            break;
        }

        name = "/C";
        errcode = fs_getdiskfreespace(name, &free_size, &total_size);
        if (errcode < 0)
            errcode = -errno;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+MFSINFO: %lld,%lld", total_size, free_size);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_mflist_set_cmd - List file information.
 *  1.AT+MFLST=<path>: response +MFLST: <fname>,<size>
 *  2.AT+MFLST: response spatial information for all files stored in default volume,
 *              +MFLST: <fname>,<size>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mflist_set_cmd(AT_REQ_RESP *at_req)
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
                                                 "AT+MFLIST=", &count);
        if (count == 0) {
            tmpname = FS_CUSTOME_DEFAULT_VULUME;
        } else if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        } else {
            tmpname = (char *)paras_arr[0];
        }

        if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN) {
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_MF_EBUSY;
            break;
        }

        name = fs_mfat_fullpath_get(tmpname);

        at_cmd = aos_zalloc(sizeof(FS_AT_CMD));
        at_cmd->at_req = at_req;
        at_cmd->cmd = "MFLIST";
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
            errcode = FS_MF_ENOENT;
        }

        aos_free(findinfo);
        aos_free(at_cmd->ptr);
        aos_free(at_cmd);
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfsize_set_cmd - get file information.
 *  response AT+MFSIZE: <fname>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfsize_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    u16 count, line_count;
    struct stat st;
    int errcode;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFSIZE=", &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_MF_EBUSY;
            break;
        }

        name = fs_mfat_fullpath_get(tmpname);

        errcode = stat(name, &st);
        if (errcode != AIC_SUCCESS) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+MFSIZE: %lu", st.st_size);
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
 * fs_at_qfupl_set_cmd - Upload a file to storage.
 *  response AT+MFPUT= <fname>,<fsize>[,<data>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfput_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL, *ptr = NULL, *para;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    ssize_t rsize, wsize, datalen;
    static char *filename = NULL;
    static u32 filesize, recvlen;
    u64 free_bytes, total_bytes;
    u16 count = 0, line_count;
    struct statfs stfs;
    struct stat buf;
    static int fd;
    char *tmpname;
    int errcode;
    boolean valid;
    int dirpos;
    boolean convert;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;

    do {
        errcode = AIC_SUCCESS;
        if (at_req->at_mode == AT_CMD_MODE) {
            fd = -1;
            recvlen = 0;
            paras_arr = at_get_para_array(at_req->at_req, "AT+MFPUT=", &count, 3, 3);
            if (!paras_arr || count < 2 || count > 3) {
                errcode = AIC_EINVAL;
                break;
            }

            tmpname = (char *)paras_arr[0];
            if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN) {
                errcode = AIC_EINVAL;
                break;
            }

            if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
                errcode = FS_MF_EBUSY;
                break;
            }

            filename = fs_mfat_fullpath_get(tmpname);
            if (filename && ((u32)tmpname == (u32)filename)) {
                filename = aos_zalloc(FS_CUSTOM_MAX_NAME_LEN + 4);
                aos_memcpy(filename, tmpname, strlen(tmpname));
            }

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

            dirpos = fs_mfat_dir_lastpos(filename);
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
                errcode = fs_getdiskfreespace(FS_CUSTOME_DEFAULT_VULUME, &free_bytes, &total_bytes);
                if (errcode < 0) {
                    break;
                }

                data = (u8 *)paras_arr[2];

                if (s_mfcfg.input_format == FS_MF_INPUT_HEX) {
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

                    if (datalen) {
                        rsize = write(fd, ptr, wsize);
                    } else {
                        rsize = 0;
                    }

                    aos_free(ptr);
                } else if (s_mfcfg.input_format == FS_MF_INPUT_ESCAPE_CHAR) {
                    datalen = strlen(data);
                    if (filesize && (filesize != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }

                    ptr = aos_zalloc(strlen(data) + 4);
                    AIC_ASSERT(ptr);

                    if (datalen) {
                        aic_escape_data_normal(ptr, data, (int)datalen, (int *)&wsize);
                        wsize = wsize > free_bytes ? free_bytes : wsize;
                        rsize = write(fd, ptr, wsize);
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
            }

            aos_change_timer(fs_at_timer, s_mfcfg.timeout * 1000, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            aic_get_wakelock();
            at_req->at_mode = AT_DATA_MODE;
            line_pointers[0] = (ulong)">";
            line_count = 1;
        } else if (at_req->at_upstream_data != AIC_NULL &&
                   at_req->at_mode == AT_DATA_MODE) {
            if (fs_at_timeout_flag) {
MFPUT_DONE:
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
                         "+MFPUT: %ld", buf.st_size);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                aos_free(filename);
                filename = NULL;
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

            errcode = fs_getdiskfreespace(FS_CUSTOME_DEFAULT_VULUME, &free_bytes, &total_bytes);
            if (errcode < 0) {
                break;
            }
            wsize = wsize > free_bytes ? free_bytes : wsize;

            if (wsize)
                rsize = write(fd, at_req->at_upstream_data, wsize);

            if (rsize < 0) {
                errcode = -errno;
                break;
            }

            if (filesize && (recvlen >= filesize)) {
                errcode = AIC_SUCCESS;
                goto MFPUT_DONE;
            }

            aos_change_timer(fs_at_timer, s_mfcfg.timeout * 1000, false);
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
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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

        aic_put_wakelock();
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_mfget_set_cmd - Download a file from storage.
 *  response AT+QFDWL=<fname>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfget_set_cmd(AT_REQ_RESP *at_req)
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
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MFGET=",
                                                 &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_MF_EBUSY;
            break;
        }

        filename = fs_mfat_fullpath_get(tmpname);

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
            snprintf(result_line, rspsize, "+MFGET: %ld,", st.st_size);
            line_pointers[0] = (ulong)result_line;
            aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            errcode = AIC_SUCCESS;
            break;
        }

        frag_size = s_mfcfg.frag_size ? s_mfcfg.frag_size : st.st_size;

        data = aos_zalloc(frag_size + 4);
        if (data == NULL) {
            errcode = AIC_ENOMEM;
            break;
        }

        if (s_mfcfg.output_format == FS_MF_OUTPUT_HEX) {
            ptr = aos_zalloc(frag_size * 2 + 4);
            if (ptr == NULL) {
                errcode = AIC_ENOMEM;
                break;
            }
        }

        if (s_mfcfg.output_format == FS_MF_OUTPUT_HEX) {
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

            if (s_mfcfg.output_format == FS_MF_OUTPUT_HEX) {
                hex2string(data, ptr, rsize);
                snprintf(result_line, rspsize, "+MFGET: %d,%s", rsize, ptr);
                line_pointers[0] = (ulong)result_line;
                aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            } else {
                snprintf(result_line, rspsize, "+MFGET: %d,", rsize);
                i = strlen(result_line);
                aos_memcpy(result_line + i, data, rsize);
                aic_transmit_at_result(at_req->serial_port_id, result_line, i + rsize);
            }

            st.st_size -= rsize;
            if (s_mfcfg.frag_interval)
                aos_sleep(s_mfcfg.frag_interval);
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        if (result_line == NULL) {
            rspsize = 100;
            result_line = aos_zalloc(100);
            AIC_ASSERT(result_line);
        }
        snprintf(result_line, rspsize, "+CME ERROR: %d",
                 errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfopen_set_cmd - response AT+MFOPEN=<fname>[,<mode>].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfopen_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    char *name = NULL, *tmpname = NULL;
    u16 count, line_count;
    int fd, flags, mode;
    int errcode, dirpos;
    boolean valid;

    do {
        errcode = AIC_SUCCESS;

        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MFOPEN=",
                                                 &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_MF_EBUSY;
            break;
        }

        name = fs_mfat_fullpath_get(tmpname);

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
            dirpos = fs_mfat_dir_lastpos(name);
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

        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "+MFOPEN: %u", fd);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfread_set_cmd - response AT+MFREAD=<fhandle>[,<length>].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfread_set_cmd(AT_REQ_RESP *at_req)
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
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MFREAD=",
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
            errcode = FS_MF_ERROR;
            break;
        }

        frag_size = s_mfcfg.frag_size ? s_mfcfg.frag_size : st.st_size;

        if (s_mfcfg.output_format == FS_MF_OUTPUT_HEX) {
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
                errcode = FS_MF_EREAD;
                break;
            }
        } else {
            rsize = read(fd, data, frag_size);
            if (rsize < 0) {
                errcode = -errno;
                if (errcode == AIC_EACCES)
                    errcode = FS_MF_EREAD;
                break;
            }
            // snprintf(result_line, rspsize, "+MFREAD: %d,%ld,0,0,0", fd, length == 0xFFFFFFFF ? 0 : length);
            // line_pointers[0] = (ulong)result_line;
            // line_count = 1;
            // aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
            errcode = AIC_SUCCESS;
            break;
        }

        if (s_mfcfg.output_format == FS_MF_OUTPUT_HEX) {
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
                    errcode = FS_MF_EREAD;
                break;
            }

            if (rsize == 0) {
                errcode = AIC_SUCCESS;
                break;
            }

            length -= rsize;
            asize += rsize;
            st.st_size -= rsize;

            if (s_mfcfg.output_format == FS_MF_OUTPUT_HEX) {
                hex2string(data, ptr, rsize);
                snprintf(result_line, rspsize, "+MFREAD: %d,%d,%d,%d,%s",
                         fd, tsize, asize, rsize, ptr);
                line_pointers[0] = (ulong)result_line;
                errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            } else {
                snprintf(result_line, rspsize, "+MFREAD: %d,%d,%d,%d,",
                         fd, tsize, asize, rsize);
                i = strlen(result_line);
                aos_memcpy(result_line + i, data, rsize);
                errcode = aic_transmit_at_result(at_req->serial_port_id, result_line, i + rsize);
            }

            if (errcode != AIC_SUCCESS) {
                aicx_error(0x14276a, "d", "resp at fail, errcode=%d.\n", errcode);
                break;
            }
            if (s_mfcfg.frag_interval)
                aos_sleep(s_mfcfg.frag_interval);
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        if (result_line == NULL) {
            rspsize = 100;
            result_line = aos_zalloc(rspsize);
            AIC_ASSERT(result_line);
        }
        snprintf(result_line, rspsize, "+CME ERROR: %d",
                 errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfwrite_set_cmd - response AT+MFWRITE=<fhandle>,<length>[,<data>]].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfwrite_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL, *ptr = NULL, *para;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    static u32 length, recvlen, wlength;
    u64 free_bytes, total_bytes;
    ssize_t rsize, wsize, datalen;
    ulong *paras_arr = NULL;
    boolean valid, convert;
    u16 count = 0, line_count;
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
            paras_arr = at_get_para_array(at_req->at_req, "AT+MFWRITE=", &count, 3, 3);
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

            if (count == 3) {
                errcode = fs_getdiskfreespace(FS_CUSTOME_DEFAULT_VULUME, &free_bytes, &total_bytes);
                if (errcode < 0) {
                    break;
                }

                data = (u8 *)paras_arr[2];

                if (s_mfcfg.input_format == FS_MF_INPUT_HEX) {
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

                    rsize = write(fd, ptr, wsize);
                    aos_free(ptr);
                } else if (s_mfcfg.input_format == FS_MF_INPUT_ESCAPE_CHAR) {
                    datalen = strlen(data);
                    if (length && (length != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }

                    ptr = aos_zalloc(strlen(data) + 4);
                    AIC_ASSERT(ptr);

                    aic_escape_data_normal(ptr, data, (int)datalen, (int *)&wsize);
                    wsize = wsize > free_bytes ? free_bytes : wsize;
                    rsize = write(fd, ptr, wsize);
                    aos_free(ptr);
                } else {
                    datalen = strlen(data);
                    if (length && (length != datalen)) {
                        errcode = AIC_EINVAL;
                        break;
                    }
                    wsize = datalen > free_bytes ? free_bytes : datalen;
                    rsize = write(fd, data, wsize);
                }
                if (rsize < 0) {
                    errcode = -errno;
                    break;
                }
                wlength = rsize;
                goto MFWRITE_DONE;
            }

            aos_change_timer(fs_at_timer, s_mfcfg.timeout * 1000, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            aic_get_wakelock();
            at_req->at_mode = AT_DATA_MODE;
            line_pointers[0] = (ulong)">";
            line_count = 1;
            errcode = AIC_SUCCESS;
        } else if (at_req->at_upstream_data != AIC_NULL &&
                   at_req->at_mode == AT_DATA_MODE) {
            if (fs_at_timeout_flag) {
MFWRITE_DONE:
                errcode = fsync(fd);
                if (errcode != AIC_SUCCESS) {
                    errcode = -errno;
                    break;
                }

                at_req->at_mode = AT_CMD_MODE;

                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         "+MFWRITE: %d,%d", fd, wlength);
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

            errcode = fs_getdiskfreespace(FS_CUSTOME_DEFAULT_VULUME, &free_bytes, &total_bytes);
            if (errcode < 0) {
                break;
            }
            wsize = wsize > free_bytes ? free_bytes : wsize;

            if (wsize)
                rsize = write(fd, at_req->at_upstream_data, wsize);

            if (rsize < 0) {
                errcode = -errno;
                break;
            }
            wlength += rsize;

            if (length && (recvlen >= length)) {
                errcode = AIC_SUCCESS;
                goto MFWRITE_DONE;
            }

            aos_change_timer(fs_at_timer, s_mfcfg.timeout * 1000, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);
        } else {
            errcode = AIC_EPERM;
            break;
        }

    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;

        if (at_req->at_mode != AT_CMD_MODE) {
            at_req->at_mode = AT_CMD_MODE;
        }

        aic_put_wakelock();
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_mfsync_set_cmd - response AT+MFSYNC=<fhandle>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfsync_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode;
    int fd;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFSYNC=", &count);
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

        errcode = fsync(fd);
        if (errcode < 0) {
            errcode = -errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfseek_set_cmd - response AT+MFSEEK=<fhandle>[,<offset>,<fromwhere>].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfseek_set_cmd(AT_REQ_RESP *at_req)
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
                                                 "AT+MFSEEK=", &count);
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
        } else if (count == 1) {
            offset = 0;
            fromwhere = SEEK_CUR;
        } else {
            errcode = AIC_EINVAL;
            break;
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
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_count = 0;
        if (count == 1) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+MFSEEK: %d,%ld", fd, roffset);
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
 * fs_at_mftrunc_set_cmd - response AT+MFTRUNC=<fhandle>,<length>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mftrunc_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode, fd;
    off_t length;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFTRUNC=", &count);
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
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfclose_set_cmd - response AT+MFCLOSE=<fhandle>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfclose_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, line_count;
    int errcode, fd;
    boolean valid;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFCLOSE=", &count);
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

        errcode = close(fd);
        if (errcode < 0)
            errcode = -errno;
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfdelete_set_cmd - Delete file in storage.AT+QFDEL=<fname>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfdelete_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    char *name = NULL, *tmpname = NULL;
    struct fs_loopfind_info *findinfo;
    u16 count, line_count;
    int errcode;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFDELETE=", &count);
        if (!paras_arr || count != 1) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if (strlen(tmpname) == 0) {
            errcode = FS_MF_EBUSY;
            break;
        }

        name = fs_mfat_fullpath_get(tmpname);

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
        aos_free(findinfo);

        if (errcode < 0)
            errcode = -errno;

        if (strcmp(name, FS_CUSTOME_DEFAULT_VULUME) == 0) {
            errcode = mkdir(FS_CUSTOME_DEFAULT_VULUME, O_RDWR);
            if (errcode != AIC_SUCCESS) {
                errcode = -errno;
                break;
            }
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if (name && ((u32)tmpname != (u32)name)) {
        aos_free(name);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_mfmove_set_cmd -
 *  move source directory or file to destination directory or file.
 *  response AT+FSMOVE: <old_fname>,<new_fname>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfmove_set_cmd(AT_REQ_RESP *at_req)
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
                                                 "AT+MFMOVE=", &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        }

        tmp_old_fname = (char *)paras_arr[0];
        if (strlen(tmp_old_fname) > FS_CUSTOM_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_old_fname) == 0) ||
            ((strlen(tmp_old_fname) == 1) && (tmp_old_fname[0] == '/'))) {
            errcode = FS_MF_EBUSY;
            break;
        }

        old_fname = fs_mfat_fullpath_get(tmp_old_fname);

        tmp_new_fname = (char *)paras_arr[1];
        if (strlen(tmp_new_fname) > FS_CUSTOM_MAX_NAME_LEN) {
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmp_new_fname) == 0) ||
            ((strlen(tmp_new_fname) == 1) && (tmp_new_fname[0] == '/'))) {
            errcode = FS_MF_EBUSY;
            break;
        }

        new_fname = fs_mfat_fullpath_get(tmp_new_fname);

        errcode = stat(old_fname, &st);
        if (errcode != 0) {
            errcode = -errno;
            break;
        }

        dirpos = fs_mfat_dir_lastpos(new_fname);
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
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
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
 * fs_at_mfcheck_set_cmd -
 *  response AT+FSCHECK: <fname>[,<verify_alg>]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mfcheck_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, check[64], tmp[32];
    u16 count, line_count, verify_alg, crc16, i;
    char *filename = NULL, *tmpname = NULL;
    int errcode, ret = 0, fd = -1;
    char *data = NULL;
    struct stat st;
    ssize_t rsize;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+MFCHECK=", &count);
        if (!paras_arr || ((count != 1) && (count != 2))) {
            errcode = AIC_EINVAL;
            break;
        }

        tmpname = (char *)paras_arr[0];
        if (strlen(tmpname) > FS_CUSTOM_MAX_NAME_LEN){
            errcode = AIC_EINVAL;
            break;
        }

        if ((strlen(tmpname) == 0) || ((strlen(tmpname) == 1) && (tmpname[0] == '/'))) {
            errcode = FS_MF_EBUSY;
            break;
        }

        filename = fs_mfat_fullpath_get(tmpname);

        if (count == 2)
            verify_alg = atoi((u8 *)paras_arr[1]);
        else
            verify_alg = FS_MF_VERIFY_MD5;

        if (verify_alg >= FS_MF_VERIFY_MAX) {
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
            case FS_MF_VERIFY_MD5:
                {
                    aos_memset(check, 0x0, sizeof(check));
#ifdef CONFIG_CRYPTO_MBEDTLS
#ifdef CONFIG_CRYPTO_MBEDTLS_3
                    ret = mbedtls_md5(data, rsize, check);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
                    mbedtls_md5(data, rsize, check);
#endif
#endif
                    if (ret < 0) {
                        errcode = AIC_ENOPROTOOPT;
                        break;
                    } else {
                        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                                 "+MFCHECK: ");
                        for (i = 0; i < 16; i++) {
                            snprintf(tmp, sizeof(tmp), "%02X", check[i]);
                            strcat(result_line, tmp);
                        }
                        line_pointers[0] = (ulong)result_line;
                    }
                }
                break;
            case FS_MF_VERIFY_SHA:
                {
                    aos_memset(check, 0x0, sizeof(check));
#ifdef CONFIG_CRYPTO_MBEDTLS
#ifdef CONFIG_CRYPTO_MBEDTLS_3
                    ret = mbedtls_sha1(data, rsize, check);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
                    mbedtls_sha1(data, rsize, check);
#endif
#endif
                    if (ret < 0) {
                        errcode = AIC_ENOPROTOOPT;
                        break;
                    } else {
                        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                                 "+MFCHECK: ");
                        for (i = 0; i < 20; i++) {
                            snprintf(tmp, sizeof(tmp), "%02X", check[i]);
                            strcat(result_line, tmp);
                        }
                        line_pointers[0] = (ulong)result_line;
                    }
                }
                break;
            case FS_MF_VERIFY_SHA256:
                {
                    aos_memset(check, 0x0, sizeof(check));
#ifdef CONFIG_CRYPTO_MBEDTLS
#ifdef CONFIG_CRYPTO_MBEDTLS_3
                    ret = mbedtls_sha256(data, rsize, check, 0);
#endif
#ifdef CONFIG_CRYPTO_MBEDTLS_2
                    mbedtls_sha256(data, rsize, check, 0);
#endif
#endif
                    if (ret < 0) {
                        errcode = AIC_ENOPROTOOPT;
                        break;
                    } else {
                        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                                 "+MFCHECK: ");
                        for (i = 0; i < 32; i++) {
                            snprintf(tmp, sizeof(tmp), "%02X", check[i]);
                            strcat(result_line, tmp);
                        }
                        line_pointers[0] = (ulong)result_line;
                    }
                }
                break;
            case FS_MF_VERIFY_CRC:
                {
                    crc16 = fs_crc_16(data, rsize);
                    snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                             "+MFCHECK: %02X", crc16);
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
                 "+CME ERROR: %d", errcode < 0 ? fs_map_mf_ret(errcode) : errcode);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/*
 *  Add fs AT handle here.
 */
const static AT_CMD_HANDLE fs_mf_at_handles[] =
{
    {"AT+MFCFG", fs_at_mfcfg_cmd, NULL, fs_at_mfcfg_set_cmd, NULL},
    {"AT+MFSINFO", NULL, NULL, fs_at_mfsinfo_set_cmd, NULL},
    {"AT+MFLIST", NULL, NULL, fs_at_mflist_set_cmd, NULL},
    {"AT+MFSIZE", NULL, NULL, fs_at_mfsize_set_cmd, NULL},
    {"AT+MFPUT", NULL, NULL, fs_at_mfput_set_cmd, NULL},
    {"AT+MFGET", NULL, NULL, fs_at_mfget_set_cmd, NULL},
    {"AT+MFOPEN", NULL, NULL, fs_at_mfopen_set_cmd, NULL},
    {"AT+MFREAD", NULL, NULL, fs_at_mfread_set_cmd, NULL},
    {"AT+MFWRITE", NULL, NULL, fs_at_mfwrite_set_cmd, NULL},
    {"AT+MFSYNC", NULL, NULL, fs_at_mfsync_set_cmd, NULL},
    {"AT+MFSEEK", NULL, NULL, fs_at_mfseek_set_cmd, NULL},
    {"AT+MFTRUNC", NULL, NULL, fs_at_mftrunc_set_cmd, NULL},
    {"AT+MFCLOSE", NULL, NULL, fs_at_mfclose_set_cmd, NULL},
    {"AT+MFDELETE", NULL, NULL, fs_at_mfdelete_set_cmd, NULL},
    {"AT+MFMOVE", NULL, NULL, fs_at_mfmove_set_cmd, NULL},
    {"AT+MFCHECK", NULL, NULL, fs_at_mfcheck_set_cmd, NULL},
    {AIC_NULL},
};

/**
 * fs_mf_at_cmd_init - fs mf at cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int fs_mf_at_cmd_init(void)
{
    list_head_init(&fs_at_open_list);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fs_mf_at_handles);

    fs_at_timer = aos_create_timer("fs custom at timer", fs_at_timeout,
                                   NULL, FS_AT_TIMEOUT,
                                   AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(fs_at_timer != AOS_NULL);

    return AIC_SUCCESS;
}

AIC_COMP_P2_INIT_EXPORT(fs_mf_at_cmd_init);

