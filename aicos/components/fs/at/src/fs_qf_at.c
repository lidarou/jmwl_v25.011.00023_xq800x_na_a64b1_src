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
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x017365
#define LOG_MODULE    AMOD_FS

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <fs_api.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <aic_at_cmd_handle.h>
#include <aic_checksum.h>
#include <aic_serial_recv_send.h>
#include <aic_list.h>
#include <errno.h>
#include <slp_inc.h>
#include "fs_at.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FS_CUSTOM_MAX_NAME_LEN      (80)

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_timer_ptr fs_at_timer;
static boolean fs_at_timeout_flag = false;
static u32 fs_at_conn_id;
static struct list_head fs_at_open_list;
static u32 fs_at_max_data_len = 10240;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
char *fs_at_filename_format(char *name)
{
    if ((name[0] == '.') || (strlen(name) == 0)) {
        return NULL;
    }

    if ((strlen("UFS") == strlen(name)) &&
        !memcmp("UFS", name, strlen("UFS"))) {
        name = "C:/";
    } else if (!memcmp("UFS:", name, strlen("UFS:"))) {
        if (name[4] == '/') {
            name += 2;
            name[0] = 'C';
        } else {
            name += 1;
            aos_memcpy(name, "C:/", 3);
        }
    } else if ((strlen("SD") == strlen(name)) &&
               !memcmp("SD", name, strlen("SD"))) {
        name = "D:/";
    } else if (!memcmp("SD:", name, strlen("SD:"))) {
        if (name[3] == '/') {
            name += 1;
        } else {
            aos_memcpy(name, "D:/", 3);
        }
    }

    if (name[strlen(name) - 1] == '.') {
        name[strlen(name) - 1] = '\0';
    }

    return name;
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
    u8 *result_line = NULL, *fullpath = NULL;
    int ret = AIC_SUCCESS;
    struct stat buf;

    if (!memcmp(at_cmd->cmd, "QFLDS", strlen("QFLDS")) &&
        strlen(at_cmd->cmd) == strlen("QFLDS")) {
        if (find_info->dent->d_type == DT_REG) {

            ret = stat(find_info->fullpath, &buf);
            if (ret != 0) {
                ret = errno;
                goto find_fail;
            }

            at_cmd->file_size += buf.st_size;
            at_cmd->file_num++;
        }
    } else if (!memcmp(at_cmd->cmd, "QFLST", strlen("QFLST")) &&
               strlen(at_cmd->cmd) == strlen("QFLST")) {
        AIC_ASSERT(at_cmd->ptr);
        result_line = at_cmd->ptr;

        fullpath = aos_zalloc(strlen(find_info->fullpath) + 10);

        if (!memcmp(find_info->fullpath, "C:/", strlen("C:/")) ||
            !memcmp(find_info->fullpath, "D:/", strlen("D:/"))) {
            snprintf(fullpath, strlen(find_info->fullpath) + 10, "%s",
                     find_info->fullpath + 3);
        } else if (!memcmp(find_info->fullpath, "/C/", strlen("/C/")) ||
                   !memcmp(find_info->fullpath, "/D/", strlen("/D/"))) {
            snprintf(fullpath, strlen(find_info->fullpath) + 10, "%s",
                     find_info->fullpath + 3);
        } else {
            snprintf(fullpath, strlen(find_info->fullpath) + 10, "%s",
                     find_info->fullpath);
        }

        if (find_info->dent->d_type & DT_DIR) {
            snprintf(result_line, NAME_MAX, "+QFLST: %s/", fullpath);
        } else {
            ret = stat(find_info->fullpath, &buf);
            if (ret != 0) {
                ret = errno;
                goto find_fail;
            }

            at_cmd->file_size += buf.st_size;
            snprintf(result_line, NAME_MAX, "+QFLST: %s,%ld",
                     fullpath, buf.st_size);
        }

        result_line[strlen(result_line)] = '\0';

        line_pointers[0] = (ulong)result_line;
        aic_fill_transmit_cmd_resp(at_cmd->at_req, line_pointers, 1);
    }

find_fail:
    if (fullpath)
        aos_free(fullpath);
    return ret;
}

