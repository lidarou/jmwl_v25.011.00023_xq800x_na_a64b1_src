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
#define AIC_THIS_FILE    0x1bc8eb
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
#include <slp_inc.h>
#include <errno.h>
#include <sys/mount.h>
#include "fs_at.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_timer_ptr fs_at_timer;
static boolean fs_at_timeout_flag = false;
static u32 fs_at_conn_id;

LIST_HEAD(fs_at_open_list);

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
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
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    FS_AT_CMD *at_cmd = (FS_AT_CMD *)find_info->param;
    u8 *result_line = NULL;
    int ret = AIC_SUCCESS;
    struct stat buf;

    if (!memcmp(at_cmd->cmd, "FSMEM", strlen("FSMEM")) &&
        strlen(at_cmd->cmd) == strlen("FSMEM")) {
        if (find_info->dent->d_type == DT_REG) {
            ret = stat(find_info->fullpath, &buf);
            if (ret != 0) {
                ret = errno;
                goto find_fail;
            }

            at_cmd->file_num++;
            at_cmd->file_size += buf.st_size;
        }
    } else if (!memcmp(at_cmd->cmd, "FSLS", strlen("FSLS")) &&
               strlen(at_cmd->cmd) == strlen("FSLS")) {
        AIC_ASSERT(at_cmd->ptr);
        result_line = at_cmd->ptr;

        if (find_info->dent->d_type & DT_DIR) {
            snprintf(result_line, NAME_MAX + 10, "+FSLS: %s", find_info->fullpath);
        } else {
            ret = stat(find_info->fullpath, &buf);
            if (ret != 0) {
                ret = errno;
                goto find_fail;
            }

            at_cmd->file_size += buf.st_size;
            snprintf(result_line, NAME_MAX + 10, "+FSLS: %s,%ld",
                     find_info->fullpath, buf.st_size);
        }

        result_line[strlen(result_line)] = '\0';

        line_pointers[0] = (ulong)result_line;
        aic_fill_transmit_cmd_resp(at_cmd->at_req, line_pointers, 1);
    }

