

  .global reset_handler

  .global __tx_NMIHandler
  .global __tx_BadHandler
  .global __tx_SVCallHandler
  .global __tx_DBGHandler
  .global __tx_PendSVHandler 
  .global __tx_SysTickHandler 
  .global __tx_BadHandler
  .global rt1052_usb_irq_handle

  .syntax unified
  .section .vectors, "ax"
  .code 16
  .align 0
  .global _vectors

_vectors:
  .word __stack_end__ 
  .word reset_handler 
  .word __tx_NMIHandler 
  .word __tx_HardfaultHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word 0 // Reserved
  .word 0 // Reserved
  .word 0 // Reserved
  .word 0 // Reserved
  .word __tx_SVCallHandler //_SVC_Handler - used by Threadx scheduler //
  .word __tx_DBGHandler
  .word 0 // Reserved
  .word __tx_PendSVHandler 
  .word __tx_SysTickHandler   // Used by Threadx timer functionality
  .word __tx_BadHandler       // Populate with user Interrupt handler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler//num 50
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler//num 100
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word rt1052_usb_irq_handle //113
  .word __tx_BadHandler //114
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler
  .word __tx_BadHandler//num 120
  .word __tx_BadHandler
#ifdef CONFIG_TIMER_TX_NO_TICK_IRQ
  .word tmr_handler
#endif

    .section .init, "ax"
    .thumb_func
reset_handler:

// low level hardware config, such as PLL setup goes here

  b _start



