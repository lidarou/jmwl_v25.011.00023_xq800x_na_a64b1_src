# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

file(GLOB SOURCES ./*.c)
aic_library_sources_ifdef(CONFIG_APP_SMS_FP ${SOURCES})


if(CONFIG_RESOLUTION_240x320)
    file(GLOB_RECURSE  SOURCES ./app_sms_240x320/*.c)
    aic_library_sources_ifdef(CONFIG_APP_SMS_FP ${SOURCES})
endif()
