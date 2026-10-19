/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the l1 icache and dcache operations for cortex-m series
 *    Include cortex-m7,cortex-m star.
 */

.global icache_enable
.global icache_disable
.global icache_inv_by_range
.global icache_inv_all
.global dcache_enable
.global dcache_disable
.global dcache_clean_all
.global dcache_invcc_all
.global dcache_inv_by_range
.global dcache_clean_by_range
.global dcache_invcc_by_range

.global l1icache_enable
.global l1icache_disable
.global l1icache_inv_by_range
.global l1icache_inv_all
.global l1dcache_enable
.global l1dcache_disable
.global l1dcache_inv_all
.global l1dcache_clean_all
.global l1dcache_invcc_all
.global l1dcache_inv_by_range
.global l1dcache_clean_by_range
.global l1dcache_invcc_by_range

.EQU CCSIDR, 0XE000ED80
.EQU CSSELR, 0XE000ED84
.EQU CCR, 0XE000ED14
.EQU ICIALLU, 0XE000EF50
.EQU ICIMVAU, 0XE000EF58
.EQU DCISW, 0XE000EF60
.EQU DCCSW, 0XE000EF6C
.EQU DCCISW, 0XE000EF74
.EQU DCIMVAC, 0XE000EF5C
.EQU DCCMVAC, 0xE000EF68
.EQU DCCIMVAC, 0xE000EF70
.EQU CCR_DC_EN, 0X10000
.EQU CCR_IC_EN, 0x20000

.text 16
.align 2
.syntax unified

/*******************************************************************
  macro for invalidate,clean,invalidate and clean dcache by set/way
  r0 store start address which not required for cacheline aligned
  r1 store size in byte to be operated
 *******************************************************************/
.macro op_cache_by_range op_reg loop
  push {r11,lr}
  bic r2, r0, #0x1F // r2 is start address, cache line aligned
  and r0, #0x1F // start addr & 0x1f = to be operated additional
  add r1, r0 // total bytes to be operated actually
  add r1, #0x1F // round up
  lsr r1, #0x5 // r1 is loop counter in cache line size
  ldr r11, =\op_reg
\loop :
  str r2, [r11]
  add r2, #0x20
  subs r1, #0x1
  bgt \loop // >
  dsb
  isb
  pop {r11,lr}
  bx lr
.endm

.macro op_cache_by_sw op_reg loop1 loop2
  push {r4,r6,r7,r8,r11,lr}
  mov r0, #0x0
  ldr r11, =CSSELR
  str r0, [r11]
  dsb
  ldr r11, =CCSIDR
  ldr r2, [r11]
  and r1, r2, #0x7
  add r7, r1, #0x4
  ubfx r4, r2, #3, #10 // Number of way
  ubfx r2, r2, #13, #15  // Number of set
  clz r6, r4
  ldr r11, =\op_reg
\loop1 :
  mov r1, r4
  lsls r8, r2, r7
\loop2 :
  lsls r3, r1, r6
  orrs r3, r3, r8
  str r3, [r11]
  subs r1, r1, #0x1
  bge \loop2
  subs r2, r2, #0x1
  bge \loop1
  dsb
  isb
  pop {r4,r6,r7,r8,r11,lr}
  bx lr
.endm

.thumb_func
l1dcache_inv_all:
  op_cache_by_sw DCISW dinv1 dinv2
.thumb_func
l1dcache_clean_all:
  op_cache_by_sw DCCSW dclean1 dclean2
.thumb_func
l1dcache_invcc_all:
  op_cache_by_sw DCCISW dinvcc1 dinvcc2
.thumb_func
l1dcache_inv_by_range:
  op_cache_by_range DCIMVAC dinv
.thumb_func
l1dcache_clean_by_range:
  op_cache_by_range DCCMVAC dclean
.thumb_func
l1dcache_invcc_by_range:
  op_cache_by_range DCCIMVAC dinvcc
.thumb_func
l1icache_inv_by_range:
  op_cache_by_range ICIMVAU iinv

.thumb_func
l1icache_enable:
  push {r11, lr}
  ldr r0, = l1icache_inv_all
  blx r0
  ldr r11, = CCR
  ldr r0, [r11]
  orr r0, r0, CCR_IC_EN
  str r0, [r11]
  dsb
  isb
  pop {r11, lr}
  bx lr

.thumb_func
l1icache_disable:
  push {r11, lr}
  ldr r11, = CCR
  ldr r0, [r11]
  bic r0, r0, CCR_IC_EN
  str r0, [r11]
  dsb
  isb
  pop {r11, lr}
  bx lr

.thumb_func
l1icache_inv_all:
  push {r11, lr}
  mov r0, #0x0
  ldr r11, =ICIALLU
  str r0, [r11]
  dsb
  isb
  pop {r11, lr}
  bx lr

.thumb_func
l1dcache_enable:
  push {r11, lr}
  ldr r0, =l1dcache_inv_all
  blx r0
  ldr r11, =CCR
  ldr r0, [r11]
  orr r0, r0, CCR_DC_EN
  str r0, [r11]
  dsb
  isb
  pop {r11, lr}
  bx lr

.thumb_func
l1dcache_disable:
  push {r11, lr}
  ldr r0, =l1dcache_clean_all
  blx r0
  ldr r0, [r11]
  bic r0, r0, CCR_DC_EN
  str r0, [r11]
  dsb
  isb
  pop {r11, lr}
  bx lr

.thumb_func
dcache_clean_all:
  b l1dcache_clean_all
.thumb_func
dcache_invcc_all:
  b l1dcache_invcc_all
.thumb_func
dcache_inv_by_range:
  b l1dcache_inv_by_range
.thumb_func
dcache_clean_by_range:
  b l1dcache_clean_by_range
.thumb_func
dcache_invcc_by_range:
  b l1dcache_invcc_by_range
.thumb_func
icache_inv_by_range:
  b l1icache_inv_by_range
.thumb_func
icache_enable:
  b l1icache_enable
.thumb_func
icache_disable:
  b l1icache_disable
.thumb_func
icache_inv_all:
  b l1icache_inv_all
.thumb_func
dcache_enable:
  b l1dcache_enable
.thumb_func
dcache_disable:
  b l1dcache_disable
