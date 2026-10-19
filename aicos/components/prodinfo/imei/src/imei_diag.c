/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic imei diag Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a30d0
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_nvram_id.h>
#include <aic_nvram_api.h>
#include <aic_errno.h>
#include "IMEI.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef enum {
    IMEI_DIAG_READ_SUBCMD = 0,
    IMEI_DIAG_WRITE_SUBCMD
} IMEI_DIAG_SUBCMD;

typedef enum {
    IMEI_0 = 0,
    IMEI_1,
    IMEI_MAX
} IMEI_NUM;

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static u16 imei_id[IMEI_MAX] = {
    NVRAM_ID_IMEI0,
    NVRAM_ID_IMEI1
};

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_imei_diag_handle - imei diag handle.
 *  diag cmd:
 *  magic | sn | sys | errno | cmd | subcmd | len | idx | data | tail
 *
 * Parameter:
 * @diag_cmd: receive diag parameter.
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_imei_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    u16 item_id, item_size, rsp_len;
    diag_rsp_cmd_t rsp;
    u8 *ptr = NULL;
    IMEI_T imei;
    int status;
    u8 idx;

    idx = *(u8 *)diag_cmd->data;
    if ((IMEI_MAX <= idx) || (!diag_cmd->data_len)) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_IMEI, diag_cmd->subcmd, NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    item_id = imei_id[idx];
    item_size = aic_nvram_get_length(imei_id[idx]);
    if (!item_size) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_IMEI, diag_cmd->subcmd, NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    do {
        status = aic_nvram_read(item_id, item_size, &imei);
        if (status != AIC_SUCCESS)
            break;

        rsp_len = sizeof(idx);

        ptr = aos_zalloc(item_size + rsp_len);
        AIC_ASSERT(ptr);

        switch (diag_cmd->subcmd) {
            case IMEI_DIAG_READ_SUBCMD:
                *ptr = idx;
                aos_memcpy(ptr + rsp_len, imei.IMEI, sizeof(imei.IMEI));
                rsp_len += sizeof(imei.IMEI);
                break;
            case IMEI_DIAG_WRITE_SUBCMD:
                if (sizeof(imei.IMEI) != (diag_cmd->data_len - rsp_len)) {
                    status = AIC_EINVAL;
                    break;
                }

                aos_memcpy(imei.IMEI, (u8 *)diag_cmd->data + rsp_len, sizeof(imei.IMEI));
                imei.IMEI[sizeof(imei.IMEI) - 1] |= 0xF;
                status = aic_nvram_sync_write(item_id, item_size, &imei, true);
                *ptr = idx;
                break;
            default:
                status = AIC_EINVAL;
                break;
        }
    } while(0);

    if (AIC_SUCCESS == status) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2, diag_cmd->sys,
                     DIAG_CMD_IMEI, diag_cmd->subcmd, ptr, rsp_len);
    } else {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_IMEI, diag_cmd->subcmd, NULL, 0);
    }
    diag_cmd_response(&rsp);

    if (ptr) {
        aos_free(ptr);
    }
}

/**
 * aic_imei_diag_cmd_init - imei diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_imei_diag_cmd_init(void)
{
    diag_register_cmd_handler(DIAG_CMD_IMEI, aic_imei_diag_handle, NULL);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_imei_diag_cmd_init);

