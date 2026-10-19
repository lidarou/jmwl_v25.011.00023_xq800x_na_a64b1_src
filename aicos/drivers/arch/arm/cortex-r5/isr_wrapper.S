/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the isr functions with vic(pl192)
 */

#ifdef CONFIG_RTOS_THREADX
.extern _tx_thread_context_save
#elif defined(CONFIG_RTOS_LITEOS_M)
.extern OsIrqReturn
.extern isr_wrapper
#endif

.arm
.text
.section .isrfunc, "ax"

/*******************************************************************
  macro for isr func,call isr_wrapper with int number
 *******************************************************************/
.macro isr_func irq_n
#ifdef CONFIG_RTOS_THREADX
  STMDB   sp!, {r0-r4}
  /* Save irq number to r4 */
  mov r4, #\irq_n
  /* Jump to context save to save system context.  */
  b _tx_thread_context_save
#elif defined(CONFIG_RTOS_LITEOS_M)
  /* Adjust point of interrupt */
  SUB     lr, lr, #4

  PUSH    {lr}
  MRS     lr, SPSR
  PUSH    {lr}

  PUSH    {r0-r4, r12}

  MOV     r0, #\irq_n
  BLX     isr_wrapper

  B       OsIrqReturn
#endif
.endm

.align 4
.global isr_func0
isr_func0:
  isr_func 0

.global isr_func1
isr_func1:
  isr_func 1

.global isr_func2
isr_func2:
  isr_func 2

.global isr_func3
isr_func3:
  isr_func 3

.global isr_func4
isr_func4:
  isr_func 4

.global isr_func5
isr_func5:
  isr_func 5

.global isr_func6
isr_func6:
  isr_func 6

.global isr_func7
isr_func7:
  isr_func 7

.global isr_func8
isr_func8:
  isr_func 8

.global isr_func9
isr_func9:
  isr_func 9

.global isr_func10
isr_func10:
  isr_func 10

.global isr_func11
isr_func11:
  isr_func 11

.global isr_func12
isr_func12:
  isr_func 12

.global isr_func13
isr_func13:
  isr_func 13

.global isr_func14
isr_func14:
  isr_func 14

.global isr_func15
isr_func15:
  isr_func 15

.global isr_func16
isr_func16:
  isr_func 16

.global isr_func17
isr_func17:
  isr_func 17

.global isr_func18
isr_func18:
  isr_func 18

.global isr_func19
isr_func19:
  isr_func 19

.global isr_func20
isr_func20:
  isr_func 20

.global isr_func21
isr_func21:
  isr_func 21

.global isr_func22
isr_func22:
  isr_func 22

.global isr_func23
isr_func23:
  isr_func 23

.global isr_func24
isr_func24:
  isr_func 24

.global isr_func25
isr_func25:
  isr_func 25

.global isr_func26
isr_func26:
  isr_func 26

.global isr_func27
isr_func27:
  isr_func 27

.global isr_func28
isr_func28:
  isr_func 28

.global isr_func29
isr_func29:
  isr_func 29

.global isr_func30
isr_func30:
  isr_func 30

.global isr_func31
isr_func31:
  isr_func 31
