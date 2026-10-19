/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert command interface.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x179636
#define LOG_MODULE    AMOD_SYS

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_math.h>
#include <ambox_api.h>
#include <adbg_cfg_api.h>
#include <aic_osa.h>
#include <aic_version.h>
#include <aic_boot_mode.h>
#include <aos_thread.h>
#include <assert_adpter.h>
#include <assert_cmd.h>
#include <assert_dump.h>
#include <core_op.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <stdio.h>
#include <stdarg.h>
#include <sprintf_nano.h>

#ifdef CONFIG_RTOS_LITEOS_M
#include <los_task.h>
#endif
#include "exception_parse.h"

#if 0
extern aos_thread_desc g_thread_desc_tbl[];
#endif

static LIST_HEAD(assert_cmd_list);

void assert_diag_send(diag_recv_cmd_t *diag_cmd, void *data, size_t size)
{
    bool b_log_dump, b_gxem;

    /* empty the mail before reset. */
    ambox_empty_recvbox_inassert();

    /* gxem mode can't used diag to dump, force to log dump. */
    b_gxem = aic_boot_mode_is_gxem();
    b_log_dump = adbg_is_ldump_on();

    if (diag_cmd->subcmd == ASSERT_DUMP_SUBCMD ||
        diag_cmd->subcmd == ASSERT_DUMP_FINISH) {
        /* dump by log */
        if (b_log_dump || b_gxem) {
            alog_dump(AIC_THIS_SYS, data, size);
            return;
        }
    }

#ifdef CONFIG_SUBSYS_CP
    /* gxem mode can't send and assert info by diag. */
    if (!b_gxem) {
        diag_rsp_cmd_t rsp;

        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, 0, diag_cmd->sys,
                     DIAG_CMD_ASSERT, diag_cmd->subcmd, data, size);
        assert_diag_cmd_response(&rsp);
    }
#endif
}

static void assert_output(diag_recv_cmd_t *diag_cmd, char *fmt, ...)
{
    size_t len = 0;
    va_list args;
    /* not used the stack, may cause stack overflow. */
    static char s_tmp_buf[256];

    /* buid the trace body. */
    va_start(args, fmt);
    len = vsnprintf_nano(s_tmp_buf, sizeof(s_tmp_buf) - 1, fmt, args);
    va_end(args);

    if (!len)
        return;

    /* gxem mode can't used diag to output assert info. */
    if (aic_boot_mode_is_gxem() || adbg_is_ldump_on())
        ASSERT_LOG("%s", s_tmp_buf);
    else
        assert_diag_send(diag_cmd, (void *)s_tmp_buf, len);
}

static void assert_register_cmd(u16 cmd_code, assert_cmd_func func,
                                char *description)
{
    struct assert_cmd_info *cmd;

    cmd = aos_malloc(sizeof(*cmd));
    AIC_ASSERT(cmd != NULL);

    list_head_init(&cmd->node);
    cmd->cmd_code = cmd_code;
    cmd->func = func;
    cmd->description = description;

    list_add_tail(&cmd->node, &assert_cmd_list);
}

static assert_cmd_func assert_get_cmd_func(u16 cmd_code)
{
    struct assert_cmd_info *cmd;

    list_for_each_entry(cmd, &assert_cmd_list, node, struct assert_cmd_info) {
        if (cmd_code == cmd->cmd_code)
            return cmd->func;
    }

    return NULL;
}

#ifdef CONFIG_SUBSYS_CP
static diag_recv_cmd_t *assert_get_diag_cmd(void)
{
    diag_recv_cmd_t *diag_cmd = NULL;
    diag_rsp_cmd_t rsp;

    diag_cmd = assert_diag_cmd_recv();
    if (!diag_cmd)
        return NULL;

    if (DIAG_CMD_ASSERT != diag_cmd->cmd) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_CMD, 0,
                     diag_cmd->sys, diag_cmd->cmd, diag_cmd->subcmd, NULL, 0);
        assert_diag_cmd_response(&rsp);
        return NULL;
    }

    return diag_cmd;
}
#endif

static void assert_output_assert_string(diag_recv_cmd_t *diag_cmd,
                                        struct assert_common_info *info)
{
    u32 assert_type = info->assert_type;
    const char *info_string = info->info_string;
    /* not used the stack, may cause stack overflow. */
    char *version_info = aos_malloc_relaxed(AIC_VERSION_LEN);

