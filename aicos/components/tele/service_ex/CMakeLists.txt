# General configuration options
#
# Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.


aic_include_directories(${CMAKE_CURRENT_LIST_DIR}/)

file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_TELE_SERVICE_EX ${SOURCES})