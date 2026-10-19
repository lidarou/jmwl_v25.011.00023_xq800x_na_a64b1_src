/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This log diag cmd header file.
 */
#ifndef __ALOG_DIAG_CMD_H__
#define __ALOG_DIAG_CMD_H__

enum {
    LOG_SUBCMD_BASE = 0x00,
    LOG_SUBCMD_GET_DBG_SWITCH,
    LOG_SUBCMD_GET_LOG_SWITCH,
    LOG_SUBCMD_GET_LOG_LEVEL,
    LOG_SUBCMD_GET_LOG_DEV,
    LOG_SUBCMD_GET_LOG_BAUD_RATE,
    LOG_SUBCMD_SET_DBG_SWITCH,
    LOG_SUBCMD_SET_LOG_SWITCH,
    LOG_SUBCMD_SET_LOG_LEVEL,
    LOG_SUBCMD_SET_LOG_DEV,
    LOG_SUBCMD_SET_LOG_BAUD_RATE,
    LOG_SUBCMD_SET_LOG_DUMP,
    LOG_SUBCMD_SET_PC_SAMP
};

int alog_cmd_get_cp_data(u8 subcmd, u32 *value);
int alog_cmd_set_cp_data(u8 subcmd, u32 value);

int alog_get_cp_log_level(u8 module, u8 *value);
int alog_set_cp_log_level(u8 module, u8 level);
int atdbg_cp_samppc_set(u8 act, u32 us);

#define alog_get_cp_dbg_gate(v)         alog_cmd_get_cp_data(LOG_SUBCMD_GET_DBG_SWITCH, v)
#define alog_get_cp_log_switch(v)       alog_cmd_get_cp_data(LOG_SUBCMD_GET_LOG_SWITCH, v)
#define alog_get_cp_log_dev(v)          alog_cmd_get_cp_data(LOG_SUBCMD_GET_LOG_DEV, v)
#define alog_get_cp_log_baud_rate(v)    alog_cmd_get_cp_data(LOG_SUBCMD_GET_LOG_BAUD_RATE, v)

#define alog_set_cp_dbg_gate(v)         alog_cmd_set_cp_data(LOG_SUBCMD_SET_DBG_SWITCH, v)
#define alog_set_cp_log_switch(v)       alog_cmd_set_cp_data(LOG_SUBCMD_SET_LOG_SWITCH, v)
#define alog_set_cp_log_dev(v)          alog_cmd_set_cp_data(LOG_SUBCMD_SET_LOG_DEV, v)
#define alog_set_cp_log_baud_rate(v)    alog_cmd_set_cp_data(LOG_SUBCMD_SET_LOG_BAUD_RATE, v)
#define alog_set_cp_ldump(v)            alog_cmd_set_cp_data(LOG_SUBCMD_SET_LOG_DUMP, v)
#define atdbg_samppc_set(v)             alog_cmd_set_cp_data(LOG_SUBCMD_SET_LOG_DUMP, v)


#endif /* __ALOG_DIAG_CMD_H__ */