    if (version_info) {
        aic_get_version(version_info, AIC_VERSION_LEN);
        assert_output(diag_cmd, "\nSoftware version: \n%s\n", version_info);
        aos_free(version_info);
    }

#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
    if (AR_EXCEPTION_ASSERT == assert_type) {
        struct assert_ar_regs *regs_ptr = &(info->regs);
        char *excp_reason = NULL;
        u32 fault_address;

        assert_output(diag_cmd, "\nException at 0x%08x\nASSERT(%s)\n",
                      regs_ptr->cur_mode_reg.r14, info_string);

        if (DABT_EXCEPTION == info->exception_type) {
            excp_reason = exception_reason_parse(regs_ptr->dabt_reg.fsr,
                                                    regs_ptr->dabt_reg.afsr);
            fault_address = regs_ptr->dabt_reg.far;
        } else if (PABT_EXCEPTION == info->exception_type) {
            excp_reason = exception_reason_parse(regs_ptr->pabt_reg.fsr,
                                                    regs_ptr->pabt_reg.afsr);
            fault_address = regs_ptr->pabt_reg.far;
        }

        if (excp_reason)
            assert_output(diag_cmd, "Falut reason: %s\nFault address: 0x%08x\n",
                          excp_reason, fault_address);
    } else
#elif defined(ASSERT_ARM_ARCH_M)
    if (M_EXCEPTION_ASSERT == assert_type) {
        struct assert_m_regs *regs_ptr = &(info->regs);
        u32 exception_type = info->exception_type;
        char *excp_reason = NULL;

        assert_output(diag_cmd, "\nException at 0x%08x\nASSERT(%s)\n",
                        regs_ptr->exc_frame.pc, info_string);

        excp_reason = exception_reason_parse(regs_ptr->fault_reg.fault_status,
                                                exception_type);

        if (excp_reason) {
            assert_output(diag_cmd, "Falut reason: %s\n", excp_reason);

            if (regs_ptr->fault_reg.fault_status & (1 << 1 | 1 << 7))
                assert_output(diag_cmd, "Fault address: 0x%08x\n",
                                regs_ptr->fault_reg.fault_address);
        }
    } else
#else
#error "!!!CPU type not support!!!\n"
#endif
        assert_output(diag_cmd, "\n%s\n", info_string);
}

#if 0
static void assert_output_thread_info(diag_recv_cmd_t *diag_cmd,
                                      aos_thread_id thread_id)
{
    if (AOS_INVALID_THREAD_ID == thread_id) {
        assert_output(diag_cmd,
                      "%-*cNone thread!\n",
                      SPACE_NUM, SPACE_CHAR);
    } else {
#ifdef CONFIG_RTOS_THREADX

        TX_THREAD *thread_ptr = g_thread_desc_tbl[thread_id].tcb_ptr;

        assert_output(diag_cmd,
                      "%-*cName:         %s\n"
                      "%-*cStack_ptr:    0x%08x\n"
                      "%-*cStack_top:    0x%08x\n"
                      "%-*cStack_base:   0x%08x\n",
                      SPACE_NUM, SPACE_CHAR,
                      thread_ptr->tx_thread_name,
                      SPACE_NUM, SPACE_CHAR,
                      (u32)thread_ptr->tx_thread_stack_ptr,
                      SPACE_NUM, SPACE_CHAR,
                      (u32)thread_ptr->tx_thread_stack_start,
                      SPACE_NUM, SPACE_CHAR,
                      (u32)thread_ptr->tx_thread_stack_end);
#else
        TSK_INFO_S task_info;
        u32 task_id = g_thread_desc_tbl[thread_id].los_task;

        if (LOS_OK == LOS_TaskInfoGet(task_id, &task_info)) {
            assert_output(diag_cmd,
                      "%-*cName:         %s\n"
                      "%-*cStack_ptr:    0x%08x\n"
                      "%-*cStack_top:    0x%08x\n"
                      "%-*cStack_base:   0x%08x\n",
                      SPACE_NUM, SPACE_CHAR,
                      task_info.acName,
                      SPACE_NUM, SPACE_CHAR,
                      (u32)task_info.uwTopOfStack,
                      SPACE_NUM, SPACE_CHAR,
                      (u32)task_info.uwTopOfStack,
                      SPACE_NUM, SPACE_CHAR,
                      (u32)task_info.uwBottomOfStack);
        } else {
            assert_output(diag_cmd,
                      "%-*cthread_id: %d, thread info get failed!\n",
                      SPACE_NUM, SPACE_CHAR,
                      thread_id);
        }

#endif
    }
}

