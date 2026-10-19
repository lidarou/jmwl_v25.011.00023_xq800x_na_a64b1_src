/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file process factory diag and at command.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0caae5
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <fmd.h>
#include <aic_md_api.h>
#include <aic_md_id.h>
#include <aic_bitmap.h>
#include <aic_checksum.h>
#include "aic_factory.h"
#include "SN.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FACTORY_FILE_NAME           "factory"
#define FACTORY_BAK_FILE_NAME       "factory_bak"
#define STATION_NAME                "station"
#define SN_NAME                     "sn"
#define FACTORY_MAX_NAME_LEN         0x16

#define FACTORY_ACQUIRE_MUTEX(mutex_ptr, wait_timeout) \
if (mutex_ptr && AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    u32 mutex_status; \
    mutex_status = aos_get_mutex(mutex_ptr, wait_timeout); \
    AIC_ASSERT(AOS_SUCCESS == mutex_status); \
}

#define FACTORY_RELEASE_MUTEX(mutex_ptr) \
if (mutex_ptr && AOS_INVALID_THREAD_ID != aos_identify_thread()) { \
    aos_put_mutex(mutex_ptr); \
}

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef enum {
    DIAG_GXEM_INFO_GET_SUBCMD = 0,
    DIAG_GXEM_INFO_SET_SUBCMD,
} GXEM_DIAG_SUBCMD;

typedef enum {
    DIAG_FACTORY_INFO_GET_SUBCMD = 0,
    DIAG_FACTORY_INFO_SET_SUBCMD,
    DIAG_FACTORY_INFO_GET_ALL_SUBCMD,
    DIAG_FACTORY_INFO_SET_ALL_SUBCMD,
} FACTORY_DIAG_SUBCMD;

typedef enum {
    FACTORY_STATION_NOT_DO = 0,
    FACTORY_STATION_FAIL   = 1,
    FACTORY_STATION_PASS   = 3,
} FACTORY_STATION_ERROR;

/**
 * factory station information.
 *
 * @idx: station index.
 * @name: station name.
 * @flag: station flag. 0x0-not do;0x1-fail;0x3-pass.
 */
typedef struct _factory_station_info {
    u8      idx;
    char    name[FACTORY_MAX_NAME_LEN];
    u8      flag;
} FACTORY_STATION_INFO;

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_mutex_ptr factory_mutex = NULL;
static u32 factory_file_size = 0;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * factory_file_open - open factory file.
 *
 * Parameter:
 * @name: input open file name.
 * @handle: output file handle.
 * @size: output factory file size.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int factory_file_open(char *name, u32 *handle, u32 *size)
{
    int errcode;

    FACTORY_ACQUIRE_MUTEX(factory_mutex, AOS_WAIT_FOREVER);

    errcode = fmd_open(name, handle);
    if (AIC_SUCCESS != errcode) {
        aicx_error(0x18372a, "sd", "%s open fail, errcode(%d).\n", name, errcode);
        return errcode;
    }

    errcode = fmd_get_partition_size(*handle, size);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x1391dc, "sd", "%s get size fail, errcode(%d).\n", name, errcode);

    return AIC_SUCCESS;
}

/**
 * factory_file_read - read data from factory file.
 *
 * Parameter:
 * @handle: input file handle.
 * @size: input factory file size.
 * @buf: output read data.
 *
 * Return:
 */
void factory_file_read(u32 handle, u32 size, u8 *buf)
{
    int errcode;

    errcode = fmd_read(handle, 0, size, buf);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x120758, "xd", "read size(0x%x) fail, errcode(%d).\n", size, errcode);
}

/**
 * factory_file_write - write buf to factory file.
 *
 * Parameter:
 * @handle: intput file handle.
 * @size: input write size.
 * @buf: input write buf.
 *
 * Return:
 */
