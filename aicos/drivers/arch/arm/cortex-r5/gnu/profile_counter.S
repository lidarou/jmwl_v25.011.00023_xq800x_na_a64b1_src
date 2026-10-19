
    .code 32
    .align  0

    .global cur_mcount_ptr

    .global __gnu_mcount_nc
    .type   __gnu_mcount_nc,function
__gnu_mcount_nc:
    /* save r0-r2 lr */
    STMDB   sp!, {r0-r2, lr}
    /* store prof data base addr and size */
    ldr r0, =cur_mcount_ptr
    ldr r2, [r0]
    ldr r1, =0x802c0000
    cmp r2, r1
    beq __gnu_mcnt_ret
    ldr r1, [sp, #0x10]
    str r1, [r2], #0x4
    str r2, [r0]
__gnu_mcnt_ret:
    /* switch lr and pc in sp memory */
    ldr r0, [sp, #0xc]
    ldr r1, [sp, #0x10]
    str r0, [sp, #0x10]
    str r1, [sp, #0xc]
    /* restore r0-r3,pc,lr */
    LDMIA   sp!, {r0-r2, lr, pc}

