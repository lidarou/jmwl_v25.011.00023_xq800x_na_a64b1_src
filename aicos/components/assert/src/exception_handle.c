/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines cortexa/cortexr exception handle interface.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01f86c
#define LOG_MODULE    AMOD_SYS

#include <stdio.h>
#include <assert_internal.h>
#include <system_assert.h>
#include <aic_assert_def.h>
#include <stdio.h>

extern struct assert_info s_assert_info;

#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
void exception_handle(char *except, u32 exception_type)
{
    u32 i = s_assert_info.assert_cnt;
    char assert_info[30] = {0};

    s_assert_info.info[i].assert_type = AR_EXCEPTION_ASSERT;
    s_assert_info.info[i].exception_type = exception_type;

    strncpy(assert_info, ASSERT_KEY, sizeof(assert_info) - 1);
    strncat(assert_info, except, sizeof(assert_info) - 1 - strlen(assert_info));

    system_assert(assert_info);
}

void assert_handle_und(void)
{
    exception_handle(" UND excpt", UND_EXCEPTION);
}

void assert_handle_swi(void)
{
    exception_handle(" SWI excpt", SWI_EXCEPTION);
}

void assert_handle_pabt(void)
{
    exception_handle(" P-ABT excpt", PABT_EXCEPTION);
}

void assert_handle_dabt(void)
{
    exception_handle(" D-ABT excpt", DABT_EXCEPTION);
}

void assert_handle_resrv(void)
{
    exception_handle(" RSV excpt", 0);
}

#elif defined(ASSERT_ARM_ARCH_M)

#define SCB_CFSR (*(volatile u32 *)0xE000ED28)
#define SCB_HFSR (*(volatile u32 *)0xE000ED2C)
#define SCB_MMAR (*(volatile u32 *)0xE000ED34)
#define SCB_BFAR (*(volatile u32 *)0xE000ED38)

#define SCB_MFSR (*(volatile u8 *)0xE000ED28)
#define SCB_BFSR (*(volatile u8 *)0xE000ED29)
#define SCB_UFSR (*(volatile u16 *)0xE000ED2A)

extern struct assert_m_fault_status_reg fault_reg;

void exception_handle(char *except, u32 exception_type, u32 exc_return)
{
    u32 i = s_assert_info.assert_cnt;
    char assert_info[30] = {0};

    s_assert_info.info[i].assert_type = M_EXCEPTION_ASSERT;
    s_assert_info.info[i].exception_type = exception_type;
    s_assert_info.info[i].regs.exc_return = exc_return;

    strncpy(assert_info, ASSERT_KEY, sizeof(assert_info) - 1);
    strncat(assert_info, except, sizeof(assert_info) - 1 - strlen(assert_info));

    system_assert(assert_info);
}

void assert_handle_hard_fault(u32 exc_return)
{
    u32 exception_type = 0;
    u32 status = SCB_HFSR;

    if ((SCB_HFSR & (1 << 30))) {
        if (SCB_MFSR) {
            fault_reg.fault_status = (u32)SCB_MFSR;

            if (SCB_MFSR & (1 << 7))
                fault_reg.fault_address = SCB_MMAR;

            exception_type = MEMMANAGE_EXCEPTION;
        } else if (SCB_BFSR) {
            fault_reg.fault_status = (u32)SCB_BFSR;

            if (SCB_BFSR & (1 << 7))
                fault_reg.fault_address = SCB_BFAR;

            exception_type = BUSFAULT_EXCEPTION;
        } else if (SCB_UFSR) {
            fault_reg.fault_status = (u32)SCB_UFSR;
            exception_type = USAGEFAULT_EXCEPTION;
        } else {
            exception_type = HARDFAULT_EXCEPTION;
        }
    } else {
        fault_reg.fault_status = status;
        exception_type = HARDFAULT_EXCEPTION;
    }

    exception_handle(" Hard fault", exception_type, exc_return);
}

void assert_handle_mem_manage(u32 exc_return)
{
    fault_reg.fault_status = (u32)SCB_MFSR;
    fault_reg.fault_address = SCB_MMAR;

    exception_handle(" Mem mgr fault", MEMMANAGE_EXCEPTION, exc_return);
}

void assert_handle_bus_fault(u32 exc_return)
{
    fault_reg.fault_status = (u32)SCB_BFSR;
    fault_reg.fault_address = SCB_BFAR;

    exception_handle(" Bus fault", BUSFAULT_EXCEPTION, exc_return);

}

void assert_handle_usage_fault(u32 exc_return)
{
    fault_reg.fault_status = (u32)SCB_UFSR;

    exception_handle(" Usage fault", USAGEFAULT_EXCEPTION, exc_return);
}
#else
#error "!!!CPU type not support!!!\n"
#endif