void factory_file_write(u32 handle, u32 size, u8 *buf)
{
    int errcode;
    u8 *ptr;

    errcode = fmd_erase(handle, 0, size);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x1c4af4, "d", "erase fail, errcode(%d).\n", errcode);

    errcode = fmd_write(handle, 0, size, buf);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x14dd34, "xd", "write size(0x%x) fail, errcode(%d).\n", size, errcode);

    ptr = aos_zalloc(size);
    errcode = fmd_read(handle, 0, size, ptr);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x14dd34, "xd", "write size(0x%x) fail, errcode(%d).\n", size, errcode);

    if (memcmp(ptr, buf, size))
        AIC_ASSERT(0);

    aos_free(ptr);
}

/**
 * factory_file_close - close factory file.
 *
 * Parameter:
 * @handle: input file handle.
 *
 * Return:
 */
void factory_file_close(u32 handle)
{
    int errcode;

    errcode = fmd_close(handle);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x18ce68, "d", "factory close fail, errcode(%d).\n", errcode);

    FACTORY_RELEASE_MUTEX(factory_mutex);
}

/**
 * factory_data_check - check factory file.
 *
 * Parameter:
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
int factory_data_check()
{
    u16 ori_checksum, bak_checksum, checksum, data_size;
    u32 ori_handle, bak_handle, ori_size, bak_size;
    u8 *ori_buf, *bak_buf, status;
    int errcode;

    status = 0;
    errcode = AIC_SUCCESS;

    /* check ori partition. */
    errcode = factory_file_open(FACTORY_FILE_NAME,&ori_handle, &ori_size);
    if (AIC_SUCCESS != errcode)
        return errcode;

    ori_buf = aos_dma_malloc(ori_size);
    factory_file_read(ori_handle, ori_size, ori_buf);
    data_size = *(u16 *)ori_buf;
    if (data_size <= (ori_size - sizeof(checksum))) {
        ori_checksum = *(u16 *)(ori_buf + data_size);
        checksum = aic_calc_checksum(ori_buf, data_size);
        if (checksum == ori_checksum)
            status = BIT_0;
    }

    /* check backup partition. */
    errcode = factory_file_open(FACTORY_BAK_FILE_NAME, &bak_handle, &bak_size);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x18372a, "sd", "%s open fail, errcode(%d).\n",
                    FACTORY_BAK_FILE_NAME, errcode);

    bak_buf = aos_dma_malloc(bak_size);
    factory_file_read(bak_handle, bak_size, bak_buf);
    data_size = *(u16 *)bak_buf;
    if (data_size <= (bak_size - sizeof(checksum))) {
        bak_checksum = *(u16 *)(bak_buf + data_size);
        checksum = aic_calc_checksum(bak_buf, data_size);
        if (checksum == bak_checksum) {
            if (!status) {
                status |= BIT_1;
            } else if (status && checksum == ori_checksum)
                status |= BIT_1;
        }
    }

    switch (status) {
        case 1:
            aicx_info(0x1b4228, "", "factory back file is damage.\n");
            factory_file_write(bak_handle, ori_size, ori_buf);
            factory_file_size = ori_size;
            break;
        case 2:
            aicx_info(0x0842de, "", "factory ori file is damage.\n");
            factory_file_write(ori_handle, bak_size, bak_buf);
            factory_file_size = bak_size;
            break;
        case 3:
            aicx_info(0x0a272e, "", "factory file all is correct.\n");
            factory_file_size = ori_size;
            break;
        default:
            aicx_info(0x0c85a6, "", "factory file all is damage.\n");
            factory_file_size = 0;
            errcode = AIC_ENOEXEC;
            break;
    }

    factory_file_close(ori_handle);
    factory_file_close(bak_handle);

    aos_dma_free(ori_buf);
    aos_dma_free(bak_buf);

    return errcode;
}

/**
 * factory_data_read - read data from factory.
 *
 * Parameter:
 * @buf: read data buf.
 * @size: read data size.
 *
 * Return:
 */
