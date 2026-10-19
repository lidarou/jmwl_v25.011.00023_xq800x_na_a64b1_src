# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(bc_main.c)

add_subdirectory_ifdef(CONFIG_BCMARK_DHRYSTONE dhrystone)
