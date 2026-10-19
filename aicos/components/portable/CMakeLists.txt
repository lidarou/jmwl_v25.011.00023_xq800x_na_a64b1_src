# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

file(GLOB_RECURSE SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_SUBSYS_AP ${SOURCES} )

file(GLOB SOURCES ./time/*.c)
aic_library_sources_ifdef(CONFIG_SUBSYS_AP    ${SOURCES})