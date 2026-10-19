# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(EIP130_DDK_3_1 DDK-130_v3.1)

include_directories()

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/GX310
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/um/
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_GlobalControl/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_GlobalControl/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/GX310
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Interrupts/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Interrupts/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Interrupts/src/GX310
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_VAL/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_VAL/src/arm_threadx
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_VAL/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_VEX/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/DriverFramework/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/arm_threadx
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/DriverFramework/src/arm_threadx
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/UMDevXS/UserPart/incl/
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/DriverFramework/Basic_Defs_API/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/DriverFramework/CLib_Abstraction_API/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/DriverFramework/Device_API/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/DriverFramework/DMAResource_API/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP130/TokenHelper/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP130/TokenHelper/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP130/MailboxControl/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP130/MailboxControl/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP201/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP201/src
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/Log/incl
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/Log/src/printf
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/config/
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/config/GX310/
)

file(GLOB VAL_SRCS ./DDK-130_v3.1/Integration/Adapter_VAL/src/*.c)
file(GLOB VEX_SRCS ./DDK-130_v3.1/Integration/Adapter_VEX/src/*.c)
file(GLOB DRIVER_FRAMEWORK_COMM ./DDK-130_v3.1/Integration/DriverFramework/src/*.c)
file(GLOB DRIVER_FRAMEWORK_ARM DDK-130_v3.1/Integration/DriverFramework/src/arm_threadx/*.c)

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP130/MailboxControl/src/eip130.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP130/TokenHelper/src/eip130_token_common.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Kit/EIP201/src/eip201.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/adapter_driver_init.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/adapter_driver_val_init.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/adapter_global_driver_init.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/adapter_global_init.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/GX310/adapter_global_control_init.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/GX310/adapter_init.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_DriverInit/src/GX310/adapter_init_val.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/adapter_bufmanager.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/adapter_lock_internal.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/arm_threadx/adapter_firmware.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/arm_threadx/adapter_getpid.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/arm_threadx/adapter_lock.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/arm_threadx/adapter_sleep.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Generic/src/arm_threadx/adapter_thread.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_GlobalControl/src/adapter_global_eip130.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_GlobalControl/src/um/adapter_memxs.c
    ${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_Interrupts/src/um/adapter_interrupts.c
    ${VAL_SRCS}
    ${VEX_SRCS}
    ${DRIVER_FRAMEWORK_COMM}
    ${DRIVER_FRAMEWORK_ARM}
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_VAL/src/*.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/Adapter_VEX/src/*.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/DriverFramework/src/*.c
    ##${CMAKE_CURRENT_LIST_DIR}/DDK-130_v3.1/Integration/DriverFramework/src/arm_threadx/*.c
)
