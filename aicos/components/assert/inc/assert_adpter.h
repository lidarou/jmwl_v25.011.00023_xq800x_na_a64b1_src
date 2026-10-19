/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert adapter Interface.
 */

#ifndef __ASSERT_ADPTER_H__
#define __ASSERT_ADPTER_H__
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <../diag/inc/diag_cmd_codec.h>

#define ASSERT_LOG  alog_info

#define CP_RESET_ACK            "CP_RESET_ACK"
#define CP_ASSERT_ACK           "CP_ASSERT_ACK"
#define DUMP_FINISH             "DF"

#define ASSERT_ASSERT_SUBCMD    1

#define ASSERT_HLEP_SUBCMD      (u16)'h'
#define ASSERT_ALLINFO_SUBCMD   (u16)'a'
#define ASSERT_VERSION_SUBCMD   (u16)'v'
#define ASSERT_RESET_SUBCMD     (u16)'r'
#define ASSERT_DUMP_SUBCMD      (u16)'d'
#define ASSERT_DUMP_FINISH      (u16)'f'

void sys_notify_ap_assert(void);

#ifdef CONFIG_SUBSYS_AP
int ap_notify_other_sys_assert(u8 sys);
void ap_save_cp_reg_info(void);
#endif

#ifdef CONFIG_SUBSYS_CP
void cp_assert_notify_ap(const char *info_string);
bool cp_assert_is_brr_blocked(u8 brr);
diag_recv_cmd_t *assert_diag_cmd_recv(void);
void assert_diag_cmd_response(diag_rsp_cmd_t *rsp_cmd);
#endif

#endif /* __ASSERT_ADPTER_H__ */