void factory_data_read(u8 *buf, u32 size)
{
    u32 handle, file_size;
    u8 *file_buf;
    int errcode;

    errcode = factory_data_check();
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x0aa096, "d", "factory data read fail, errcode(%d).\n", errcode);

    /* check ori fatory file. */
    errcode = factory_file_open(FACTORY_FILE_NAME,&handle, &file_size);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                    0x18372a, "sd", "%s open fail, errcode(%d).\n",
                    FACTORY_FILE_NAME, errcode);

    file_buf = aos_zalloc(file_size);
    factory_file_read(handle, file_size, file_buf);

    AICX_ASSERT_IFX(size == file_size,
                    0x0dd60a, "xx", "factory size(0x%x 0x%x) error.\n", size, file_size);

    aos_memcpy(buf, file_buf, size);

    factory_file_close(handle);
    aos_free(file_buf);
}

/**
 * factory_data_write - write data to factory.
 *
 * Parameter:
 * @buf: write data buf.
 * @size: write data size.
 *
 * Return:
 */
void factory_data_write(u8 *buf, u32 size)
{
    u16 checksum, data_size;
    u32 file_size, handle;
    int errcode;

    data_size = *(u16 *)buf;
    AIC_ASSERT((data_size + sizeof(checksum)) <= size);

    checksum = aic_calc_checksum(buf, data_size);
    *(u16 *)(buf + data_size) = checksum;

    /* write ori factory file. */
    errcode = factory_file_open(FACTORY_FILE_NAME,&handle, &file_size);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode && size == file_size,
                    0x18372a, "sd", "%s open fail, errcode(%d).\n",
                    FACTORY_FILE_NAME, errcode);
    factory_file_write(handle, size, buf);
    factory_file_close(handle);

    /* write backup factory file. */
    errcode = factory_file_open(FACTORY_BAK_FILE_NAME,&handle, &file_size);
    AICX_ASSERT_IFX(AIC_SUCCESS == errcode && size == file_size,
                    0x18372a, "sd", "%s open fail, errcode(%d).\n",
                    FACTORY_BAK_FILE_NAME, errcode);
    factory_file_write(handle, size, buf);
    factory_file_close(handle);
}

/**
 * factory_get_config - get config form input factory buf.
 *
 * Parameter:
 * @buf: factory config buffer.
 * @config_name: input config name.
 * @config: output config.
 * @config_len: output config size..
 *
 * Return:
 */
void factory_get_config(u8 *buf, const char *config_name, u8 **config,
                        u16 *config_len)
{
    u16 len, total_len, count;
    u8 *ptr, *tmp_name;

    total_len = *(u16 *)buf;
    ptr = buf + sizeof(total_len);
    count = sizeof(total_len);
    while(1) {
        tmp_name = ptr;
        len = *(u16 *)(ptr + FACTORY_MAX_NAME_LEN);
        if (len == 0 || len == 0xFFFF)
            break;

        ptr += FACTORY_MAX_NAME_LEN + sizeof(u16);
        count += FACTORY_MAX_NAME_LEN + sizeof(u16);
        if (!memcmp(config_name, tmp_name, strlen(tmp_name)) &&
            (strlen(config_name) == strlen(tmp_name))) {
            *config = ptr;
            *config_len = len;
            break;
        }

        ptr += len;
        count += len;

        if (count >= total_len)
            break;
    }
}

