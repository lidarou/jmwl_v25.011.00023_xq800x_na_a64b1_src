    .section .init, "ax"
    .code 32
    .align  0

    .global _mainCRTStartup
    .global _start
    .global start

    .extern mpu_config_before_load_image
    .extern tcm_preload_enable
    .extern g_sys_stack_limit
    .extern g_fiq_stack_limit
    .extern g_und_stack_limit
    .extern g_abt_stack_limit
    .extern g_irq_stack_limit
    .extern g_svc_stack_limit

start:
_start:
_mainCRTStartup:
/* 0. Set sctlr.v = 0 to use low vector address */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x2000
    mcr p15, 0, r0, c1, c0, 0
/* 1. Set general register to 0 */
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
/* 2. Set all modes stack */
/* 2.1 Set IRQ mode */
    mrs r0, cpsr
    mvn r1, #0x1F
    and r2, r1, r0
    orr r2, r2, #0x12
    msr cpsr, r2
    ldr r0, = g_irq_stack_limit
    ldr r13, [r0]
    mov r14, #0
/* 2.2 Set ABT mode */
    mrs r0, cpsr
    mvn r1, #0x1F
    and r2, r1, r0
    orr r2, r2, #0x17
    msr cpsr, r2
    ldr r0, = g_abt_stack_limit
    ldr r13, [r0]
    mov r14, #0
/*  2.3 Set FIQ mode */
    mrs r0, cpsr
    mvn r1, #0x1F
    and r2, r1, r0
    orr r2, r2, #0x11
    msr cpsr, r2
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
    ldr r0, = g_fiq_stack_limit
    ldr r13, [r0]
    mov r14, #0
/*  2.4 Set UND mode */
    mrs r0, cpsr
    mvn r1, #0x1F
    and r2, r1, r0
    orr r2, r2, #0x1B
    msr cpsr, r2
    ldr r0, = g_und_stack_limit
    ldr r13, [r0]
    mov r14, #0
/*  2.5 Set SYS mode */
    mrs r0, cpsr
    mvn r1, #0x1F
    and r2, r1, r0
    orr r2, r2, #0x1F
    msr cpsr, r2
    ldr r0, = g_sys_stack_limit
    ldr r13, [r0]
    mov r14, #0
/*  2.6 Set SVC mode */
    mrs r0, cpsr
    mvn r1, #0x1F
    and r2, r1, r0
    orr r2, r2, #0x13
    msr cpsr, r2
    ldr r0, = g_svc_stack_limit
    ldr r13, [r0]
    mov r14, #0
/* 3. Enable cpu features */
    /* 3.1 Enable tcm */
    blx tcm_preload_enable

    /* 3.2 Enable mpu and cache */
    blx mpu_config_before_load_image
    /* 3.3 Enable async abort exception */
    mrs r0, cpsr
    bic r0, r0, #0x100 /* Bit8 is the A enable */
    msr cpsr_xsf, r0

    bl  __main
