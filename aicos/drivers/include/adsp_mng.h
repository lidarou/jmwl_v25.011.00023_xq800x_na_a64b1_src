/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    ADSP manager header file, including structor and API declair etc.f
 */

#include "snd_ipc.h"

#define ADSP_MVOL_LEVEL_MAX                 15

enum {
    ADSP_CMD_DUMP       = SND_IPC_CMD_BASE, /* 100 */
    ADSP_CMD_POWER_OFF,                     /* 101 */
    ADSP_CMD_LOG,                           /* 102 */
    ADSP_CMD_MD_MUSIC,                      /* 103 */
    ADSP_CMD_MD_REC,                        /* 104 */
    ADSP_CMD_MD_VOLTE,                      /* 105 */
    ADSP_CMD_MD_VOIP,                       /* 106 */
    ADSP_CMD_MASTER_VOL,                    /* 107 */
    ADSP_CMD_MUTE_UL,                       /* 108 */

    ADSP_CMD_DEBUG = 200,
    ADSP_CMD_ASSERT = 201,
};

struct adsp_cmd_common {
    u32 cmd;
};

struct adsp_cmd_dump {
    u32 cmd;
    u32 en;
    u32 pos;
};

struct adsp_cmd_log {
    u32 cmd;
    u32 is_on;
    u32 ipc_chn;
};

struct adsp_cmd_md {
    u32 cmd;
    u32 data;
};

struct adsp_cmd_master_vol {
    u32 cmd;
    u32 vol;
};

int adsp_power_on(BOOL is_high);
int adsp_power_off(void);

int adsp_set_master_vol(u32 vol);
int adsp_set_mute_ul(u32 is_mute);

int adsp_dump_set(u32 en, u32 pos);
int adsp_send_command(void *cmd_ptr);

