/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "initcall.h"
#include "platform/boot.h"
#include "platform/platform.h"

struct boot_arm_vector_table *vt_cpy;

__attribute__((naked)) void
boot_jump_to_next_image(uint32_t reset_handler_addr)
{
    __asm volatile(
        "mov     r7, r0                  \n"
        "movs    r0, #0                  \n" /* Clear registers: R0-R12, */
        "mov     r1, r0                  \n" /* except R7 */
        "mov     r2, r0                  \n"
        "mov     r3, r0                  \n"
        "mov     r4, r0                  \n"
        "mov     r5, r0                  \n"
        "mov     r6, r0                  \n"
        "mov     r8, r0                  \n"
        "mov     r9, r0                  \n"
        "mov     r10, r0                 \n"
        "mov     r11, r0                 \n"
        "mov     r12, r0                 \n"
        "mov     lr,  r0                 \n"
        "bx      r7                      \n" /* Jump to Reset_handler */
    );
}

void boot_platform_quit(struct boot_arm_vector_table *vt)
{
    vt_cpy = vt;

    log_print("(%s) aic bootloader boot exit ...\nset msp = 0x%x, jump to 0x%x\n",
        platform.name, vt->msp, vt_cpy->reset);

    __set_MSPLIM(0);
    __set_MSP(vt->msp);
    __DSB();
    __ISB();

    boot_jump_to_next_image(vt_cpy->reset);

    log_print("boot_platform_quit fail... enter while(1).\n");
    while(1);
}
