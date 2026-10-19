# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories( ./ )

file(GLOB SOURCES ./*.c)
aic_library_sources_ifndef(CONFIG_APP_ENTRY_NOGUI  ${SOURCES} )

if(CONFIG_RESOLUTION_128x128)
    aic_library_include_directories( ./res_128x128 )
    file(GLOB SOURCES ./res_128x128/*.c)
else()
    aic_library_include_directories( ./res/ )
    file(GLOB SOURCES ./res/*.c)
endif()

if(SOURCES)
    aic_library_sources_ifndef(CONFIG_APP_ENTRY_NOGUI   ${SOURCES} )
endif()