/**
 * fs_at_upl_cmd - response AT+FSUPL=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfupl_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSUPL: <filename>[,(1-<freesize>)[,(1-65535)[,(0,1)]]]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfupl_set_cmd - Upload a file to storage.
 *  response AT+QFUPL= <filename>[,(1-<freesize>)
 *                     [,(range of supported<timeout>)
 *                     [,(list of supported<ackmode>)]]]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfupl_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    static char *filename = NULL, *name = NULL, *data = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u16 count, line_count, checksum, A_cnt, i;
    static u32 filesize, timeout, recvlen;
    u64 free_bytes, total_bytes;
    static boolean ackmode;
    ssize_t rsize, wsize;
    struct stat st;
    int error_code;
    static int fd;
    long roffset;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;

    do {
        error_code = AIC_SUCCESS;
        count = 0;
        if (at_req->at_mode == AT_CMD_MODE) {
            fd = -1;
            paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                     "AT+QFUPL=",
                                                     &count);
            if (!paras_arr || count < 1 || count > 4) {
                error_code = AIC_EINVAL;
                break;
            }

            if (strlen((char *)paras_arr[0]) > FS_CUSTOM_MAX_NAME_LEN) {
                error_code = AIC_ENAMETOOLONG;
                break;
            }

            name = aos_zalloc(strlen((char *)paras_arr[0]) + 4);
            aos_memcpy(name, (char *)paras_arr[0],
                       strlen((char *)paras_arr[0]));
            filesize = fs_at_max_data_len;
            timeout = FS_AT_TIMEOUT;
            ackmode = false;
            recvlen = 0;

            filename = fs_at_filename_format(name);

            if (count == 2) {
                filesize = atoi((u8 *)paras_arr[1]);
            } else if (count == 3) {
                filesize = atoi((u8 *)paras_arr[1]);
                timeout = atoi((u8 *)paras_arr[2]);
            } else if (count == 4) {
                filesize = atoi((u8 *)paras_arr[1]);
                timeout = atoi((u8 *)paras_arr[2]);
                ackmode = atoi((u8 *)paras_arr[3]);
            }

            if ((filename == NULL) || (timeout < 1) || (timeout > 0xFFFF) || !filesize) {
                error_code = AIC_EINVAL;
                break;
            }

            timeout = timeout * 1000;

            error_code = fs_getdiskfreespace("C:/", &free_bytes, &total_bytes);
            if (error_code != AIC_SUCCESS) {
                break;
            }

            if (free_bytes == 0) {
                error_code = AIC_ENOSPC;
                break;
            }

            error_code = stat(filename, &st);
            if (error_code == 0) {
                error_code = AIC_EEXIST;
                break;
            } else {
                error_code = AIC_SUCCESS;
            }

            data = aos_zalloc(filesize);
            if (data == NULL) {
                error_code = AIC_ENOMEM;
                break;
            }

            fd = open(filename, O_RDWR | O_CREAT);
            if (fd < 0) {
                error_code = errno;
                break;
            }

            roffset = lseek(fd, 0, SEEK_SET);
            if (roffset != 0) {
                if (roffset < 0)
                    error_code = errno;
                else
                    error_code = AIC_EIO;
                break;
            }

            aos_change_timer(fs_at_timer, timeout, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            aic_get_wakelock();
            at_req->at_mode = AT_DATA_MODE;
            line_pointers[0] = (ulong)AT_RESULT_CONNECT;
            line_count = 1;
        } else if (at_req->at_upstream_data != AIC_NULL &&
                   at_req->at_mode == AT_DATA_MODE) {
            // if (fs_at_timeout_flag) {
            //     error_code = AIC_ETIME;
            //     break;
            // }

            if (fs_at_timeout_flag || (strncmp(at_req->at_upstream_data, "+++", 3) == 0)) {
UPL_DONE:
                if (recvlen) {
                    error_code = fs_getdiskfreespace("C:/", &free_bytes, &total_bytes);
                    if (error_code != AIC_SUCCESS) {
                        break;
                    }

                    if (recvlen > free_bytes)
                        recvlen = free_bytes;

                    rsize = write(fd, data, recvlen);
                    if (rsize < 0) {
                        error_code = errno;
                        break;
                    }
                } else {
                    rsize = 0;
                }

                if (rsize) {
                    checksum = xor_16_check(data, rsize);
                } else {
                    rsize = 0;
                    checksum = 0;
                }

                error_code = close(fd);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }
                fd = -1;

                at_req->at_mode = AT_CMD_MODE;

                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         "+QFUPL: %d,%d", rsize, checksum);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                aos_free(name);
                name = NULL;
                aos_free(data);
                data = NULL;
                aic_put_wakelock();
                break;
            }

            if ((recvlen + at_req->upstream_data_len) > filesize) {
                wsize = filesize -  recvlen;
            } else {
                wsize = at_req->upstream_data_len;
            }

            if (wsize > 0) {
                aos_memcpy(data + recvlen, at_req->at_upstream_data, wsize);
                recvlen += wsize;
            }

            if (ackmode & (at_req->upstream_data_len >= 1024)) {
                A_cnt =  at_req->upstream_data_len / 1024;
                for (i = 0; i < (A_cnt - 1); i++) {
                    strcpy(result_line, "A");
                    line_pointers[0] = (ulong)result_line;
                    aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
                }
                strcpy(result_line, "A");
                line_pointers[0] = (ulong)result_line;
                line_count = 1;
            }

            if (recvlen >= filesize) {
                error_code = AIC_SUCCESS;
                goto UPL_DONE;
            }

            aos_change_timer(fs_at_timer, timeout, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            error_code = AIC_SUCCESS;
        } else {
            error_code = AIC_EPERM;
            break;
        }

    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;

        if (at_req->at_mode != AT_CMD_MODE) {
            at_req->at_mode = AT_CMD_MODE;
        }

        if (fd >= 0) {
            close(fd);
            fd = -1;
            unlink(filename);
        }


        if (name) {
            aos_free(name);
            name = NULL;
        }

        if (data) {
            aos_free(data);
            data = NULL;
        }

        aic_put_wakelock();
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);
    if (line_count)
        error_code = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
    
    return error_code;
}

/**
 * fs_at_qfdwl_cmd - response AT+QFDWL=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfdwl_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"+QFDWL: <filename>";
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/**
 * fs_at_qfdwl_set_cmd - Download a file from storage.
 *  response AT+QFDWL=<filename>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfdwl_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL;
    u16 count, line_count, checksum;
    struct stat *st = NULL;
    char *filename;
    int error_code;
    ssize_t rsize;
    long roffset;
    int fd = -1;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+QFDWL=",
                                                 &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        filename = (char *)paras_arr[0];
        if (strlen(filename) > FS_CUSTOM_MAX_NAME_LEN) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        filename = fs_at_filename_format(filename);
        if (filename == NULL) {
            error_code = AIC_EINVAL;
            break;
        }

        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            error_code = errno;
            break;
        }

        roffset = lseek(fd, 0, SEEK_SET);
        if (roffset != 0) {
            if (roffset < 0)
                error_code = errno;
            else
                error_code = AIC_EIO;
            break;
        }

        st = aos_zalloc(sizeof(struct stat));
        error_code = fstat(fd, st);
        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
        }

        checksum = 0;
        if (!st->st_size) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+QFDWL: %ld,%d", st->st_size, checksum);
            error_code = AIC_SUCCESS;
            break;
        }

        data = aos_zalloc(st->st_size);

        rsize = read(fd, data, st->st_size);
        if (rsize < 0) {
            error_code = errno;
            break;
        }

        if (rsize) {
            checksum = xor_16_check(data, rsize);
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+QFDWL: %d,%d", rsize, checksum);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+QFDWL: %d,%d", rsize, checksum);
            error_code = AIC_SUCCESS;
            break;
        }

        line_pointers[0] = (ulong)AT_RESULT_CONNECT;
        aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

        while (rsize) {
            if (rsize > 0xFFFF) {
                error_code = aic_transmit_at_data(at_req->serial_port_id, data,
                                                  0xFFFF);
                if (error_code != AIC_SUCCESS)
                    break;
                rsize -= 0xFFFF;
            } else {
                error_code = aic_transmit_at_data(at_req->serial_port_id, data,
                                                  rsize);
                if (error_code != AIC_SUCCESS)
                    break;
                rsize -= rsize;
            }
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count= 2;
    }

    if (fd >= 0)
        fs_close(fd);

    if (data)
        aos_free(data);

    if (st)
        aos_free(st);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qfopen_cmd - response AT+QFOPEN=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfopen_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFOPEN: <filename>[,<mode>]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfopen_query_cmd - response AT+QFOPEN?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfopen_query_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    struct list_head *pos = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    u8 *result_line = NULL;
    int error_code;

    result_line = aos_zalloc(NAME_MAX + 16);
    AIC_ASSERT(result_line);

    error_code = AIC_SUCCESS;

    list_for_each_safe(pos, next, &fs_at_open_list) {
        pEntry = list_entry(pos, FS_AT_OPEN_FILE, node);
        snprintf(result_line, NAME_MAX + 16,
                 "%s,%u,%d", pEntry->name, pEntry->fd, pEntry->mode);
            result_line[strlen(result_line)] = '\0';
            line_pointers[0] = (ulong)result_line;
            error_code = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            if (error_code != AIC_SUCCESS)
                break;
    }

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    error_code = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

    if (result_line)
        aos_free(result_line);

    return error_code;
}

/**
 * fs_at_qfopen_set_cmd - response AT+QFOPEN=<filename>[,<mode>].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfopen_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    FS_AT_OPEN_FILE *open_file;
    u16 count, line_count;
    int fd, flags, mode;
    int error_code;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+QFOPEN=",
                                                 &count);
        if (!paras_arr || count < 1 || count > 2) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        if (strlen(name) > FS_CUSTOM_MAX_NAME_LEN) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        if (count == 2) {
            mode = atoi((u8 *)paras_arr[1]);
        } else {
            mode = 0;
        }

        switch (mode) {
            case 0:
                flags = O_RDWR | O_CREAT;
                break;
            case 1:
                flags = O_RDWR | O_TRUNC | O_CREAT;
                break;
            case 2:
                flags = O_RDONLY;
                break;
            default:
                flags = O_RDONLY;
                break;
        }

        if (mode > 2) {
            error_code = AIC_EACCES;
            break;
        }

        name = fs_at_filename_format(name);
        if (name == NULL) {
            error_code = AIC_EINVAL;
            break;
        }

        fd = open(name, flags);
        if (fd < 0) {
            error_code = errno;
            break;
        }

        error_code = fsync(fd);
        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
        }

        open_file = aos_zalloc(sizeof(FS_AT_OPEN_FILE));
        open_file->fd = fd;
        open_file->name = aos_zalloc(strlen(name) + 4);
        aos_memcpy(open_file->name, name, strlen(name));
        open_file->mode = mode;

        list_add(&open_file->node, &fs_at_open_list);

        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "%u", fd);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;

        error_code = AIC_SUCCESS;
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qfread_cmd - response AT+QFREAD=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfread_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFREAD: <filehandle>[,<length>]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfread_set_cmd - response AT+QFREAD=<filehandle>[,<length>].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfread_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL;
    struct list_head *p = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    struct stat *st = NULL;
    u16 count, line_count;
    int error_code;
    ssize_t rsize;
    boolean find;
    ulong length;
    int fd;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+QFREAD=",
                                                 &count);
        if (!paras_arr || count < 1 || count > 2) {
            error_code = AIC_EINVAL;
            break;
        }

        fd = atoi((u8 *)paras_arr[0]);
        if (count == 2) {
            length = atoi((u8 *)paras_arr[1]);
        } else {
            length = 0;
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
            error_code = AIC_EPERM;
            break;
        }

        st = aos_zalloc(sizeof(struct stat));
        error_code = fstat(fd, st);
        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
        }

        rsize = 0;
        if (st->st_size) {
            data = aos_zalloc(st->st_size);
            if (!length)
                length = st->st_size;
            rsize = read(fd, data, length);
            if (rsize < 0) {
                error_code = errno;
                break;
            }
        }

        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "CONNECT: %d", rsize);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
        aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

        while (rsize) {
            if (rsize > 0xFFFF) {
                error_code = aic_transmit_at_data(at_req->serial_port_id, data,
                                                  0xFFFF);
                if (error_code != AIC_SUCCESS)
                    break;
                rsize -= 0xFFFF;
            } else {
                error_code = aic_transmit_at_data(at_req->serial_port_id, data,
                                                  rsize);
                if (error_code != AIC_SUCCESS)
                    break;
                rsize -= rsize;
            }
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if(st)
        aos_free(st);

    if(data)
        aos_free(data);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qfwrite_cmd - response AT+QFWRITE=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfwrite_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFWRITE: <filehandle>[,<length>[,<timeout>]]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfwrite_set_cmd - response AT+QFWRITE=<filehandle>[,<length>[,<timeout>]].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfwrite_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    static u32 length, timeout, recvlen;
    struct list_head *p = NULL, *next;
    u64 free_bytes, total_bytes;
    static char *data = NULL;
    ulong *paras_arr = NULL;
    FS_AT_OPEN_FILE *pEntry;
    u16 count, line_count;
    ssize_t rsize, wsize;
    struct stat st;
    int error_code;
    static int fd;
    boolean find;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;
    error_code = AIC_SUCCESS;

    do {
        count = 0;
        if (at_req->at_mode == AT_CMD_MODE) {
            paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                     "AT+QFWRITE=", &count);
            if (!paras_arr || count < 1 || count > 3) {
                error_code = AIC_EINVAL;
                break;
            }

            length = fs_at_max_data_len;
            timeout = FS_AT_TIMEOUT;
            recvlen = 0;

            fd = atoi((u8 *)paras_arr[0]);
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
                error_code = AIC_EBADF;
                break;
            }

            if (count == 2) {
                length = atoi((u8 *)paras_arr[1]);
            } else if (count == 3) {
                length = atoi((u8 *)paras_arr[1]);
                timeout = atoi((u8 *)paras_arr[2]);
            }

            if ((timeout < 1) || (timeout > 0xFFFF) || (length == 0)) {
                error_code = AIC_EINVAL;
                break;
            }

            data = aos_zalloc(length);
            if (data == NULL) {
                error_code = AIC_ENOMEM;
                break;
            }

            timeout = timeout * 1000;

            aos_change_timer(fs_at_timer, timeout, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            aic_get_wakelock();
            at_req->at_mode = AT_DATA_MODE;
            line_pointers[0] = (ulong)AT_RESULT_CONNECT;
            line_count = 1;
            error_code = AIC_SUCCESS;
        } else if (at_req->at_upstream_data != AIC_NULL &&
                   at_req->at_mode == AT_DATA_MODE) {
            if (fs_at_timeout_flag) {
                error_code = AIC_ETIME;
                break;
            }

            if (strncmp(at_req->at_upstream_data, "+++", 3) == 0) {
WRITE_DONE:
                error_code = fs_getdiskfreespace("C:/", &free_bytes, &total_bytes);
                if (error_code != AIC_SUCCESS) {
                    break;
                }

                if (recvlen > free_bytes)
                    recvlen = free_bytes;

                rsize = write(fd, data, recvlen);
                if (rsize < 0) {
                    error_code = errno;
                    break;
                }

                error_code = fsync(fd);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }

                error_code = fstat(fd, &st);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }

                at_req->at_mode = AT_CMD_MODE;

                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         "+QFWRITE: %d,%ld", recvlen, st.st_size);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                error_code = AIC_SUCCESS;
                aos_free(data);
                data = NULL;
                aic_put_wakelock();
                break;
            }

            if (length && ((recvlen + at_req->upstream_data_len) > length)) {
                wsize = length - recvlen;
            } else {
                wsize = at_req->upstream_data_len;
            }

            if (wsize > 0) {
                aos_memcpy(data + recvlen, at_req->at_upstream_data, wsize);
                recvlen += wsize;
            }

            if (recvlen >= length) {
                error_code = AIC_SUCCESS;
                goto WRITE_DONE;
            }

            aos_change_timer(fs_at_timer, timeout, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);
        } else {
            error_code = AIC_EPERM;
            break;
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;

        if (at_req->at_mode != AT_CMD_MODE) {
            at_req->at_mode = AT_CMD_MODE;
        }

        if (data) {
            aos_free(data);
            data = NULL;
        }

        aic_put_wakelock();
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);
    if (line_count)
        error_code = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    return error_code;
}

/**
 * fs_at_qfseek_cmd - response AT+QFSEEK=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfseek_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFSEEK: <filehandle>,<offset>[,<whence>]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfseek_set_cmd - response AT+QFSEEK=<filehandle>,<offset>[,<whence>].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfseek_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct list_head *p = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    u16 count, line_count;
    long offset, roffset;
    int error_code;
    boolean find;
    u32 position;
    int fd;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QFSEEK=", &count);
        if (!paras_arr || count < 2 || count > 3) {
            error_code = AIC_EINVAL;
            break;
        }

        fd = atoi((u8 *)paras_arr[0]);
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
            error_code = AIC_EPERM;
            break;
        }

        offset = atoi((u8 *)paras_arr[1]);;
        position = 1;

        if (count == 3) {
            position = atoi((u8 *)paras_arr[2]);
        }

        roffset = lseek(fd, offset, position);
        if (roffset < 0) {
            error_code = errno;
            break;
        }
        error_code = AIC_SUCCESS;
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
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
 * fs_at_qfposition_cmd - response AT+QFPOSITION=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfposition_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFPOSITION: <filehandle>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfposition_set_cmd - response AT+QFPOSITION=<filehandle>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfposition_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct list_head *p = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    u16 count, line_count;
    int error_code;
    boolean find;
    long offset;
    int fd;

    do {
        error_code = AIC_SUCCESS;

        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QFPOSITION=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        fd = atoi((u8 *)paras_arr[0]);
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
            error_code = AIC_EPERM;
            break;
        }

        offset = fs_getpos(fd);
        if (offset < 0) {
            error_code = errno;
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? error_code : -error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+QFPOSITION: %ld", offset);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qfclose_cmd - response AT+QFCLOSE=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfclose_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFCLOSE: <filehandle>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfclose_set_cmd - response AT+QFCLOSE=<filehandle>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfclose_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct list_head *p = NULL, *next;
    FS_AT_OPEN_FILE *pEntry;
    u16 count, line_count;
    int error_code;
    boolean find;
    int fd;

    do {
        find = false;
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QFCLOSE=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        fd = atoi((u8 *)paras_arr[0]);
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
            error_code = AIC_EPERM;
            break;
        }

        error_code = close(fd);
        if (error_code < 0)
            error_code = errno;
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
        line_count = 1;
    }

    if (find) {
        list_del(p);
        aos_free(pEntry->name);
        aos_free(pEntry);
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qfdel_cmd - response AT+QFDEL=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfdel_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+QFDEL: <filename>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_qfdel_set_cmd - Delete file in storage.
 *  1.AT+QFDEL=<filename>: delete the file
 *  2.AT+QFDEL="*": delete all the file.

 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qfdel_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct fs_loopfind_info *findinfo;
    u16 count, line_count, len;
    int error_code;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QFDEL=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        len = strlen(name);
        if (len > FS_CUSTOM_MAX_NAME_LEN) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        name = fs_at_filename_format(name);
        if (name == NULL) {
            error_code = AIC_EINVAL;
            break;
        }

        if (name[len - 1] == '*') {
            findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
            AIC_ASSERT(findinfo);
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->deltype = FS_DEL_FILE;
            findinfo->dir = name;
            error_code = fs_loopfind(name, findinfo);
            aos_free(findinfo);
        } else {
            error_code = unlink(name);
        }

        if (error_code < 0)
            error_code = errno;
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
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
 * fs_at_qflds_cmd - response AT+QFLDS=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qflds_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_qflds_set_cmd - Gets memory space information.
 *  1.AT+QFLDS=<name_pattern>: response +QFLDS: <free_size>,<total_size>
 *  2.AT+QFLDS: response default name pattern, +QFLDS: <file_size>,<file_number>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qflds_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct fs_loopfind_info *findinfo;
    u64 free_bytes, total_bytes;
    FS_AT_CMD *at_cmd = NULL;
    u16 count, line_count;
    int error_code;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QFLDS=", &count);
        if (count == 0) {
            name = "C:/";
        } else if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        } else {
            name = (char *)paras_arr[0];
            if ((strlen("UFS") == strlen(name)) &&
                !memcmp("UFS", name, strlen("UFS"))) {
                name = "C:/";
            } else if ((strlen("SD") == strlen(name)) &&
                        !memcmp("SD", name, strlen("SD"))) {
                name = "D:/";
            } else {
                error_code = AIC_EINVAL;
                break;
            }
        }

        if (strlen(name) > FS_CUSTOM_MAX_NAME_LEN) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        name = fs_at_filename_format(name);
        if (name == NULL) {
            error_code = AIC_EINVAL;
            break;
        }

        if (count) {
            error_code = fs_getdiskfreespace(name, &free_bytes, &total_bytes);
        } else {
            at_cmd = aos_zalloc(sizeof(FS_AT_CMD));
            at_cmd->at_req = at_req;
            at_cmd->cmd = "QFLDS";

            findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
            AIC_ASSERT(findinfo);
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->dir = name;
            findinfo->callback = (u32)fs_at_loopfind_callback;
            findinfo->param = (void *)at_cmd;
            error_code = fs_loopfind(name, findinfo);
            aos_free(findinfo);
        }

        if (error_code < 0)
            error_code = errno;
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        if (count) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+QFLDS: %d,%d", (int)free_bytes, (int)total_bytes);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+QFLDS: %d,%d",
                     (int)at_cmd->file_size, (int)at_cmd->file_num);
            aos_free(at_cmd);
        }
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    if (count)
        aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_qflst_cmd - response AT+QFLST=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qflst_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_qflst_set_cmd - List file information.
 *  1.AT+QFLST=<name_pattern>: response +QFLST: <filename>,<file_size>
 *  2.AT+QFLST: response spatial information for all files stored in UFS,
 *              +QFLST: <filename>,<file_size>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_qflst_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct fs_loopfind_info *findinfo;
    FS_AT_CMD *at_cmd = NULL;
    int error_code;
    char *name;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QFLST=", &count);
        if (count == 0) {
            name = "C:";
        } else if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        } else {
            name = (char *)paras_arr[0];
        }

        if (strlen(name) > FS_CUSTOM_MAX_NAME_LEN) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        at_cmd = aos_zalloc(sizeof(FS_AT_CMD));

        name = fs_at_filename_format(name);
        if (name == NULL) {
            error_code = AIC_EINVAL;
            break;
        }

        if (!memcmp("C:", name ,strlen("C:"))) {
            at_cmd->target = "UFS";
        } else if (!memcmp("D:", name ,strlen("D:"))) {
            at_cmd->target = "SD";
        } else {
            at_cmd->target = "UFS";
        }

        at_cmd->at_req = at_req;
        at_cmd->cmd = "QFLST";
        at_cmd->ptr = aos_zalloc(NAME_MAX);
        AIC_ASSERT(at_cmd->ptr);

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);
        findinfo->findtype = FS_FIND_ALLDIR;
        findinfo->dir = name;
        findinfo->callback = (u32)fs_at_loopfind_callback;
        findinfo->param = (void *)at_cmd;
        error_code = fs_loopfind(name, findinfo);
        aos_free(findinfo);
        aos_free(at_cmd->ptr);
        aos_free(at_cmd);

        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/*
 *  Add fs AT handle here.
 */
