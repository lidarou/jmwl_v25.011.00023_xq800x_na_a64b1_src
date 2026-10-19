# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

file(GLOB_RECURSE SOURCES ./em.c)
aic_library_sources_ifdef(CONFIG_APP_EM ${SOURCES})

file(GLOB_RECURSE SOURCES ./ui/*.c)
aic_library_sources_ifdef(CONFIG_APP_EM ${SOURCES})

file(GLOB_RECURSE SOURCES ./flow/*.c)
aic_library_sources_ifdef(CONFIG_APP_EM ${SOURCES})

aic_library_include_directories(./)
aic_library_include_directories(./ui/)
aic_library_include_directories(./flow/)