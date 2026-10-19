# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

if(CONFIG_RESOLUTION_128x128)
    file(GLOB SOURCES ./comp_msgbox_128x128.c)
    aic_library_sources_ifdef(CONFIG_COMP_MSGBOX ${SOURCES})
else()
    file(GLOB SOURCES ./comp_msgbox.c)
    aic_library_sources_ifdef(CONFIG_COMP_MSGBOX ${SOURCES})
endif()


