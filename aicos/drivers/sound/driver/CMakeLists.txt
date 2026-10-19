# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/sound/driver
    ${AIC_SOURCE_ROOT}/drivers/sound/ipc
    ${AIC_SOURCE_ROOT}/drivers/sound/adsp
)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/snd_card.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_compat.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_ctl.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_dpm.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_dai.c
    ${CMAKE_CURRENT_LIST_DIR}/snd_ext_pa.c
    ${CMAKE_CURRENT_LIST_DIR}/stream/snd_stream_pcm.c
)

aic_library_sources_ifdef(CONFIG_SOUND_SUPPORT
    ${SRCS}
)

aic_library_sources_ifdef(CONFIG_SOUND_AIC_CODEC
    ${CMAKE_CURRENT_LIST_DIR}/codec/aic_codec.c
)

aic_library_sources_ifdef(CONFIG_SOUND_CODEC_ES8374
    ${CMAKE_CURRENT_LIST_DIR}/codec/es8374.c
)

aic_library_sources_ifdef(CONFIG_SOUND_CODEC_ES8311
    ${CMAKE_CURRENT_LIST_DIR}/codec/es8311.c
)

aic_library_sources_ifdef(CONFIG_SOUND_CODEC_ES7243L
    ${CMAKE_CURRENT_LIST_DIR}/codec/es7243l.c
)

aic_library_sources_ifdef(CONFIG_SOUND_CODEC_CJC8910
    ${CMAKE_CURRENT_LIST_DIR}/codec/cjc8910.c
)

aic_library_sources_ifdef(CONFIG_SOUND_CODEC_CJC8911
    ${CMAKE_CURRENT_LIST_DIR}/codec/CJC8911_Reg.c
)

aic_library_sources_ifdef(CONFIG_SOUND_CODEC_NO_CTRL
    ${CMAKE_CURRENT_LIST_DIR}/codec/no_ctrl_codec.c
)

aic_library_sources_ifdef(CONFIG_SOUND_STREAM_IPC
    ${CMAKE_CURRENT_LIST_DIR}/stream/snd_stream_compr.c
)

if (CONFIG_SOUND_STREAM_IPC OR CONFIG_VOICE_EFFECT)
aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/stream/snd_stream_vorec.c
)
endif()

aic_library_sources_ifdef(CONFIG_SOUND_PA_AW87390
    ${CMAKE_CURRENT_LIST_DIR}/codec/aw87390.c
)

aic_library_sources_ifdef(CONFIG_SOUND_PA_AW88261
    ${CMAKE_CURRENT_LIST_DIR}/codec/aw88261.c
)

aic_library_sources_ifdef(CONFIG_SOUND_PA_HAA9210
    ${CMAKE_CURRENT_LIST_DIR}/codec/haa9210.c
)

aic_library_sources_ifdef(CONFIG_SOUND_GPIO_CTRL_PA
    ${CMAKE_CURRENT_LIST_DIR}/codec/snd_gpio_ctrl_pa.c
)

aic_library_sources_ifdef(CONFIG_SND_STREAM_CMUX
    ${CMAKE_CURRENT_LIST_DIR}/stream/snd_stream_cmux_voice.c
)

if (CONFIG_SND_STREAM_CMUX OR CONFIG_SND_STREAM_VOICE_MIX)
aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/stream/snd_stream_ext.c
)
endif()
