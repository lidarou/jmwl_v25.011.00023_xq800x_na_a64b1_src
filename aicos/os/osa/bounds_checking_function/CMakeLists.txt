# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_named(bounds_chk)

aic_library_compile_definitions(
    SECUREC_ENABLE_WCHAR_FUNC=0
    SECUREC_SNPRINTF_TRUNCATED=0
)

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/memcpy_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/memmove_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/memset_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/securecutil.c
    ${CMAKE_CURRENT_LIST_DIR}/src/strcat_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/strcpy_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/strncat_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/strncpy_s.c
    ${CMAKE_CURRENT_LIST_DIR}/src/strtok_s.c
)

aic_library_sources(${SOURCES})

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
)
