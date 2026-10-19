/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file process calibation data diag command.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08db20
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_math.h>
#include <fmd.h>
#include <aic_list.h>
#include <aic_checksum.h>
#include <aic_boot_mode.h>
#include <sysoff.h>
#include "aic_cali_data_cmd.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define CALI_DATA_FILE_NAME                 "calibration"

#define CALI_DATA_TRANSFER_PACKET_LEN       0xFFFF

#define CALI_DATA_ACQUIRE_MUTEX(mutex_ptr, wait_timeout) \
if (mutex_ptr && AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    u32 mutex_status; \
    mutex_status = aos_get_mutex(mutex_ptr, wait_timeout); \
    AIC_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define CALI_DATA_RELEASE_MUTEX(mutex_ptr) \
if (mutex_ptr && AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    aos_put_mutex(mutex_ptr); \
}

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef enum {
    DIAG_CALI_DATA_READ_SUBCMD = 0,
    DIAG_CALI_DATA_WRITE_SUBCMD,
} CALI_DATA_DIAG_SUBCMD;

/**
 * calibration data lock callback.
 *
 * @func: calibration data lock function.
 * @param: callback parameter.
 * @node: calibration data lock node.
 */
typedef struct _cali_data_lock_cb {
    CALI_DATA_LOCK_FUNC func;
    void *param;
    struct list_head node;
} CALI_DATA_LOCK_CB_T;

/**
 * calibration data transfer packet head.
 *
 * @index: transfer packet index.
 * @checksum: packet data checksum.
 * @transfer_size: transfer packet max size.
 * @total_count: transfer total packet count.
 * @total_checksum: all calibration data checksum.
 * @total_size: calibration data total size.
 */
typedef struct _cali_data_transfer_head {
    u16 index;
    u16 checksum;
    u32 transfer_size;
    u16 total_count;
    u16 total_checksum;
    u32 total_size;
} CALI_DATA_TRANSFER_HEAD;

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_mutex_ptr cali_data_mutex = NULL;

LIST_HEAD(s_cali_data_list);

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
extern int ftl_export(char *name, u8 *buf);
extern int ftl_import(char *name, u8 *buf);

/**
 * aic_cali_data_register_hook -
 *  register callback to calibration data command,
 *  will be call it if calibration read or write.
 *
 * Parameter:
 *  @lock_func: input calibration data lock callback.
 *  @param: input callback param.
 *
 * Return:
 */
void aic_cali_data_register_hook(CALI_DATA_LOCK_FUNC lock_func, void* param)
{
    CALI_DATA_LOCK_CB_T *lock_cb = NULL;

    lock_cb = aos_zalloc(sizeof(CALI_DATA_LOCK_CB_T));
    AIC_ASSERT(lock_cb);

    lock_cb->func = lock_func;
    lock_cb->param = param;

    list_add(&lock_cb->node, &s_cali_data_list);
}

/**
 * aic_cali_data_unregister_hook -
 *  unregister callback to calibration data command.
 *
 * Parameter:
 *  @lock_func: input calibration data lock callback.
 *
 * Return:
 */
void aic_cali_data_unregister_hook(CALI_DATA_LOCK_FUNC lock_func)
{
    CALI_DATA_LOCK_CB_T *data_lock_cb = NULL;
    struct list_head *pos, *next;

    list_for_each_safe(pos, next, &s_cali_data_list) {
        data_lock_cb = list_entry(pos, CALI_DATA_LOCK_CB_T, node);
        if (data_lock_cb->func == lock_func) {
            list_del(&data_lock_cb->node);
            aos_free(data_lock_cb);
        }
    }
}

/**
 * aic_cali_data_lock_callback - calibration data lock callback.
 *
 * Parameter:
 * @lock: false: unlock; true: lock.
 *
 * Return:
 *  FS_SUCCESS pass, other fail.
 */
void aic_cali_data_lock_callback(boolean lock)
{
    CALI_DATA_LOCK_CB_T *data_lock_cb = NULL;
    struct list_head *pos, *next;

    if (lock) {
        CALI_DATA_ACQUIRE_MUTEX(cali_data_mutex, AOS_WAIT_FOREVER);
    }

    list_for_each_safe(pos, next, &s_cali_data_list) {
        data_lock_cb = list_entry(pos, CALI_DATA_LOCK_CB_T, node);
        data_lock_cb->func(lock, data_lock_cb->param);
    }

    if (!lock) {
        CALI_DATA_RELEASE_MUTEX(cali_data_mutex);
    }
}

/**
 * aic_cali_data_read - read calibration data.
 *
 * Parameter:
 * @diag_cmd: receive diag parameter
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_cali_data_read(diag_send_cmd_t *diag_cmd, void *pri)
{
    static u32 handle, total_size, reserved_size, offset, total_count;
    static u8 *data_buf = NULL, *rsp_buf = NULL;
    static u16 transfer_index, total_checksum;
    CALI_DATA_TRANSFER_HEAD *transfer_head;
    diag_rsp_cmd_t rsp;
    u32 send_data_len;
    u16 head_size;
    int errcode;

    transfer_head = (CALI_DATA_TRANSFER_HEAD *)diag_cmd->data;
    head_size = sizeof(CALI_DATA_TRANSFER_HEAD);

    if ((diag_cmd->data_len != head_size) ||
        (transfer_head->transfer_size == 0))
        goto read_fail;

    if (transfer_head->index == 0) {
        errcode = fmd_open(CALI_DATA_FILE_NAME, &handle);
        if (errcode != AIC_SUCCESS) {
            aicx_error(0x0807e4, "sd", "open %s partition error, errcode=%d\n",
                       CALI_DATA_FILE_NAME, errcode);
            goto read_fail;
        }

        errcode = fmd_get_partition_size(handle, &total_size);
        if (errcode != AIC_SUCCESS) {
            aicx_error(0x115229, "sd", "get %s partition size error, errcode=%d\n",
                       CALI_DATA_FILE_NAME, errcode);
            goto read_fail;
        }
        reserved_size = total_size;

        total_count = DIV_ROUND_UP(total_size, transfer_head->transfer_size);

        if (data_buf == NULL) {
            data_buf = aos_malloc(total_size);
            AIC_ASSERT(data_buf);
        }

        errcode = ftl_export(CALI_DATA_FILE_NAME, data_buf);
        AIC_ASSERT(errcode == AIC_SUCCESS);

        total_checksum = aic_calc_checksum(data_buf, total_size);

        transfer_index = 0;
        offset = 0;

        if (rsp_buf == NULL) {
            rsp_buf = aos_malloc(head_size + transfer_head->transfer_size);
            AIC_ASSERT(rsp_buf);
        }
    }

    if ((transfer_head->index != transfer_index) ||
        (transfer_head->index >= total_count))
        goto read_fail;

    send_data_len = transfer_head->transfer_size < reserved_size ?
                    transfer_head->transfer_size : reserved_size;
    aos_memcpy(rsp_buf + head_size, data_buf + offset, send_data_len);
    reserved_size -= send_data_len;
    offset += send_data_len;

    transfer_head = (CALI_DATA_TRANSFER_HEAD *)rsp_buf;
    transfer_head->index = transfer_index;
    transfer_head->checksum = aic_calc_checksum(rsp_buf + head_size,
                                                send_data_len);
    transfer_head->transfer_size = send_data_len;

    transfer_head->total_count = total_count;
    transfer_head->total_checksum = total_checksum;
    transfer_head->total_size = total_size;

    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                 diag_cmd->m2, diag_cmd->sys, DIAG_CMD_CALI_DATA,
                 diag_cmd->subcmd, rsp_buf, send_data_len + head_size);
    diag_cmd_response(&rsp);

    transfer_index++;

    if (reserved_size == 0) {
        aos_free(data_buf);
        data_buf = NULL;

        aos_free(rsp_buf);
        rsp_buf = NULL;

        transfer_index = 0;

        errcode = fmd_close(handle);
        AIC_ASSERT(errcode == AIC_SUCCESS);
        handle = 0;
    }

    return;

read_fail:
    if (data_buf) {
        aos_free(data_buf);
        data_buf = NULL;
    }

    if (rsp_buf) {
        aos_free(rsp_buf);
        rsp_buf = NULL;
    }

    transfer_index = 0;

    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                 diag_cmd->m2, diag_cmd->sys, DIAG_CMD_CALI_DATA,
                 diag_cmd->subcmd, NULL, 0);
    diag_cmd_response(&rsp);

    if (handle) {
        errcode = fmd_close(handle);
        AIC_ASSERT(errcode == AIC_SUCCESS);
        handle = 0;
    }
}

/**
 * aic_cali_data_write - write calibration data.
 *
 * Parameter:
 * @diag_cmd: receive diag parameter
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_cali_data_write(diag_send_cmd_t *diag_cmd, void *pri)
{
    static u32 handle = 0, total_size, transfer_index, reserved_size;
    static u8 *data_buf = NULL, *recv_buf = NULL;
    CALI_DATA_TRANSFER_HEAD *transfer_head;
    u16 head_size, checksum;
    diag_rsp_cmd_t rsp;
    int errcode;

    if (!aic_boot_mode_is_gxem()) {
        goto write_fail;
    }

    transfer_head = (CALI_DATA_TRANSFER_HEAD *)diag_cmd->data;
    head_size = sizeof(CALI_DATA_TRANSFER_HEAD);

    if (diag_cmd->data_len != (head_size + transfer_head->transfer_size))
        goto write_fail;

    if (transfer_head->index == 0) {
        errcode = fmd_open(CALI_DATA_FILE_NAME, &handle);
        if (errcode != AIC_SUCCESS) {
            aicx_error(0x0807e4, "sd", "open %s partition error, errcode=%d\n",
                       CALI_DATA_FILE_NAME, errcode);
            goto write_fail;
        }

        errcode = fmd_get_partition_size(handle, &total_size);
        if (errcode != AIC_SUCCESS) {
            aicx_error(0x115229, "sd", "get %s partition size error, errcode=%d\n",
                       CALI_DATA_FILE_NAME, errcode);
            goto write_fail;
        }
        if (data_buf == NULL) {
            data_buf = aos_malloc(total_size);
            AIC_ASSERT(data_buf);
        }
        aos_memset(data_buf, 0xFF, total_size);
        reserved_size = total_size;

        transfer_index = 0;
    }

    if ((transfer_head->index != transfer_index) ||
        (transfer_head->total_size > total_size))
        goto write_fail;

    recv_buf = (u8 *)diag_cmd->data;
    checksum = aic_calc_checksum(recv_buf + head_size,
                                 diag_cmd->data_len - head_size);
    if (checksum != transfer_head->checksum) {
        goto write_fail;
    }

    aos_memcpy((u8 *)data_buf + (total_size - reserved_size),
               recv_buf + head_size, diag_cmd->data_len - head_size);
    reserved_size -= diag_cmd->data_len - head_size;

    ++transfer_index;

    if (transfer_index == transfer_head->total_count) {
        if (reserved_size != (total_size - transfer_head->total_size)) {
            goto write_fail;
        }

        if (reserved_size) {
            aos_memset(data_buf + transfer_head->total_size, 0xFF,
                       reserved_size);
        }

        checksum = aic_calc_checksum(data_buf, total_size);
        if (checksum != transfer_head->total_checksum) {
            goto write_fail;
        }

        errcode = fmd_close(handle);
        AIC_ASSERT(errcode == AIC_SUCCESS);
        handle = 0;

        errcode = ftl_import(CALI_DATA_FILE_NAME, data_buf);
        AIC_ASSERT(errcode == AIC_SUCCESS);

        aos_free(data_buf);
        data_buf = NULL;

        transfer_index = 0;
    }

    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                 diag_cmd->m2, diag_cmd->sys, DIAG_CMD_CALI_DATA,
                 diag_cmd->subcmd, NULL, 0);
    diag_cmd_response(&rsp);

    return;

write_fail:
    if (data_buf) {
        aos_free(data_buf);
        data_buf = NULL;
    }

    transfer_index = 0;

    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                 diag_cmd->m2, diag_cmd->sys, DIAG_CMD_CALI_DATA,
                 diag_cmd->subcmd, NULL, 0);
    diag_cmd_response(&rsp);

    if (handle) {
        errcode = fmd_close(handle);
        AIC_ASSERT(errcode == AIC_SUCCESS);
        handle = 0;
    }
}

/**
 * aic_cali_data_diag_handle - calibration data diag handle.
 *
 * Parameter:
 * @diag_cmd: receive diag parameter
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_cali_data_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    diag_rsp_cmd_t rsp;

    do {
        switch (diag_cmd->subcmd) {
            case DIAG_CALI_DATA_READ_SUBCMD:
                aic_cali_data_lock_callback(true);
                aic_cali_data_read(diag_cmd, pri);
                aic_cali_data_lock_callback(false);
                break;
            case DIAG_CALI_DATA_WRITE_SUBCMD:
                aic_cali_data_lock_callback(true);
                aic_cali_data_write(diag_cmd, pri);
                break;
            default:
                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                             diag_cmd->m2, diag_cmd->sys, DIAG_CMD_CALI_DATA,
                             diag_cmd->subcmd, NULL, 0);
                diag_cmd_response(&rsp);
                break;
        }
    } while(0);
}

/**
 * aic_cali_data_cmd_init - calibration data diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_cali_data_cmd_init(void)
{
    cali_data_mutex = aos_create_mutex(CALI_DATA_FILE_NAME);
    AIC_ASSERT(cali_data_mutex);

    diag_register_cmd_handler(DIAG_CMD_CALI_DATA, aic_cali_data_diag_handle,
                              NULL);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_cali_data_cmd_init);