find_fail:
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
u32 fs_at_upl_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSUPL: <filename>[,< filesize>[, <timeout>[,<ackmode>]]]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_upl_set_cmd - Upload a file to storage.
 *  response AT+FSUPL= <filename>[,< filesize>[, <timeout>[,<ackmode>]]]
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_upl_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL;
    static u32 filesize, timeout, recvlen;
    u16 count, line_count, checksum;
    static char *filename = NULL;
    static boolean ackmode;
    ssize_t rsize, wsize;
    struct statfs stfs;
    struct stat buf;
    int error_code;
    static int fd;
    long roffset;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;

    do {
        error_code = AIC_SUCCESS;
        if (at_req->at_mode == AT_CMD_MODE) {
            fd = -1;
            paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                     "AT+FSUPL=",
                                                     &count);
            if (!paras_arr || count < 1 || count > 4) {
                error_code = AIC_EINVAL;
                break;
            }

            if (strlen((char *)paras_arr[0]) > NAME_MAX) {
                error_code = AIC_ENAMETOOLONG;
                break;
            }

            filename = aos_zalloc(strlen((char *)paras_arr[0]) + 4);
            aos_memcpy(filename, (char *)paras_arr[0],
                       strlen((char *)paras_arr[0]));
            filesize = 10240;
            timeout = FS_AT_TIMEOUT;
            ackmode = false;
            recvlen = 0;

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

            if ((timeout < 1) || (timeout > 0xFFFF) || !filesize) {
                error_code = AIC_EINVAL;
                break;
            }

            timeout = timeout * 1000;

            error_code = statfs(filename, &stfs);
            if (error_code != AIC_SUCCESS) {
                error_code = -errno;
                break;
            }

            if (stfs.f_bfree == 0) {
                error_code = AIC_ENOSPC;
                break;
            }

            error_code = stat(filename, &buf);
            if (error_code == 0) {
                error_code = AIC_EEXIST;
                break;
            } else {
                error_code = AIC_SUCCESS;
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
            if (fs_at_timeout_flag) {
                error_code = AIC_ETIME;
                break;
            }

            if (strncmp(at_req->at_upstream_data, "+++", 3) == 0) {
UPL_DONE:
                error_code = close(fd);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }

                fd = open(filename, O_RDONLY);
                if (fd < 0) {
                    error_code = errno;
                    break;
                }

                error_code = fstat(fd, &buf);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }

                if (buf.st_size) {
                    data = aos_zalloc(buf.st_size);
                    rsize = read(fd, data, buf.st_size);
                    if (rsize < 0) {
                        error_code = errno;
                        break;
                    }

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
                         "+FSUPL: %d,%d", rsize, checksum);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                aos_free(filename);
                filename = NULL;
                aic_put_wakelock();
                break;
            }

            recvlen += at_req->upstream_data_len;
            if (recvlen > filesize) {
                wsize = at_req->upstream_data_len - (recvlen - filesize);
            } else {
                wsize = at_req->upstream_data_len;
            }

            if (wsize > 0) {
                rsize = write(fd, at_req->at_upstream_data, wsize);
                if (rsize < 0) {
                    error_code = -errno;
                    if (error_code != AIC_ENOSPC) {
                        break;
                    }
                }
            }

            if ((error_code == AIC_ENOSPC) || (recvlen >= filesize)) {
                error_code = AIC_SUCCESS;
                if (ackmode) {
                    strcpy(result_line, "A");
                    line_pointers[0] = (ulong)result_line;
                    error_code = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
                    if (error_code != AIC_SUCCESS)
                        break;
                }
                goto UPL_DONE;
            }

            aos_change_timer(fs_at_timer, timeout, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            error_code = AIC_SUCCESS;
            if (ackmode) {
                strcpy(result_line, "A");
                line_pointers[0] = (ulong)result_line;
                line_count = 1;
                break;
            }
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

        if (filename) {
            aos_free(filename);
            filename = NULL;
        }

        aic_put_wakelock();
    }

    if (data)
        aos_free(data);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_dwl_cmd - response AT+FSDWL=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_dwl_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"+FSDWL: <filename>";
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/**
 * fs_at_dwl_set_cmd - Download a file from storage.
 *  response AT+FSDWL=<filename>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_dwl_set_cmd(AT_REQ_RESP *at_req)
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
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+FSDWL=",
                                                 &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        filename = (char *)paras_arr[0];
        if (strlen(filename) > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
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
                     "+FSDWL: %ld,%d", st->st_size, checksum);
            error_code = AIC_SUCCESS;
            break;
        }

        data = aos_malloc(st->st_size);
        if (!data) {
            error_code = AIC_ENOMEM;
            break;
        }

        rsize = read(fd, data, st->st_size);
        if (rsize < 0) {
            error_code = errno;
            aos_free(data);
            break;
        }

        if (rsize) {
            checksum = xor_16_check(data, rsize);
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+FSDWL: %d,%d", rsize, checksum);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+FSDWL: %d,%d", rsize, checksum);
            error_code = AIC_SUCCESS;
            aos_free(data);
            break;
        }

        line_pointers[0] = (ulong)AT_RESULT_CONNECT;
        aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

        error_code = aic_transimt_reply_no_copy(at_req->serial_port_id,
                                                data, rsize);
        if (error_code != AIC_SUCCESS) {
            aos_free(data);
            break;
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

    if (st)
        aos_free(st);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_open_cmd - response AT+FSOPEN=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_open_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSOPEN: <filename>[,<mode>]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_open_query_cmd - response AT+FSOPEN?.
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
 * fs_at_open_set_cmd - response AT+FSOPEN=<filename>[,<mode>].
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
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    FS_AT_OPEN_FILE *open_file;
    u16 count, line_count;
    int fd, flags, mode;
    int error_code;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+FSOPEN=",
                                                 &count);
        if (!paras_arr || count < 1 || count > 2) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        if (strlen(name) > NAME_MAX) {
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

        fd = open(name, flags);
        if (fd < 0) {
            error_code = errno;
            break;
        }

        if (flags != O_RDONLY) {
            error_code = fsync(fd);
            if (error_code != AIC_SUCCESS) {
                error_code = errno;
                break;
            }
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
 * fs_at_read_cmd - response AT+FSREAD=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_read_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSREAD: <filehandle>[,<length>]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_read_set_cmd - response AT+FSREAD=<filehandle>[,<length>].
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
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL;
    struct list_head *p = NULL, *next;
    ssize_t rsize, read_size;
    FS_AT_OPEN_FILE *pEntry;
    struct stat *st = NULL;
    u16 count, line_count;
    int error_code;
    boolean find;
    ulong length;
    int fd;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+FSREAD=",
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
            if ((length >= st->st_size) || !length)
                rsize = st->st_size;
            else
                rsize = length;
        }

        snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "CONNECT: %d", rsize);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
        aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

        while (rsize > 0) {
            read_size = rsize > FS_AT_REPLY_MAX_SIZE ?
                        FS_AT_REPLY_MAX_SIZE : rsize;
            data = aos_malloc(read_size);
            if (!data) {
                error_code = AIC_ENOMEM;
                break;
            }

            read_size = read(fd, data, read_size);
            if (read_size < 0) {
                aos_free(data);
                error_code = errno;
                break;
            }

            if (read_size == 0) {
                aos_free(data);
                error_code = AIC_SUCCESS;
                break;
            }

            error_code = aic_transimt_reply_no_copy(at_req->serial_port_id,
                                                    data, read_size);
            if (error_code != AIC_SUCCESS) {
                aos_free(data);
                break;
            }

            rsize -= read_size;
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

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_write_cmd - response AT+FSWRITE=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_write_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSWRITE: <filehandle>[,<length>[,<timeout>]]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_write_set_cmd - response AT+FSWRITE=<filehandle>[,<length>[,<timeout>]].
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_write_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *data = NULL;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    static u32 length, timeout, recvlen;
    struct list_head *p = NULL, *next;
    ulong *paras_arr = NULL;
    FS_AT_OPEN_FILE *pEntry;
    static off_t start_pos;
    u16 count, line_count;
    ssize_t rsize, wsize;
    struct stat buf;
    int error_code;
    static int fd;
    off_t end_pos;
    boolean find;

    aos_deactivate_timer(fs_at_timer);
    fs_at_conn_id = at_req->serial_port_id;
    line_count = 0;
    error_code = AIC_SUCCESS;

    do {
        if (at_req->at_mode == AT_CMD_MODE) {
            paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                     "AT+FSWRITE=", &count);
            if (!paras_arr || count < 1 || count > 3) {
                error_code = AIC_EINVAL;
                break;
            }

            length = 0;
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

            if ((timeout < 1) || (timeout > 0xFFFF)) {
                error_code = AIC_EINVAL;
                break;
            }

            timeout = timeout * 1000;

            aos_change_timer(fs_at_timer, timeout, false);
            fs_at_timeout_flag = false;
            aos_activate_timer(fs_at_timer);

            start_pos = fs_getpos(fd);

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
                error_code = fsync(fd);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }

                error_code = fstat(fd, &buf);
                if (error_code != AIC_SUCCESS) {
                    error_code = errno;
                    break;
                }

                end_pos = fs_getpos(fd);
                if (end_pos < 0) {
                    error_code = errno;
                    break;
                }

                at_req->at_mode = AT_CMD_MODE;

                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         "+FSWRITE: %ld,%ld", end_pos - start_pos, buf.st_size);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_count = 2;
                error_code = AIC_SUCCESS;
                aic_put_wakelock();
                break;
            }

            recvlen += at_req->upstream_data_len;
            if (recvlen > length) {
                wsize = at_req->upstream_data_len - (recvlen - length);
            } else {
                wsize = at_req->upstream_data_len;
            }

            if (wsize > 0) {
                rsize = write(fd, at_req->at_upstream_data, wsize);
                if (rsize < 0) {
                    error_code = -errno;
                    if (error_code != AIC_ENOSPC) {
                        break;
                    }
                }
            }

            if ((error_code == AIC_ENOSPC) || (recvlen >= length)) {
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

        aic_put_wakelock();
    }

    if(data)
        aos_free(data);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_seek_cmd - response AT+FSSEEK=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_seek_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSSEEK: <filehandle>,<offset>[,<whence>]";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_seek_set_cmd - response AT+FSSEEK=<filehandle>,<offset>[,<whence>].
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
                                                 "AT+FSSEEK=", &count);
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
 * fs_at_position_cmd - response AT+FSPOSITION=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_position_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSPOSITION: <filehandle>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_position_set_cmd - response AT+FSPOSITION=<filehandle>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_position_set_cmd(AT_REQ_RESP *at_req)
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
                                                 "AT+FSPOSITION=", &count);
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
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+FSPOSITION: %ld", offset);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_close_cmd - response AT+FSCLOSE=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_close_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSCLOSE: <filehandle>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_close_set_cmd - response AT+FSCLOSE=<filehandle>.
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
                                                 "AT+FSCLOSE=", &count);
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
 * fs_at_del_cmd - response AT+FSDEL=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_del_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSDEL: <filename>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_del_set_cmd - Delete file in storage.
 *  1.AT+FSDEL=<filename>: delete the file
 *  2.AT+FSDEL="*": delete all the file.

 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_del_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    struct fs_loopfind_info *findinfo;
    u16 count, line_count, len;
    int error_code;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSDEL=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        len = strlen(name);
        if (len > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        if (name[len - 1] == '*') {
            findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
            AIC_ASSERT(findinfo);
            findinfo->deltype = FS_DEL_ALL;
            findinfo->findtype = FS_FIND_ALLDIR;
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
 * fs_at_mem_cmd - response AT+FSMEM=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mem_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSMEM: <name_pattern>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_mem_set_cmd - Gets memory space information.
 *  1.AT+FSMEM=<name_pattern>: response +FSMEM: <free_size>,<total_size>
 *  2.AT+FSMEM: response default name pattern, +FSMEM: <file_size>,<file_number>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mem_set_cmd(AT_REQ_RESP *at_req)
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
                                                 "AT+FSMEM=", &count);
        if (count == 0) {
            name = "";
        } else if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        } else {
            name = (char *)paras_arr[0];
        }

        if (strlen(name) > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        if (count) {
            error_code = fs_getdiskfreespace(name, &free_bytes, &total_bytes);
        } else {
            at_cmd = aos_zalloc(sizeof(FS_AT_CMD));
            at_cmd->at_req = at_req;
            at_cmd->cmd = "FSMEM";

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
                     "+FSMEM: %lld,%lld", free_bytes, total_bytes);
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+FSMEM: %lld,%lld",
                     at_cmd->file_size, at_cmd->file_num);
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
 * fs_at_ls_cmd - response AT+FSLS=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_ls_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSLS: <name_pattern>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_ls_set_cmd - List file information.
 *  1.AT+FSLS=<name_pattern>: response +FSLS: <filename>,<file_size>
 *  2.AT+FSLS: response spatial information for all files stored in UFS,
 *              +FSLS: <filename>,<file_size>
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_ls_set_cmd(AT_REQ_RESP *at_req)
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
                                                 "AT+FSLS=", &count);
        if (count == 0) {
            name = "/C/";
        } else if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        } else {
            name = (char *)paras_arr[0];
        }

        if (strlen(name) > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        at_cmd = aos_zalloc(sizeof(FS_AT_CMD));
        at_cmd->at_req = at_req;
        at_cmd->cmd = "FSLS";
        at_cmd->ptr = aos_zalloc(NAME_MAX + 10);
        AIC_ASSERT(at_cmd->ptr);

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);
        findinfo->findtype = FS_FIND_CURDIR;
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

/**
 * fs_at_mkdir_cmd - response AT+FSMKDIR=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_mkdir_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSMKDIR: <dir>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_mkdir_set_cmd - create directory.
 *  response AT+FSMKDIR=<dir>
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
    int error_code;
    char *name;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSMKDIR=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        if (strlen(name) > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        error_code = mkdir(name, O_RDWR | O_CREAT);
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

/**
 * fs_at_rmdir_cmd - response AT+FSRMDIR=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_rmdir_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSRMDIR: <dir>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_rmdir_set_cmd - remove directory.
 *  response AT+FSRMDIR=<dirname>
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
    struct fs_loopfind_info *findinfo;
    int error_code;
    char *name;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSRMDIR=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }


        name = (char *)paras_arr[0];
        if (strlen(name) > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
        AIC_ASSERT(findinfo);
        findinfo->findtype = FS_FIND_ALLDIR;
        findinfo->deltype = FS_DEL_ALL;
        findinfo->dir = name;
        error_code = fs_loopfind(name, findinfo);
        aos_free(findinfo);

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

/**
 * fs_at_cd_cmd - response AT+FSCD=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_cd_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSCD: <curr_dir>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_cd_query_cmd - Get current work directory.
 *  response AT+FSCD?
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
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errno);
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
 *  response AT+FSCD=<curr_dir>
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
    int error_code;
    char *name;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSCD=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        if (strlen(name) > NAME_MAX) {
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        error_code = chdir(name);
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

/**
 * fs_at_rename_cmd - response AT+FSRENAME=?
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_rename_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSRENAME: <old_name>,<new_name>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);

}

/**
 * fs_at_rename_set_cmd - file rename.
 *  response AT+FSRENAME: <old_name>,<new_name>
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
    char *name, *new_name;
    int error_code;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSRENAME=", &count);
        if (!paras_arr || count != 2) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        new_name = (char *)paras_arr[1];
        if ((strlen(name) > NAME_MAX) ||
            (strlen(new_name) > NAME_MAX)){
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        error_code = rename(name, new_name);
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

/**
 * fs_at_info_cmd - response AT+FSINFO=?
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_info_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSINFO: <filename>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_info_set_cmd - get file information.
 *  response AT+FSINFO: <filename>
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
    struct stat *st = NULL;
    u16 count, line_count;
    int error_code;
    char *name;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSINFO=", &count);
        if (!paras_arr || count != 1) {
            error_code = AIC_EINVAL;
            break;
        }

        name = (char *)paras_arr[0];
        if (strlen(name) > NAME_MAX){
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        st = aos_zalloc(sizeof(struct stat));
        error_code = stat(name, st);
        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code < 0 ? -error_code : error_code);
        line_pointers[0] = (ulong)result_line;
        line_count = 1;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+FSINFO: %ld,%lu", st->st_mode, st->st_size);
        line_pointers[0] = (ulong)result_line;
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_count = 2;
    }

    if (st)
        aos_free(st);

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_format_cmd - response AT+FSFORMAT=?
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_format_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSFORMAT: <fstype>,<path>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_format_set_cmd - format volume.
 *  response AT+FSFORMAT: <fstype>,<path>
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
    char *path, *fstype;
    struct statfs buf;
    int errcode;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSFORMAT=", &count);
        if (!paras_arr || count != 2) {
            errcode = AIC_EINVAL;
            break;
        } else {
            fstype = (char *)paras_arr[0];
            path = (char *)paras_arr[0];
        }

        if (strlen(path) > NAME_MAX){
            errcode = AIC_ENAMETOOLONG;
            break;
        }

        errcode = mkfs(fstype, path);
        if (errcode != AIC_SUCCESS) {
            errcode = errno;
            break;
        }

        errcode = statfs(path, &buf);
        if (errcode != AIC_SUCCESS) {
            errcode = errno;
            break;
        }
    } while(0);

    if (errcode != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode < 0 ? -errcode : errcode);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/**
 * fs_at_copy_cmd - response AT+FSCOPY=?
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_copy_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSCOPY: <src_path>,<dst_path>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_copy_set_cmd -
 *  copy source directory or file to destination directory or file.
 *  response AT+FSCOPY: <src_path>,<dst_path>
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
    char *src_path, *dst_path;
    u16 count, line_count;
    int error_code;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSCOPY=", &count);
        if (!paras_arr || count != 2) {
            error_code = AIC_EINVAL;
            break;
        }

        src_path = (char *)paras_arr[0];
        dst_path = (char *)paras_arr[1];
        if ((strlen(src_path) > NAME_MAX) ||
            (strlen(dst_path) > NAME_MAX)){
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        error_code = fs_copy(src_path, dst_path);
        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
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

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

/**
 * fs_at_move_cmd - response AT+FSMOVE=?
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 fs_at_move_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+FSMOVE: <src_path>,<dst_path>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * fs_at_move_set_cmd -
 *  move source directory or file to destination directory or file.
 *  response AT+FSMOVE: <src_path>,<dst_path>
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
    char *src_path, *dst_path;
    u16 count, line_count;
    int error_code;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FSMOVE=", &count);
        if (!paras_arr || count != 2) {
            error_code = AIC_EINVAL;
            break;
        }

        src_path = (char *)paras_arr[0];
        dst_path = (char *)paras_arr[1];
        if ((strlen(src_path) > NAME_MAX) ||
            (strlen(dst_path) > NAME_MAX)){
            error_code = AIC_ENAMETOOLONG;
            break;
        }

        error_code = fs_move(src_path, dst_path);
        if (error_code != AIC_SUCCESS) {
            error_code = errno;
            break;
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

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);
}

static void __fs_rwtest_entry(void *__length)
{
    #define TEST_NAME_LEN 50

    static char test_name[TEST_NAME_LEN];
    static uint32_t test_loop = 0;
    int fd = -1, err = -1;
    u8 test_val = 0x0;
    u16 file_id = 0, file_id_for_del = 0;
    u64 free_bytes = 0, total_bytes = 0;
    u32 test_length = (u32)(uintptr_t)__length;
    off_t offset;

    void *wbuf = aos_malloc(test_length);
    void *rbuf = aos_malloc(test_length);
    AOS_ASSERT(wbuf && rbuf);

    while (1) {
        snprintf(test_name, TEST_NAME_LEN, "/C/test_dir/fs_rwtest_file_%u", file_id++);
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

        err = fs_getdiskfreespace("/C", &free_bytes, &total_bytes);
        AOS_ASSERT(err == 0);
        if (free_bytes <= (test_length + 512)) {
            snprintf(test_name, TEST_NAME_LEN, "/C/test_dir/fs_rwtest_file_%u", file_id_for_del++);
            unlink(test_name);
        }

        if (++test_loop % 100 == 0) {
            aicx_info(0x17f990, "u", "LOOP [%u] fs_rwtest", test_loop);
        }

        aos_sleep(1);
    }

    if (fd >= 0)
        close(fd);

    if (wbuf)
        aos_free(wbuf);

    if (rbuf)
        aos_free(rbuf);
}

u32 fs_at_rwtest_set_cmd(AT_REQ_RESP *at_req)
{
    int err = AIC_SUCCESS;
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u16 count;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u32 length;
    u64 free_bytes = 0, total_bytes = 0;

    static int __started_flag;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+FSRWTEST=", &count);
        if (count > 1) {
            err = AIC_EINVAL;
            break;
        }

        if (__started_flag) {
            break;
        }

        if (count != 0) {
            length = atoi((u8 *)paras_arr[0]);
        } else {
            length = 8192;
        }

        aicx_info(0x0e68f2, "u", "fs_at_rwtest with r/w length of %u", length);

        err = mkdir("/C/test_dir", O_RDWR | O_CREAT);
        if (err) {
            err = AIC_ENOSPC;
            break;
        }

        err = fs_getdiskfreespace("/C", &free_bytes, &total_bytes);
        if (err || free_bytes < length) {
            aicx_error(0x144eaf, "dllullu", "fs_at_rwtest[%d] not enougth space[%llu/%llu]", err, free_bytes, total_bytes);
            err = AIC_ENOSPC;
            break;
        }

        aos_thread_id test_tid = aos_create_thread_simply(  "__fs_rwtest_entry",
                                                            __fs_rwtest_entry,
                                                            (void *)length,
                                                            4096,
                                                            30);
        if (test_tid == AOS_INVALID_THREAD_ID) {
            aicx_error(0x0adb27, "", "fs_at_rwtest failed to create test thread");
            err = AIC_ENOMEM;
            break;
        }

        err = AIC_SUCCESS;
    } while (0);

    if (err != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", -err);
        line_pointers[0] = (ulong)result_line;
    } else {
        __started_flag = 1;
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/*
 *  Add fs AT handle here.
 */
