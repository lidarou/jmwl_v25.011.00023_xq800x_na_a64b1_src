# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
        ./
        ./flow
)
aic_library_sources_ifdef(CONFIG_APP_CALL ./call.c)

if(CONFIG_RESOLUTION_128x128)

    aic_library_include_directories( ./ui_128x128/ )
    file(GLOB_RECURSE SOURCES ./ui_128x128/*.c)

else()

    aic_library_include_directories( ./ui/ )

    file(GLOB_RECURSE SOURCES ./ui/*.c)
    if(CONFIG_SUPPORT_PHYSICAL_KEY)
        file(GLOB_RECURSE REMOVE_SOURCES ./ui/win_dialer/*.c)
        list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
    else()
        file(GLOB_RECURSE REMOVE_SOURCES ./ui/win_dialer_withkey/*.c)
        list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
    endif()

endif()
aic_library_sources_ifdef(CONFIG_APP_CALL ${SOURCES})
