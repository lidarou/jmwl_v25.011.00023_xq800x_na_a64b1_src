 .global _start
  .extern main
  .extern memory_init

  .section .init, "ax"
  .code 16
  .align 2
  .thumb_func


_start:
  cpsid   i
  ldr r1, =__stack_end__
  mov sp, r1

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
