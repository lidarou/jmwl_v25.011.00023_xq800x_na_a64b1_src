# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

add_subdirectory_ifdef(CONFIG_SOUND_SUPPORT        driver)
add_subdirectory_ifdef(CONFIG_SND_IPC              ipc)
add_subdirectory_ifdef(CONFIG_ADSP_MNG             adsp)
add_subdirectory_ifdef(CONFIG_VOICE_TRANS          voice)
add_subdirectory_ifdef(CONFIG_SND_SIMU             simu)
add_subdirectory(hal)