/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is efuse diag cmd handle.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f19bd
#define LOG_MODULE    AMOD_DRV

#include<stdio.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_version.h>
#include <diag_cmd_api.h>
#include <efuse.h>

#define AIC_UID_LENGTH      50

void get_uid_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    char uid[AIC_UID_LENGTH + 1];
    diag_rsp_cmd_t rsp;
    int ret, len;

    ret = aic_get_uid(uid, &len);
    if (ret)
        ret = DIAG_RSP_USE_ERR;

    diag_set_rsp(&rsp, diag_cmd->sn, ret, diag_cmd->m2,
                 diag_cmd->sys, DIAG_CMD_UID, diag_cmd->subcmd,
                 uid, strlen(uid) + 1);

    diag_cmd_response(&rsp);
}

void get_pmic_id_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    char pmic_main_id[AIC_UID_LENGTH + 1];
    diag_rsp_cmd_t rsp;
    int pmic_id , len, ret = DIAG_RSP_SUCC;

    pmic_id = (aic_cmdline_get_value_int(cl_key_pmic_version) >> 16) & 0xFFFF;

    len = sprintf(pmic_main_id, "%x", pmic_id);
    if (!len)
        ret = DIAG_RSP_USE_ERR;

    diag_set_rsp(&rsp, diag_cmd->sn, ret, diag_cmd->m2,
                 diag_cmd->sys, DIAG_CMD_PMIC_ID, diag_cmd->subcmd,
                 pmic_main_id, sizeof(int));

    diag_cmd_response(&rsp);
}

int diag_efuse_cmd_init(void)
{
    /* register efuse uid get diag cmd handler. */
    diag_register_cmd_handler(DIAG_CMD_UID, get_uid_diag_handle, NULL);

    /* register pmic id get diag cmd handler. */
    diag_register_cmd_handler(DIAG_CMD_PMIC_ID, get_pmic_id_handle, NULL);

    return 0;
}
AIC_COMP_P2_INIT_EXPORT(diag_efuse_cmd_init);