const static AT_CMD_HANDLE fs_at_handles[] =
{
    {"AT+FSUPL", fs_at_upl_cmd, NULL, fs_at_upl_set_cmd, NULL},
    {"AT+FSDWL", fs_at_dwl_cmd, NULL, fs_at_dwl_set_cmd, NULL},
    {"AT+FSOPEN", fs_at_open_cmd, fs_at_open_query_cmd, fs_at_open_set_cmd, NULL},
    {"AT+FSREAD", fs_at_read_cmd, NULL, fs_at_read_set_cmd, NULL},
    {"AT+FSWRITE", fs_at_write_cmd, NULL, fs_at_write_set_cmd, NULL},
    {"AT+FSSEEK", fs_at_seek_cmd, NULL, fs_at_seek_set_cmd, NULL},
    {"AT+FSPOSITION", fs_at_position_cmd, NULL, fs_at_position_set_cmd, NULL},
    {"AT+FSCLOSE", fs_at_close_cmd, NULL, fs_at_close_set_cmd, NULL},
    {"AT+FSDEL", fs_at_del_cmd, NULL, fs_at_del_set_cmd, NULL},
    {"AT+FSMEM", fs_at_mem_cmd, NULL, fs_at_mem_set_cmd, NULL},
    {"AT+FSLS", fs_at_ls_cmd, NULL, fs_at_ls_set_cmd, NULL},
    {"AT+FSMKDIR", fs_at_mkdir_cmd, NULL, fs_at_mkdir_set_cmd, NULL},
    {"AT+FSRMDIR", fs_at_rmdir_cmd, NULL, fs_at_rmdir_set_cmd, NULL},
    {"AT+FSCD", fs_at_cd_cmd, fs_at_cd_query_cmd, fs_at_cd_set_cmd, NULL},
    {"AT+FSRENAME", fs_at_rename_cmd, NULL, fs_at_rename_set_cmd, NULL},
    {"AT+FSINFO", fs_at_info_cmd, NULL, fs_at_info_set_cmd, NULL},
    {"AT+FSFORMAT", fs_at_format_cmd, NULL, fs_at_format_set_cmd, NULL},
    {"AT+FSCOPY", fs_at_copy_cmd, NULL, fs_at_copy_set_cmd, NULL},
    {"AT+FSMOVE", fs_at_move_cmd, NULL, fs_at_move_set_cmd, NULL},
    {"AT+FSRWTEST", NULL, NULL, fs_at_rwtest_set_cmd, NULL},
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
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fs_at_handles);

    fs_at_timer = aos_create_timer("fs at timer", fs_at_timeout,
                                   NULL, FS_AT_TIMEOUT,
                                   AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(fs_at_timer != AOS_NULL);

    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_P2_INIT_EXPORT(fs_at_cmd_init);

