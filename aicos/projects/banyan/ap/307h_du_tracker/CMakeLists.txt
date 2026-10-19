cmake_minimum_required(VERSION 3.13)

aic_append_toolchain_library_yes_no(
   CONFIG_LINKER_USE_LIBC
   c
   c_nano)
aic_append_toolchain_library(m)
aic_append_toolchain_library(nosys)

#
# Create Library and add Curent Library to project library list
#

# Create Library
aic_library_named(board)

aic_compile_definitions(CMAKE_CONFIG_BOARD="${CMAKE_CONFIG_BOARD}")

#
#############################################################
#   Create static Library and add source code
# Create Library and add Curent Library to project library list
#############################################################
set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/config/sys_mem_config.c
    ${CMAKE_CURRENT_LIST_DIR}/config/aic_version.c
    ${CMAKE_CURRENT_LIST_DIR}/config/tele_cfg.c
)

set(AOS_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/config/aos_config.c
)

aic_cc_option(-fno-ident)
aic_cc_option(-fmerge-all-constants)
aic_cc_option(-fno-isolate-erroneous-paths-dereference)
aic_cc_option(-freorder-blocks-algorithm=stc)

#
#############################################################
# Support common header file overlap
# The header files priority: [board]/config > common_inc
# The [board]/config header files will overlap common_inc files.
#############################################################
#
file(GLOB allheaderfiles ${AIC_SUBSYS_PATH}/common_inc/*)
file(COPY ${allheaderfiles} DESTINATION ${AIC_CHIP_DIR})
execute_process(
    COMMAND python ${AIC_ORIGIN_ROOT}/build/filecopy.py ${AIC_CFG_PATH}/config/ ${AIC_CHIP_DIR}
)

#
#############################################################
# Public curent header file to total project
# Add the directory to INTERFACE LIBRARY
#############################################################
#
aic_library_include_directories(
    ${AIC_CFG_PATH}/config
    ${AIC_SOURCE_ROOT}/os/osa/common/inc
    ${AIC_SOURCE_ROOT}/os/osa/include
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(
    CONFIG_AIC_OSA
    ${AOS_SRCS}
)

set(PRE_FLAG
    -I${AIC_CHIP_DIR}
    -I${AIC_CHIP_COM_DIR}
    -I${AIC_OUT_DIR}/config
    -I${AIC_HWI_DIR}
    -I${AIC_TARGET_DIR}
    -I${AIC_CFG_PATH}/config
    -imacros ${AUTOCONF_H}
)

file(TOUCH_NOCREATE ${AIC_CFG_PATH}/config/aic_version.c)
file(REMOVE ${AIC_BUILD_ROOT}/aic_board.tmp ${AIC_BUILD_ROOT}/aic_board.ld)
file(COPY ${AIC_CFG_PATH}/aic_board.ld DESTINATION ${AIC_BUILD_ROOT})
file(RENAME ${AIC_BUILD_ROOT}/aic_board.ld ${AIC_BUILD_ROOT}/aic_board.c)

execute_process (
    COMMAND ${CMAKE_C_COMPILER} -E -c ${PRE_FLAG} ${AIC_BUILD_ROOT}/aic_board.c -o ${AIC_BUILD_ROOT}/aic_board.tmp
)

execute_process (
    COMMAND python ${CMAKE_SCRIPT_PATH}/ldfile_process.py ${AIC_BUILD_ROOT}/aic_board.tmp ${AIC_BUILD_ROOT}/aic_board.ld
)