static void assert_output_system_context_info(diag_recv_cmd_t *diag_cmd,
                                              struct assert_common_info *info)
{
    aos_thread_id thread_id;
    u32 assert_type = info->assert_type;

#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
    if (AR_IRQ_ASSERT == assert_type) {
        assert_output(diag_cmd, "\n It is in IRQ!\n");
    } else if ((AR_EXCEPTION_ASSERT == assert_type) &&
                ((get_spsr() & 0x1f) == IRQ_MODE)) {
        assert_output(diag_cmd, "\n It is in IRQ before exception!\n");
    } else if ((AR_FIQ_ASSERT == assert_type) &&
                ((get_spsr() & 0x1f) == IRQ_MODE)) {
        assert_output(diag_cmd, "\n It is in IRQ before FIQ!\n");
#elif defined(ASSERT_ARM_ARCH_M)
    if (M_IRQ_ASSERT == assert_type) {
        assert_output(diag_cmd, "\n It is in IRQ!\n");
    } else if ((M_EXCEPTION_ASSERT == assert_type) &&
                !(info->regs.exc_return & 0x8)) {
        assert_output(diag_cmd,
                      "\n It is in exception or IRQ before this exception!\n");
#else
#error "!!!CPU type not support!!!\n"
#endif
    } else {
        assert_output(diag_cmd, "\nCurrent thread info:\n");

        thread_id = aos_identify_thread();
        assert_output_thread_info(diag_cmd, thread_id);
    }
}

static void assert_output_cpu_info(diag_recv_cmd_t *diag_cmd,
                                   struct assert_common_info *info)
{
    u32 assert_type = info->assert_type;

#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
    struct assert_ar_regs *reg_ptr = &(info->regs);

    if (AR_NORMAL_ASSERT == assert_type)
        assert_output(diag_cmd,
                      "\nCurrent is in SVC, "
                      "below is the regs before assert:\n");
    else if (AR_IRQ_ASSERT == assert_type)
        assert_output(diag_cmd,
                      "\nCurrent is in IRQ, "
                      "below is the regs before assert:\n");
    else if (AR_FIQ_ASSERT == assert_type)
        assert_output(diag_cmd,
                      "\nCurrent is in FIQ, "
                      "below is the regs before assert:\n");
    else
        assert_output(diag_cmd,
                      "\nCurrent is in EXCEPTION, "
                      "below is the regs before exception:\n");

    assert_output(diag_cmd,
                  "\nCurrent mode:\n"
                  "%-*cR0   = 0x%08x   R1   = 0x%08x\n"
                  "%-*cR2   = 0x%08x   R3   = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r0,
                  reg_ptr->cur_mode_reg.r1,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r2,
                  reg_ptr->cur_mode_reg.r3);

    assert_output(diag_cmd,
                  "%-*cR4   = 0x%08x   R5   = 0x%08x\n"
                  "%-*cR6   = 0x%08x   R7   = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r4,
                  reg_ptr->cur_mode_reg.r5,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r6,
                  reg_ptr->cur_mode_reg.r7);

    assert_output(diag_cmd,
                  "%-*cR8   = 0x%08x   R9   = 0x%08x\n"
                  "%-*cR10  = 0x%08x   R11  = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r8,
                  reg_ptr->cur_mode_reg.r9,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r10,
                  reg_ptr->cur_mode_reg.r11);

    assert_output(diag_cmd,
                  "%-*cR12  = 0x%08x   R13  = 0x%08x\n"
                  "%-*cR14  = 0x%08x   PC   = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r12,
                  reg_ptr->cur_mode_reg.r13,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r14,
                  reg_ptr->cur_mode_reg.pc);

    assert_output(diag_cmd,
                  "%-*cSPSR = 0x%08x   CPSR = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.spsr,
                  reg_ptr->cur_mode_reg.cpsr);

    assert_output(diag_cmd,
                  "\nSVC mode:\n"
                  "%-*cR13  = 0x%08x   R14  = 0x%08x\n"
                  "%-*cSPSR = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->svc_mode_reg.r13,
                  reg_ptr->svc_mode_reg.r14,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->svc_mode_reg.spsr);

