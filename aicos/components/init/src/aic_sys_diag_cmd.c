/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This system cmd realize file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ce8d6
#define LOG_MODULE    AMOD_INIT

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_version.h>
#include <diag_cmd_api.h>
#include <aic_assert.h>
#include <../assert/inc/assert_adpter.h>

#ifdef CONFIG_ALOG_MSG_SUPPORT
#if (defined CONFIG_COMM_PS || defined CONFIG_SDK_ENABLE)
int ps_init_prim_ver(void);
#else
int ps_init_prim_ver(void)
{
    return 0;
}
#endif
#endif

void sys_diag_version_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    char version[AIC_VERSION_LEN + 1];
    diag_rsp_cmd_t rsp;

    if (SUB_CMD_GET_VERSION == diag_cmd->subcmd) {
        aic_get_version(version, sizeof(version));
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_VERSION, diag_cmd->subcmd,
                     version, strlen(version) + 1);
    }
#ifdef CONFIG_ALOG_MSG_SUPPORT
    else if (SUB_CMD_GET_MSG_VERSION == diag_cmd->subcmd) {
        ps_init_prim_ver();
        /* this command is ap notify cp out put the version, don't response. */
        return;
    }
#endif
    else {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                     diag_cmd->m2, diag_cmd->sys, DIAG_CMD_VERSION,
                     diag_cmd->subcmd, NULL, 0);
    }

    diag_cmd_response(&rsp);
}

void sys_diag_assert_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    diag_rsp_cmd_t rsp;

    if (diag_cmd->subcmd != ASSERT_ASSERT_SUBCMD) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, 0,
                     diag_cmd->sys, DIAG_CMD_ASSERT, diag_cmd->subcmd, NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, 0, diag_cmd->sys, 
                 DIAG_CMD_ASSERT, diag_cmd->subcmd, NULL, 0);
    diag_cmd_response(&rsp);

    aos_sleep(1000);
    AIC_ASSERT(AIC_FALSE);
}

int sys_diag_cmd_init(void)
{
    /* register the version diag cmd handler. */
    diag_register_cmd_handler(DIAG_CMD_VERSION, sys_diag_version_handle, NULL);
    diag_register_cmd_handler(DIAG_CMD_ASSERT, sys_diag_assert_handle, NULL);

    return 0;
}
AIC_COMP_P2_INIT_EXPORT(sys_diag_cmd_init);
