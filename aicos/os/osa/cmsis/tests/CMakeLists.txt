aic_library_named(tx_cmsis_tests)
set(SRCS
    ./main.c
    ./source/config/RV2_Config.c
    ./source/RV2_Thread.c
    ./source/RV2_Common.c
    ./source/RV2_EventFlags.c
    ./source/RV2_GenWait.c
    ./source/RV2_Kernel.c
    ./source/RV2_MemoryPool.c
    ./source/RV2_MessageQueue.c
    ./source/RV2_Mutex.c
    ./source/RV2_Semaphore.c
    ./source/RV2_ThreadFlags.c
    ./source/RV2_Timer.c
    ./source/cmsis_rv2.c
    ./source/tf_main.c
    ./source/tf_report.c
    )

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_include_directories(
    ./include
    ./source/config
)
aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/os/osa/cmsis/include
)