    assert_output(diag_cmd,
                  "\nIRQ mode:\n"
                  "%-*cR13  = 0x%08x   R14  = 0x%08x\n"
                  "%-*cSPSR = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->irq_mode_reg.r13,
                  reg_ptr->irq_mode_reg.r14,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->irq_mode_reg.spsr);

    assert_output(diag_cmd,
                  "\nAbort mode:\n"
                  "%-*cR13  = 0x%08x   R14  = 0x%08x\n"
                  "%-*cSPSR = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->abt_mode_reg.r13,
                  reg_ptr->abt_mode_reg.r14,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->abt_mode_reg.spsr);

    assert_output(diag_cmd,
                  "\nUndefind mode:\n"
                  "%-*cR13  = 0x%08x   R14  = 0x%08x\n"
                  "%-*cSPSR = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->und_mode_reg.r13,
                  reg_ptr->und_mode_reg.r14,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->und_mode_reg.spsr);

    assert_output(diag_cmd,
                  "\nFIQ mode:\n"
                  "%-*cR8   = 0x%08x   R9   = 0x%08x\n"
                  "%-*cR10  = 0x%08x   R11  = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->fiq_mode_reg.r8,
                  reg_ptr->fiq_mode_reg.r9,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->fiq_mode_reg.r10,
                  reg_ptr->fiq_mode_reg.r11);

    assert_output(diag_cmd,
                  "%-*cR12  = 0x%08x   R13  = 0x%08x\n"
                  "%-*cR14  = 0x%08x   SPSR = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->fiq_mode_reg.r12,
                  reg_ptr->fiq_mode_reg.r13,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->fiq_mode_reg.r14,
                  reg_ptr->fiq_mode_reg.spsr);
#elif defined(ASSERT_ARM_ARCH_M)
    struct assert_m_regs *reg_ptr = &(info->regs);

    if (M_NORMAL_ASSERT == assert_type)
        assert_output(diag_cmd,
                      "\nCurrent is in Thread mode, "
                      "below is the regs before assert:\n");
    else if (M_IRQ_ASSERT == assert_type)
        assert_output(diag_cmd,
                      "\nCurrent is in Interrupt exception, "
                      "below is the regs before assert:\n");
    else
        assert_output(diag_cmd,
                      "\nCurrent is in Fault exception, "
                      "below is the regs before assert:\n");

    assert_output(diag_cmd,
                  "\nCurrent registers:\n"
                  "%-*cR0      = 0x%08x   R1   = 0x%08x\n"
                  "%-*cR2      = 0x%08x   R3   = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r0,
                  reg_ptr->cur_mode_reg.r1,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r2,
                  reg_ptr->cur_mode_reg.r3);

    assert_output(diag_cmd,
                  "%-*cR4      = 0x%08x   R5   = 0x%08x\n"
                  "%-*cR6      = 0x%08x   R7   = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r4,
                  reg_ptr->cur_mode_reg.r5,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r6,
                  reg_ptr->cur_mode_reg.r7);

    assert_output(diag_cmd,
                  "%-*cR8      = 0x%08x   R9   = 0x%08x\n"
                  "%-*cR10     = 0x%08x   R11  = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r8,
                  reg_ptr->cur_mode_reg.r9,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r10,
                  reg_ptr->cur_mode_reg.r11);

    assert_output(diag_cmd,
                  "%-*cR12     = 0x%08x   R13  = 0x%08x\n"
                  "%-*cR14     = 0x%08x   PC   = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r12,
                  reg_ptr->cur_mode_reg.r13,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.r14,
                  reg_ptr->cur_mode_reg.pc);

