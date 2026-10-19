
/******************************************************************************/
/*                                                                            */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.      */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file audio diag module file, it contains serial diag command to
 *    communication with audio PC tool.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10b22c
#define LOG_MODULE    AMOD_AUDIO

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cmsis_compiler.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <diag_cmd_api.h>
#include <diag_cmd.h>

#include "audio_util.h"
#include "audio_nv.h"
#include "audio_md.h"

#define AUD_DIAG_CMD_READ       0
#define AUD_DIAG_CMD_WRITE      1
#define AUD_DIAG_CMD_GET_CUR    2
#define AUD_DIAG_CMD_APPLY      3
#define AUD_DIAG_CMD_QUERY      4

#define AUD_DIAG_E_NOMEM    1
#define AUD_DIAG_E_READ     2
#define AUD_DIAG_E_WRITE    3
#define AUD_DIAG_E_SCEN     4
#define AUD_DIAG_E_MODE     5

struct aud_diag_cmd_pair {
    u16 param : 8; /* [7:0] */
    u16 scen   : 4; /* [11:8] */
    u16 cmd   : 4; /* [12:15] */
};

union aud_diag_subcmd {
    u16 cmd;
    struct aud_diag_cmd_pair cmd_pair;
};

void audio_diag_handler(diag_send_cmd_t *diag_cmd, void *pri)
{
    union aud_diag_subcmd subcmd;
    diag_rsp_cmd_t rsp;
    u32 md_id;
    int ret, size, i;
    u8 *p_md = 0;
    u8 scen_id;

    subcmd.cmd = diag_cmd->subcmd;
    switch (subcmd.cmd_pair.cmd) {
    case AUD_DIAG_CMD_READ:
        p_md = audio_malloc(AUD_MD_SIZE_MAX);
        if (!p_md) {
            diag_set_rsp(&rsp, diag_cmd->sn, AUD_DIAG_E_NOMEM, diag_cmd->m2,
                         AIC_SYS_AP, DIAG_CMD_AUDIO, subcmd.cmd,
                         NULL, 0);
            break;
        }
        md_id = subcmd.cmd_pair.param;
        size = audio_md_read(md_id, p_md);
        if (size > 0)
            diag_set_rsp(&rsp, diag_cmd->sn, 0, diag_cmd->m2,
                         AIC_SYS_AP, DIAG_CMD_AUDIO, subcmd.cmd, p_md, size);
        else
            diag_set_rsp(&rsp, diag_cmd->sn, AUD_DIAG_E_READ, diag_cmd->m2,
                         AIC_SYS_AP, DIAG_CMD_AUDIO, subcmd.cmd, NULL, 0);
        break;
    case AUD_DIAG_CMD_WRITE:
        md_id = subcmd.cmd_pair.param;
        ret = audio_md_write(md_id, diag_cmd->data, diag_cmd->data_len);
        diag_set_rsp(&rsp, diag_cmd->sn, ret ? AUD_DIAG_E_WRITE : 0,
                     diag_cmd->m2, AIC_SYS_AP, DIAG_CMD_AUDIO,
                     subcmd.cmd, NULL, 0);
        break;
    case AUD_DIAG_CMD_GET_CUR:
        subcmd.cmd_pair.param = audio_md_get_cur();
        diag_set_rsp(&rsp, diag_cmd->sn, 0,
                     diag_cmd->m2, AIC_SYS_AP, DIAG_CMD_AUDIO,
                     subcmd.cmd, NULL, 0);
        break;
    case AUD_DIAG_CMD_APPLY:
        md_id = subcmd.cmd_pair.param;
        ret = audio_md_set_buf(md_id, diag_cmd->data, diag_cmd->data_len);
        diag_set_rsp(&rsp, diag_cmd->sn, ret ? AUD_DIAG_E_WRITE : 0,
                     diag_cmd->m2, AIC_SYS_AP, DIAG_CMD_AUDIO,
                     subcmd.cmd, NULL, 0);
        break;
    case AUD_DIAG_CMD_QUERY: {
        u16 qry_buf[20];
        u16 md_base;
        u8 cnt = 0;
        scen_id = subcmd.cmd_pair.scen;
        if (scen_id >= AUD_SCEN_MAX) {
            diag_set_rsp(&rsp, diag_cmd->sn, AUD_DIAG_E_SCEN, diag_cmd->m2,
                         AIC_SYS_AP, DIAG_CMD_AUDIO, subcmd.cmd,
                         NULL, 0);
            aicx_error(0x03043c, "d", "aud diag query error, scen_id=%d\n", scen_id);
            return;
        }
        md_base = audio_md_get_base(scen_id);
        if (!md_base) {
            diag_set_rsp(&rsp, diag_cmd->sn, AUD_DIAG_E_MODE, diag_cmd->m2,
                         AIC_SYS_AP, DIAG_CMD_AUDIO, subcmd.cmd,
                         NULL, 0);
            aicx_error(0x102f14, "d", "aud diag query, no base, scen_id=%d\n", scen_id);
            return;
        }
        for (i = 0; i < audio_md_get_count(scen_id); i++) {
            if (audio_md_check(md_base + i))
                qry_buf[cnt++] = md_base + i;
        }
        diag_set_rsp(&rsp, diag_cmd->sn, 0, diag_cmd->m2,
                     AIC_SYS_AP, DIAG_CMD_AUDIO, subcmd.cmd, qry_buf, cnt * 2);
        break;
    }
    default:
        aicx_error(0x01f949, "d", "aud diag handle, unknown subcmd=%d\n", subcmd.cmd_pair.cmd);
        return;
    }

    diag_cmd_response(&rsp);

    if (p_md)
        audio_free(p_md);

    return;
}

static int audio_diag_init(void)
{
    diag_register_cmd_handler(DIAG_CMD_AUDIO, audio_diag_handler, NULL);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(audio_diag_init);

