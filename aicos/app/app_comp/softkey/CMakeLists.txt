# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_COMP_SOFTKEY ${SOURCES})

if(CONFIG_RESOLUTION_128x128)

else()
    file(GLOB_RECURSE SOURCES ./comp_softkey/*.c)
    aic_library_sources_ifdef(CONFIG_COMP_SOFTKEY ${SOURCES})
endif()
