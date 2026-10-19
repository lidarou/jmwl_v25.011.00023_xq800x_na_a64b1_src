# General configuration options
#
# Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

file(GLOB_RECURSE SOURCES ./*.c)

aic_library_sources_ifdef(CONFIG_SUBSYS_AP ${SOURCES} )