    assert_output(diag_cmd,
                  "%-*cXPSR    = 0x%08x   MSP  = 0x%08x\n"
                  "%-*cCONTROL = 0x%08x   PSP  = 0x%08x\n",
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.xpsr,
                  reg_ptr->cur_mode_reg.msp,
                  SPACE_NUM, SPACE_CHAR,
                  reg_ptr->cur_mode_reg.control,
                  reg_ptr->cur_mode_reg.psp);
#else
#error "!!!CPU type not support!!!\n"
#endif
}

static void assert_cmd_help(void *p1, void *p2)
{
    diag_recv_cmd_t *diag_cmd = (diag_recv_cmd_t *)p1;
    struct assert_cmd_info *cmd;

    list_for_each_entry(cmd, &assert_cmd_list, node, struct assert_cmd_info) {
        /* output command name and description */
        assert_output(diag_cmd, "%s: %s\n", (char *)&(cmd->cmd_code),
                                        cmd->description);
    }
}
#endif

static void assert_cmd_output_assert_info(void *p1, void *p2)
{
    diag_recv_cmd_t *diag_cmd = (diag_recv_cmd_t *)p1;
    struct assert_common_info *info = (struct assert_common_info *)p2;

    assert_output_assert_string(diag_cmd, info);
#if 0
    assert_output_cpu_info(diag_cmd, info);
    assert_output_system_context_info(diag_cmd, info);
#endif
    if (aic_boot_mode_is_gxem() || adbg_is_ldump_on())
        alog_flush_all_log_in_assert();
}

static void assert_cmd_memory_dump(void *p1, void *p2)
{
    diag_recv_cmd_t *diag_cmd = (diag_recv_cmd_t *)p1;

    assert_dump_handle(diag_cmd);
}

#if 0
static void assert_cmd_output_version(void *p1, void *p2)
{
    diag_recv_cmd_t *diag_cmd = (diag_recv_cmd_t *)p1;
    char version_info[AIC_VERSION_LEN];

    aic_get_version(version_info, sizeof(version_info));
    assert_output(diag_cmd, "\nSoftware version: \n%s\n", version_info);
}
#endif

#ifdef CONFIG_SUBSYS_AP
//int sstest = 1;
void assert_cmd_process(struct assert_common_info *info)
{
    assert_cmd_func func;
    diag_recv_cmd_t diag_cmd_data;
    diag_recv_cmd_t *diag_cmd = &diag_cmd_data;

   // while (sstest);

    /* assert info */
    diag_set_send(diag_cmd, 0, DIAG_RSP_SUCC, 0, AIC_SYS_AP,
                  DIAG_CMD_ASSERT, ASSERT_ALLINFO_SUBCMD, NULL, 0);
    func = assert_get_cmd_func(diag_cmd->subcmd);
    if (func)
        func((void *)diag_cmd, (void *)info);

    /* mem dump */
    if (adbg_is_ap_dump()) {
        diag_set_send(diag_cmd, 0, DIAG_RSP_SUCC, 0, AIC_SYS_AP,
                      DIAG_CMD_ASSERT, ASSERT_DUMP_SUBCMD, NULL, 0);
        func = assert_get_cmd_func(diag_cmd->subcmd);
        if (func)
            func((void *)diag_cmd, (void *)info);
    }

    while (1);
}
#endif

#ifdef CONFIG_SUBSYS_CP
static void assert_cmd_reset(void *p1, void *p2)
{
    diag_recv_cmd_t *diag_cmd = (diag_recv_cmd_t *)p1;
    diag_rsp_cmd_t rsp;

    /* send reset ack, and falling deadloop. */
    cp_assert_notify_ap(CP_RESET_ACK);
    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, 0, diag_cmd->sys,
                 diag_cmd->cmd, diag_cmd->subcmd, NULL, 0);
    assert_diag_cmd_response(&rsp);

    /* empty the mail before reset. */
    ambox_empty_recvbox_inassert();

    /* don't recive any cmd now, wait ap reset. */
    while (1);
}

void assert_cmd_process(struct assert_common_info *info)
{
    assert_cmd_func func;
    diag_recv_cmd_t *diag_cmd;

    while (1) {
        /* empty the mail before reset. */
        ambox_empty_recvbox_inassert();

        diag_cmd = assert_get_diag_cmd();
        if (diag_cmd) {
            func = assert_get_cmd_func(diag_cmd->subcmd);
            if (func)
                func((void *)diag_cmd, (void *)info);
#if 0
            else {
                diag_cmd->subcmd = 'h';
                assert_cmd_help((void *)diag_cmd, (void *)info);
            }
#endif
        }
    }
}
#endif

void assert_cmd_init(void)
{
#if 0
    assert_register_cmd(ASSERT_HLEP_SUBCMD, assert_cmd_help, "Help info");
    assert_register_cmd(ASSERT_VERSION_SUBCMD,
                        assert_cmd_output_version, "Software version");
#endif
    assert_register_cmd(ASSERT_ALLINFO_SUBCMD,
                        assert_cmd_output_assert_info,"Assert info");
    assert_register_cmd(ASSERT_DUMP_SUBCMD,
                        assert_cmd_memory_dump, "Dump memory");
#ifdef CONFIG_SUBSYS_CP
    assert_register_cmd('r', assert_cmd_reset, "Reset system");
#endif
}