/**
 * station_diag_handle - station diag handle.
 *
 * Parameter:
 * @diag_cmd: receive diag parameter
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void station_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    u8 *buf = NULL, *ptr, station_num, i, *station_config;
    FACTORY_STATION_INFO *station, *tmp_station;
    diag_rsp_cmd_t rsp;
    u16 rsp_len, len;
    int errcode;

    do {
        errcode = AIC_SUCCESS;

        if (!factory_file_size) {
            errcode = AIC_ENOENT;
            break;
        }

        buf = aos_zalloc(factory_file_size);
        factory_data_read(buf, factory_file_size);

        station_config = NULL;
        factory_get_config(buf, STATION_NAME, (u8 **)&station_config, &len);
        if (!station_config || (len < sizeof(FACTORY_STATION_INFO))) {
            errcode = AIC_ESRCH;
            break;
        }

        ptr = station_config;
        station_num = 0;
        while(1) {
            station = (FACTORY_STATION_INFO *)ptr;
            AIC_ASSERT(station->idx == station_num);
            station_num++;
            ptr += sizeof(FACTORY_STATION_INFO);
            if (ptr >= (station_config + len))
                break;
        }

        switch (diag_cmd->subcmd) {
            case DIAG_FACTORY_INFO_GET_SUBCMD:
                if (diag_cmd->data_len != sizeof(FACTORY_STATION_INFO)) {
                    errcode = AIC_EINVAL;
                    break;
                }
                tmp_station = (FACTORY_STATION_INFO *)diag_cmd->data;
                ptr = station_config;
                for (i = 0; i < station_num; i++) {
                    station = (FACTORY_STATION_INFO *)ptr;
                    if (!memcmp(station->name, tmp_station->name,
                                strlen(tmp_station->name)) &&
                        (strlen(station->name) == strlen(tmp_station->name)))
                        break;
                    ptr += sizeof(FACTORY_STATION_INFO);
                }
                if (i >= station_num) {
                    errcode = AIC_EINVAL;
                    break;
                }

                rsp_len = sizeof(FACTORY_STATION_INFO);
                break;
            case DIAG_FACTORY_INFO_SET_SUBCMD:
                if (diag_cmd->data_len != sizeof(FACTORY_STATION_INFO)) {
                    errcode = AIC_EINVAL;
                    break;
                }
                tmp_station = (FACTORY_STATION_INFO *)diag_cmd->data;
                ptr = station_config;
                for (i = 0; i < station_num; i++) {
                    station = (FACTORY_STATION_INFO *)ptr;
                    if (!memcmp(station->name, tmp_station->name,
                                strlen(tmp_station->name)) &&
                        (strlen(station->name) == strlen(tmp_station->name))) {
                        station->flag = tmp_station->flag;
                        break;
                    }
                    ptr += sizeof(FACTORY_STATION_INFO);
                }
                if (i >= station_num) {
                    errcode = AIC_EINVAL;
                    break;
                }

                factory_data_write(buf, factory_file_size);

                rsp_len = 0;
                break;
            case DIAG_FACTORY_INFO_GET_ALL_SUBCMD:
                if (diag_cmd->data_len != 0) {
                    errcode = AIC_EINVAL;
                    break;
                }
                ptr = station_config;
                rsp_len = station_num * sizeof(FACTORY_STATION_INFO);
                break;
            case DIAG_FACTORY_INFO_SET_ALL_SUBCMD:
                if (diag_cmd->data_len !=
                    station_num * sizeof(FACTORY_STATION_INFO)) {
                    errcode = AIC_EINVAL;
                    break;
                }

                tmp_station = (FACTORY_STATION_INFO *)diag_cmd->data;

                ptr = station_config;
                for (i = 0; i < station_num; i++) {
                    station = (FACTORY_STATION_INFO *)ptr;
                    if (memcmp(station->name, tmp_station[i].name,
                               strlen(tmp_station[i].name)) ||
                        (station->idx != tmp_station[i].idx) ||
                        (strlen(station->name) != strlen(tmp_station[i].name))) {
                        errcode = AIC_EINVAL;
                        break;
                    } else {
                        station->flag = tmp_station[i].flag;
                    }
                    ptr += sizeof(FACTORY_STATION_INFO);
                }

                if (errcode != AIC_SUCCESS)
                    break;

                factory_data_write(buf, factory_file_size);

                rsp_len = 0;
                break;
            default:
                errcode = AIC_EINVAL;
                break;
        }
    } while(0);

    if (AIC_SUCCESS == errcode) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_STATION, diag_cmd->subcmd,
                     ptr, rsp_len);
    } else {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                     diag_cmd->m2, diag_cmd->sys, DIAG_CMD_STATION,
                     diag_cmd->subcmd, NULL, 0);
    }

    diag_cmd_response(&rsp);

    if (buf)
        aos_free(buf);
}

int station_flag_get(char *name, u8 *flag)
{
    u8 *buf, *ptr, station_num, *station_config;
    FACTORY_STATION_INFO *station;
    int ret = AIC_EPERM;
    u16 len;

    if (!factory_file_size) {
        return AIC_ENOENT;
    }

    buf = aos_zalloc(factory_file_size);
    if (!buf)
        return AIC_ENOMEM;

    factory_data_read(buf, factory_file_size);

    station_config = NULL;
    factory_get_config(buf, STATION_NAME, (u8 **)&station_config, &len);
    if (!station_config || (len < sizeof(FACTORY_STATION_INFO))) {
        aos_free(buf);
        return AIC_ESRCH;
    }

    ptr = station_config;
    station_num = 0;
    while(1) {
        station = (FACTORY_STATION_INFO *)ptr;
        AIC_ASSERT(station->idx == station_num);
        if (!memcmp(station->name, name, strlen(name)) &&
            (strlen(station->name) == strlen(name))) {
            *flag = station->flag;
            ret = AIC_SUCCESS;
            break;
        }
        station_num++;
        ptr += sizeof(FACTORY_STATION_INFO);
        if (ptr >= (station_config + len)) {
            ret = AIC_EINVAL;
            break;
        }
    }

    aos_free(buf);

    return ret;
}

/**
 * station_at_cmd - response AT^STATION=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 station_at_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)"+STATION: <name>,<flag>";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * station_at_query_cmd - response AT^STATION?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 station_at_query_cmd(AT_REQ_RESP *at_req)
{
    u8 *buf = NULL, station_num, *ptr, *station_config;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    FACTORY_STATION_INFO *station;
    int errcode;
    u16 len;

    do {
        errcode = AIC_SUCCESS;

        if (!factory_file_size) {
            errcode = AIC_ENOENT;
            break;
        }

        buf = aos_zalloc(factory_file_size);
        factory_data_read(buf, factory_file_size);

        station_config = NULL;
        factory_get_config(buf, STATION_NAME, (u8 **)&station_config, &len);
        if (!station_config || (len < sizeof(FACTORY_STATION_INFO))) {
            errcode = AIC_ESRCH;
            break;
        }

        ptr = station_config;
        station_num = 0;
        while (1) {
            station = (FACTORY_STATION_INFO *)ptr;
            AIC_ASSERT(station->idx == station_num);
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+STATION: %d,%s,%d",
                     station->idx, station->name, station->flag);
            line_pointers[0] = (ulong)result_line;
            errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
            if (errcode != AIC_SUCCESS)
                break;
            ptr += sizeof(FACTORY_STATION_INFO);
            station_num++;
            if (ptr >= (station_config + len))
                break;
        }
    } while(0);

    if (errcode == AIC_SUCCESS) {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode);
        line_pointers[0] = (ulong)result_line;
    }

    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

    if (buf)
        aos_free(buf);

    return errcode;
}

/**
 * station_at_set_cmd - response AT^STATION=<name>,<flag>.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 station_at_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE], *paras_arr = NULL;
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *name, flag;
    u8 *buf = NULL, *ptr, station_num, i, *station_config;
    FACTORY_STATION_INFO *station;
    u16 count, len, line_count;
    int errcode;

    do {
        if (!factory_file_size) {
            errcode = AIC_ENOENT;
            break;
        }

        buf = aos_zalloc(factory_file_size);
        factory_data_read(buf, factory_file_size);

        station_config = NULL;
        factory_get_config(buf, STATION_NAME, (u8 **)&station_config, &len);
        if (!station_config || (len < sizeof(FACTORY_STATION_INFO))) {
            errcode = AIC_ESRCH;
            break;
        }

        ptr = station_config;
        station_num = 0;
        while(1) {
            station = (FACTORY_STATION_INFO *)ptr;
            AIC_ASSERT(station->idx == station_num);
            station_num++;
            ptr += sizeof(FACTORY_STATION_INFO);
            if (ptr >= (station_config + len))
                break;
        }

        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT^STATION=",
                                                 &count);
        if (!paras_arr || (count != 1 && count != 2)) {
            errcode = AIC_EINVAL;
            break;
        }

        flag = 0;
        name = (char *)paras_arr[0];
        if (count == 2)
            flag = atoi((u8 *)paras_arr[1]);

        ptr = station_config;
        for (i = 0; i < station_num; i++) {
            station = (FACTORY_STATION_INFO *)ptr;
            if (!memcmp(station->name, name,strlen(name)) &&
                (strlen(station->name) == strlen(name))) {
                if (count == 2)
                    station->flag = flag;
                errcode = AIC_SUCCESS;
                break;
            }
            ptr += sizeof(FACTORY_STATION_INFO);
        }
        if (i == station_num) {
            errcode = AIC_ENOENT;
            break;
        }
    } while(0);

    line_count = 1;
    if (errcode == AIC_SUCCESS) {
        if (count == 2) {
            factory_data_write(buf, factory_file_size);
            line_pointers[0] = (ulong)AT_RESULT_OK;
        } else {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+STATION: %s,%d", station->name, station->flag);
            line_pointers[0] = (ulong)result_line;
            line_pointers[1] = (ulong)AT_RESULT_OK;
            line_count = 2;
        }
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", errcode);
        line_pointers[0] = (ulong)result_line;
    }

    aic_free_paras_mem(paras_arr, count);
    errcode = aic_fill_transmit_cmd_resp(at_req, line_pointers, line_count);

    if (buf)
        aos_free(buf);

    return errcode;
}

/**
 * sn_update - update sn by factory setting.
 *
 * Parameter:
 *
 * Return:
 */
