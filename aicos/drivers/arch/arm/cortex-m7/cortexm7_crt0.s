 .global _start
  .extern main
  .extern aic_prepare_init
  .extern icache_enable
  .extern dcache_enable
  .extern mpu_config

  .section .init, "ax"
  .code 16
  .align 2
  .thumb_func


_start:
  ldr r0, =icache_enable
  blx r0
  ldr r0, =dcache_enable
  blx r0
  ldr r0, =mpu_config
  blx r0
  CPSID   i
  ldr r1, =__stack_end__
  mov sp, r1

  /* add aic prepare init call before copy sections. */
  ldr     r0, =aic_prepare_init
  blx     r0

  /* Copy initialised sections into RAM if required. */
  ldr r0, =__data_load_start__
  ldr r1, =__data_start__
  ldr r2, =__data_end__
  bl crt0_memory_copy
  ldr r0, =__ram_ro_load_start__
  ldr r1, =__ram_ro_start__
  ldr r2, =__ram_ro_end__
  bl crt0_memory_copy

  /* Zero bss. */
  ldr r0, =__bss_start__
  ldr r1, =__bss_end__
  mov r2, #0
  bl crt0_memory_set


  /* Setup heap - not recommended for Threadx but here for compatibility reasons */
  ldr r0, = __heap_start__
  ldr r1, = __heap_end__
  sub r1, r1, r0
  mov r2, #0
  str r2, [r0]
  add r0, r0, #4
  str r1, [r0]


  ldr r0, =dcache_invcc_all
  blx r0

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
  