# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_include_directories(${CMAKE_CURRENT_LIST_DIR})

#aic_library_include_directories(
#        ${CMAKE_CURRENT_LIST_DIR}
#)

aic_library_sources_ifdef(CONFIG_LIBJPEG_SUPPORT
        ${CMAKE_CURRENT_LIST_DIR}/jcapimin.c
        ${CMAKE_CURRENT_LIST_DIR}/jcapistd.c
        ${CMAKE_CURRENT_LIST_DIR}/jdapimin.c
        ${CMAKE_CURRENT_LIST_DIR}/jdapistd.c
        ${CMAKE_CURRENT_LIST_DIR}/jcomapi.c
        ${CMAKE_CURRENT_LIST_DIR}/jcparam.c
        ${CMAKE_CURRENT_LIST_DIR}/jctrans.c
        ${CMAKE_CURRENT_LIST_DIR}/jdtrans.c

        ${CMAKE_CURRENT_LIST_DIR}/jmemnobs_aic.c

        ${CMAKE_CURRENT_LIST_DIR}/jerror_aic.c
        ${CMAKE_CURRENT_LIST_DIR}/jaricom.c
        ${CMAKE_CURRENT_LIST_DIR}/jmemmgr.c
        ${CMAKE_CURRENT_LIST_DIR}/jutils.c
)

aic_library_sources_ifdef(CONFIG_LIBJPEG_DECODER_SUPPORT
        ${CMAKE_CURRENT_LIST_DIR}/jdarith.c
        ${CMAKE_CURRENT_LIST_DIR}/jdmaster.c
        ${CMAKE_CURRENT_LIST_DIR}/jdinput.c
        ${CMAKE_CURRENT_LIST_DIR}/jdmainct.c
        ${CMAKE_CURRENT_LIST_DIR}/jdcoefct.c
        ${CMAKE_CURRENT_LIST_DIR}/jdpostct.c
        ${CMAKE_CURRENT_LIST_DIR}/jdmarker.c
        ${CMAKE_CURRENT_LIST_DIR}/jdhuff.c
        ${CMAKE_CURRENT_LIST_DIR}/jddctmgr.c
        ${CMAKE_CURRENT_LIST_DIR}/jidctint.c
        ${CMAKE_CURRENT_LIST_DIR}/jidctfst.c
        ${CMAKE_CURRENT_LIST_DIR}/jidctflt.c
        ${CMAKE_CURRENT_LIST_DIR}/jdsample.c
        ${CMAKE_CURRENT_LIST_DIR}/jdcolor_aic.c
        ${CMAKE_CURRENT_LIST_DIR}/jdmerge.c
        ${CMAKE_CURRENT_LIST_DIR}/jquant1.c
        ${CMAKE_CURRENT_LIST_DIR}/jquant2.c
        ${CMAKE_CURRENT_LIST_DIR}/jdatasrc.c
)

aic_library_sources_ifdef(CONFIG_LIBJPEG_ENCODER_SUPPORT
        ${CMAKE_CURRENT_LIST_DIR}/jcinit.c
        ${CMAKE_CURRENT_LIST_DIR}/jcmaster.c
        ${CMAKE_CURRENT_LIST_DIR}/jcmainct.c
        ${CMAKE_CURRENT_LIST_DIR}/jcprepct.c
        ${CMAKE_CURRENT_LIST_DIR}/jccoefct.c
        ${CMAKE_CURRENT_LIST_DIR}/jccolor.c
        ${CMAKE_CURRENT_LIST_DIR}/jcsample.c
        ${CMAKE_CURRENT_LIST_DIR}/jcdctmgr.c
        ${CMAKE_CURRENT_LIST_DIR}/jfdctint.c
        ${CMAKE_CURRENT_LIST_DIR}/jfdctfst.c
        ${CMAKE_CURRENT_LIST_DIR}/jfdctflt.c
        ${CMAKE_CURRENT_LIST_DIR}/jchuff.c
        ${CMAKE_CURRENT_LIST_DIR}/jcmarker.c
        ${CMAKE_CURRENT_LIST_DIR}/jdatadst.c
        ${CMAKE_CURRENT_LIST_DIR}/jcarith.c
)

aic_library_sources_ifdef(CONFIG_LIBJPEG_AIC_EXAMPLE_SUPPORT
        ${CMAKE_CURRENT_LIST_DIR}/aic_jpeglib_example.c
)

