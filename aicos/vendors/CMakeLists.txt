# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_JC_NETWORK_VIDEO_CALL jc_network_video_call)
add_subdirectory_ifdef(CONFIG_SSV_WIFI ssv_wifi)
add_subdirectory_ifdef(CONFIG_FOTA_SUPPORT_ABUP abup)
add_subdirectory_ifdef(CONFIG_TTS_IFLY tts_ifly)
add_subdirectory_ifdef(CONFIG_BK7238 bk7238)