const static AT_CMD_HANDLE fs_custom_at_handles[] =
{
    {"AT+QFUPL", fs_at_qfupl_cmd, NULL, fs_at_qfupl_set_cmd, NULL},
    {"AT+QFDWL", fs_at_qfdwl_cmd, NULL, fs_at_qfdwl_set_cmd, NULL},
    {"AT+QFOPEN", fs_at_qfopen_cmd, fs_at_qfopen_query_cmd, fs_at_qfopen_set_cmd, NULL},
    {"AT+QFREAD", fs_at_qfread_cmd, NULL, fs_at_qfread_set_cmd, NULL},
    {"AT+QFWRITE", fs_at_qfwrite_cmd, NULL, fs_at_qfwrite_set_cmd, NULL},
    {"AT+QFSEEK", fs_at_qfseek_cmd, NULL, fs_at_qfseek_set_cmd, NULL},
    {"AT+QFPOSITION", fs_at_qfposition_cmd, NULL, fs_at_qfposition_set_cmd, NULL},
    {"AT+QFCLOSE", fs_at_qfclose_cmd, NULL, fs_at_qfclose_set_cmd, NULL},
    {"AT+QFDEL", fs_at_qfdel_cmd, NULL, fs_at_qfdel_set_cmd, NULL},
    {"AT+QFLDS", fs_at_qflds_cmd, NULL, fs_at_qflds_set_cmd, NULL},
    {"AT+QFLST", fs_at_qflst_cmd, NULL, fs_at_qflst_set_cmd, NULL},
    {AIC_NULL},
};

/**
 * fs_custom_at_cmd_init - fs custom at cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int fs_custom_at_cmd_init(void)
{
    list_head_init(&fs_at_open_list);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fs_custom_at_handles);

    fs_at_timer = aos_create_timer("fs custom at timer", fs_at_timeout,
                                   NULL, FS_AT_TIMEOUT,
                                   AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(fs_at_timer != AOS_NULL);

    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_P2_INIT_EXPORT(fs_custom_at_cmd_init);