int sn_update()
{
    u8 *buf = NULL, *sn_config;
    SN_T sn = {0};
    int errcode;
    u16 len;

    do {
        errcode = AIC_SUCCESS;

        if (!factory_file_size) {
            errcode = factory_data_check();
        }

        if (errcode != AIC_SUCCESS) {
            break;
        }

        buf = aos_zalloc(factory_file_size);
        factory_data_read(buf, factory_file_size);

        sn_config = NULL;
        factory_get_config(buf, SN_NAME, (u8 **)&sn_config, &len);
        if (!sn_config ) {
            errcode = AIC_SUCCESS;
            break;
        }

        if (len > sizeof(SN_T)) {
            errcode =  AIC_E2BIG;
            break;
        }

        if (*(sn_config + len) == FACTORY_STATION_PASS) {
            errcode =  AIC_SUCCESS;
            break;
        }

        errcode = aic_md_read(MD_ID_SN0, sizeof(sn), (void *)&sn);
        AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                        0x04b9eb, "d", "read sn0 fail, errcode(%d).\n",
                        errcode);
        if (!memcmp(sn_config, sn.SN, strlen(sn.SN)) &&
            (strlen(sn_config) == strlen(sn.SN))) {
            errcode = AIC_SUCCESS;
            break;
        }

        aos_memset(&sn, 0x0, sizeof(sn));
        aos_memcpy((u8 *)sn.SN, sn_config, strlen(sn_config));
        errcode = aic_md_write(MD_ID_SN0, sizeof(sn), (void *)&sn);
        AICX_ASSERT_IFX(AIC_SUCCESS == errcode,
                        0x0b36e0, "d", "write sn0 fail, errcode(%d).\n",
                        errcode);

        *(sn_config + len) = FACTORY_STATION_PASS;

        factory_data_write(buf, factory_file_size);
    } while(0);

    if (buf)
        aos_free(buf);

    return errcode;
}

/**
 * factory_data_update - update data by factory setting.
 *
 * Parameter:
 *
 * Return:
 */
void factory_data_update(void)
{
    sn_update();
}

/*
 *  Add factory AT handle here.
 */
const static AT_CMD_HANDLE factory_at_handles[] =
{
    {"AT^STATION", station_at_cmd, station_at_query_cmd, station_at_set_cmd, NULL},
    {AIC_NULL},
};

/**
 * factory_cmd_init - factory diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int factory_cmd_init(void)
{
    factory_mutex = aos_create_mutex(FACTORY_FILE_NAME);
    AIC_ASSERT(factory_mutex);

    if (!factory_file_size) {
        factory_data_check();
    }

    if (factory_file_size) {
        diag_register_cmd_handler(DIAG_CMD_STATION, station_diag_handle, NULL);

        aic_add_at_cmd_handle((AT_CMD_HANDLE *)factory_at_handles);
    }

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(factory_cmd_init);

