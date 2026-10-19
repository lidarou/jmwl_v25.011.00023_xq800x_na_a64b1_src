cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

#############################################################
# Public curent header file to total project
# Add the directory to INTERFACE LIBRARY
#############################################################
#

aic_library_sources_ifdef(
            CONFIG_ICONV_GB2312
            ${CMAKE_CURRENT_LIST_DIR}/src/aic_gb2312.c
)

aic_library_sources_ifdef(
            CONFIG_ICONV_GBK
            ${CMAKE_CURRENT_LIST_DIR}/src/aic_gbk.c
)

aic_library_sources_ifdef(
            CONFIG_ICONV_UTF8
            ${CMAKE_CURRENT_LIST_DIR}/src/aic_utf8.c
)

aic_include_directories(
            ${CMAKE_CURRENT_LIST_DIR}/inc
)
