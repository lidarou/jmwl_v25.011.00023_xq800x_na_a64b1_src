
set(SRCS
    ./src/aic_assert.c
    ./src/aic_no_semihost_rt.c
    ./src/aic_no_heap_rt.c
    ./src/aic_checksum.c
    ./src/aic_convert.c
    ./src/aic_bitmap.c
    ./src/aic_seg_alloc.c
    )

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/os/osa/common/inc
    ${AIC_SOURCE_ROOT}/os/osa/include
    ${AIC_CFG_PATH}/config
)

aic_include_directories_ifdef(CONFIG_FSMMSG_FEATURE
    ${AIC_SOURCE_ROOT}/comm/ps/code/inc
)

set(AOS_SRCS
    ./src/aos_mem.c
    ./src/aos_bitop.c
    ./src/aos_bitstream.c
    ./src/aos_dma.c
    ./src/string_ex.c
    ./src/mem_malloc.c
    ./src/cbprintf/cbprintf_nano.c
    ./src/cbprintf/sprintf_nano.c
    )

set(AOS_TX_SRCS
    ./src/threadx/aos_event.c
    ./src/threadx/aos_msg.c
    ./src/threadx/aos_mutex.c
    ./src/threadx/aos_semaphore.c
    ./src/threadx/aos_thread.c
    ./src/threadx/aos_time.c
    ./src/threadx/aos_timer.c
    ./src/threadx/aos_interrupt.c
    ./src/threadx/aos_queue.c
    ./src/threadx/aos_utl.c
    )

set(AOS_LOS_SRCS
    ./src/liteos/aos_event.c
    ./src/liteos/aos_msg.c
    ./src/liteos/aos_mutex.c
    ./src/liteos/aos_semaphore.c
    ./src/liteos/aos_thread.c
    ./src/liteos/aos_time.c
    ./src/liteos/aos_timer.c
    ./src/liteos/aos_interrupt.c
    ./src/liteos/aos_queue.c
    ./src/liteos/aos_utl.c
    )

aic_library_sources_ifdef(
    CONFIG_OSA_RMTE_MSG
    ./src/aos_remote_msg.c
    )

aic_library_sources_ifdef(
    CONFIG_SUBSYS_CP
    ./src/aos_list.c
    )

aic_library_sources_ifdef(
    CONFIG_MEM_MODE_TLSF
    ./src/aos_mem_tlsf.c
    )

aic_library_sources_ifdef(
    CONFIG_OS_COMPONENTS_CMD
    ./src/${CONFIG_RTOS_OS_NAME}/aic_list_os_components.c
    )

aic_library_sources_ifdef(
    CONFIG_AIC_OSA
    ${AOS_SRCS}
    )

aic_library_sources_ifdef(
    CONFIG_RTOS_THREADX
    ${AOS_TX_SRCS}
    )

aic_library_sources_ifdef(
    CONFIG_RTOS_LITEOS_M
    ${AOS_LOS_SRCS}
    )

aic_library_sources_ifdef(
    CONFIG_AIC_OSA_TEST
    ./src/aos_test.c
    )
