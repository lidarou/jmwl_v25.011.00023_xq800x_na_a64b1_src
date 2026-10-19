/* .text is used instead of .section .text so it works with arm-aout too.  */
    .section .init, "ax"
    .code 32
    .align  0

    .global reset_handler
    .global _mainCRTStartup
    .global _start
    .global start

    .extern main
    .extern mpu_config_before_load_image
    .extern tcm_preload_enable
    .extern g_sys_stack_limit
    .extern g_fiq_stack_limit
    .extern g_und_stack_limit
    .extern g_abt_stack_limit
    .extern g_irq_stack_limit
    .extern g_svc_stack_limit

#ifdef CONFIG_DBG_CPPC_SAMP
    .extern     pc_samp_base
    .extern     pc_samp_end
    .extern     tmr_clr_reg
#endif

reset_handler:
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
#ifdef  CONFIG_DBG_CPPC_SAMP
    /* int fiq pc sample base and end */
    ldr r0, = pc_samp_base
    ldr r8, [r0]
    ldr r0, = pc_samp_end
    ldr r9, [r0]
    /* init tmr clear */
    ldr r0, = tmr_clr_reg
    ldr r10, [r0]


    /* init wt ptr to base */
    mov r11, r8

    /* enable fiq */
    CPSIE   f
#else
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
#endif
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

/* 4 Clear bss */
    ldr a1, .LC1        /* First arg: start of memory block */
    ldr a3, .LC2
    sub a3, a3, a1      /* Third arg: length of block */
    mov a2, #0
    bl  memset

/* Copy iram code sections into IRAM. */
    ldr r0, =__iram_code_load_start__
    ldr r1, =__iram_code_start
    ldr r2, =__iram_code_end
    bl crt0_memory_copy

/* Copy data sections into RAM. */
    ldr r0, =__data_load_start__
    ldr r1, =__data_start
    ldr r2, =__data_end
    bl crt0_memory_copy

/* Copy iram data sections into IRAM . */
    ldr r0, =__iram_data_load_start__
    ldr r1, =__iram_data_start
    ldr r2, =__iram_data_end
    bl crt0_memory_copy

    bl  main

crt0_memory_copy:
    cmp r0, r1
    beq memory_copy_done
    sub r2, r2, r1
    cmp r2, #0
    beq memory_copy_done
memory_copy_loop:
    ldrb r3, [r0]
    add r0, r0, #1
    strb r3, [r1]
    add r1, r1, #1
    sub r2, r2, #1
    cmp r2, #0
    bne memory_copy_loop
memory_copy_done:
    bx lr

    .align 0
.LC1:
    .word   __bss_start
.LC2:
    .word   __bss_end
