# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/audio_system.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_service.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_policy.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_route.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_sink_music.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_sink_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_sink.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_record.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_record_music.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_sink_record.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_client.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_client_misc.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_eng.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_dump.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_diag.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_md.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_resample.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_effect.c
    ${CMAKE_CURRENT_LIST_DIR}/tone_check.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_nrec.c
)

aic_library_sources_ifdef(
    CONFIG_AUDIO_SUPPORT
    ${SRCS}
)

aic_library_sources_ifdef(
    CONFIG_AUDIO_AT_COMM
    ${CMAKE_CURRENT_LIST_DIR}/audio_at_common.c
)

aic_library_sources_ifdef(
    CONFIG_AUDIO_AT_CMIOT
    ${CMAKE_CURRENT_LIST_DIR}/audio_at_cmiot.c
)

aic_library_sources_ifdef(
    CONFIG_TONE_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/audio_sink_tone.c
)

aic_library_sources_ifdef(
    CONFIG_NV_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/audio_nv_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_nv.c
)

aic_library_sources_ifdef(
    CONFIG_AUDIO_TTS
    ${CMAKE_CURRENT_LIST_DIR}/audio_sink_tts.c
    ${CMAKE_CURRENT_LIST_DIR}/vendor/tts_yt/ytwb_reg_place_data.c
)

aic_library_sources_ifdef(
    CONFIG_VOICE_ACC
    ${CMAKE_CURRENT_LIST_DIR}/voice_mng_acc.c
)

aic_library_sources_ifndef(
    CONFIG_VOICE_ACC
    ${CMAKE_CURRENT_LIST_DIR}/voice_mng.c
)

aic_library_sources_ifdef(
    CONFIG_VOICE_EFFECT
    ${CMAKE_CURRENT_LIST_DIR}/voice_proc.c
    ${CMAKE_CURRENT_LIST_DIR}/voice_codec.c
)

aic_library_sources_ifdef(
    CONFIG_MCU_AUDIO_VOICE_CLIENT
    ${CMAKE_CURRENT_LIST_DIR}/audio_client_voice.c
)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/mm/audio/resource
    ${AIC_SOURCE_ROOT}/mm/codec
    ${AIC_SOURCE_ROOT}/mm/codec/amr
    ${AIC_SOURCE_ROOT}/mm/audio/nv
    ${AIC_SOURCE_ROOT}/drivers/sound/adsp
    ${AIC_SOURCE_ROOT}/mm/audio/vendor/tts_yt
    ${AIC_SOURCE_ROOT}/mm/audio/vendor/voiceproc
    ${AIC_SOURCE_ROOT}/mm/audio/vendor/resampler
)

aic_library_import(yt_tts_cn_man_guixin_net
    ${CMAKE_CURRENT_LIST_DIR}/vendor/tts_yt/libyt_tts_cn_man_guixin_net.a
)


aic_library_import(voiceproc
    ${CMAKE_CURRENT_LIST_DIR}/vendor/voiceproc/${CONFIG_AUDIO_LIB_TYPE}/libvoiceproc.a
)

aic_include_directories(
    ${AIC_SOURCE_ROOT}/mm/include
)

add_subdirectory(vendor/resampler/sinc_resampler)
