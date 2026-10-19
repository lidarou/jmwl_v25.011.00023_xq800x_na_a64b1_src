 .global _start
  .extern main
  .extern aic_prepare_init
  .extern mpu_config_before_load_image

  .extern g_sys_stack_limit;
  .extern g_init_heap_start;
  .extern g_init_heap_limit;

  .section .init, "ax"
  .code 16
  .align 2
  .thumb_func

  .word __stack_end__
  .word reset_handler
_start:
  ldr r3, = g_sys_stack_limit
  ldr r1, [r3]
  mov sp, r1
  ldr r0, =mpu_config_before_load_image
  blx r0
  cpsid   i

  /* add aic prepare init call before copy sections. */
  ldr     r0, =aic_prepare_init
  blx     r0

  /* Copy .vector sections into IRAM if required. */
  ldr r0, =__vec_load_start__
  ldr r1, =__vectors_start__
  ldr r2, =__vectors_end__
  bl crt0_memory_copy

  /* Copy .idle sections into IRAM if required. */
  ldr r0, =__idle_load_start__
  ldr r1, =__idle_start__
  ldr r2, =__idle_end__
  bl crt0_memory_copy

  /* Copy .psram_code sections into MTB if required. */
  ldr r0, =__mtb_load_start__
  ldr r1, =__mtb_start__
  ldr r2, =__mtb_end__
  bl crt0_memory_copy

#if CONFIG_FLASH_XIP
  /* Copy psram text sections into PSRAM if required. */
  ldr r0, =__psram_text_load_start__
  ldr r1, =__psram_text_start__
  ldr r2, =__psram_text_end__
  bl crt0_memory_copy

  /* Copy psram ro sections into PSRAM if required. */
  ldr r0, =__psram_rodata_load_start__
  ldr r1, =__psram_rodata_start__
  ldr r2, =__psram_rodata_end__
  bl crt0_memory_copy
#endif

  /* Copy initialised sections into RAM if required. */
  ldr r0, =__data_load_start__
  ldr r1, =__data_start__
  ldr r2, =__data_end__
  bl crt0_memory_copy

  /* Zero bss. */
  ldr r0, =__bss_start__
  ldr r1, =__bss_end__
  mov r2, #0
  bl crt0_memory_set

  /* Setup heap - not recommended for Threadx but here for compatibility reasons */
  ldr r3, = g_init_heap_start
  ldr r4, = g_init_heap_limit
  ldr r0, [r3]
  ldr r1, [r4]
  sub r1, r1, r0
  mov r2, #0
  str r2, [r0]
  add r0, r0, #4
  str r1, [r0]

  /* Setup call frame for main() */
  mov r0, #0
  mov lr, r0
  mov r12, sp

start:
  /* Jump to main() */
  mov r0, #0
  mov r1, #0
  ldr r2, =main
  blx r2
  /*  when main returns, loop forever. */
crt0_exit_loop:
  b crt0_exit_loop

  /* Startup helper functions. */

crt0_memory_copy:
  cmp r0, r1
  beq memory_copy_done
  sub r2, r2, r1
  beq memory_copy_done
memory_copy_loop:
  ldrb r3, [r0]
  add r0, r0, #1
  strb r3, [r1]
  add r1, r1, #1
  sub r2, r2, #1
  bne memory_copy_loop
memory_copy_done:
  bx lr

crt0_memory_set:
  cmp r0, r1
  beq memory_set_done
  strb r2, [r0]
  add r0, r0, #1
  b crt0_memory_set
memory_set_done:
  bx lr

  /* Setup attibutes of stack and heap sections so they don't take up room in the elf file */
  .section .stack, "wa", %nobits
  .section .stack_process, "wa", %nobits
  .section .heap, "wa", %nobits
