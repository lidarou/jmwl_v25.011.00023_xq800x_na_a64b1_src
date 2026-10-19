# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_tele
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
#
aic_library_sources(${CMAKE_CURRENT_LIST_DIR}/aic_tele.c)
aic_library_sources(${CMAKE_CURRENT_LIST_DIR}/aic_tele_init.c)

aic_sdk_library(${PROJECT_NAME})

if(NOT CONFIG_SDK_ENABLE)
aic_include_directories_ifdef(CONFIG_SUBSYS_AP ${AIC_SOURCE_ROOT}/components/include/internal)

aic_library_sources_ifdef(CONFIG_BYPASS_MD_AT  ${CMAKE_CURRENT_LIST_DIR}/bypass_md_at/bypass_md_at.c)
aic_library_include_directories(${CMAKE_CURRENT_LIST_DIR}/porting)

aic_include_directories(${AIC_SOURCE_ROOT}/projects/genesis/ap/common_inc)

add_subdirectory_ifdef(CONFIG_MODEM_DRIVER modem_driver)
add_subdirectory(service)
add_subdirectory(porting)
add_subdirectory(voice)
endif()
