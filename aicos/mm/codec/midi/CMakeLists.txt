# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/codec_midi.c
    ${CMAKE_CURRENT_LIST_DIR}/host_src/eas_report.c
    ${CMAKE_CURRENT_LIST_DIR}/host_src/eas_config.c
    ${CMAKE_CURRENT_LIST_DIR}/host_src/eas_hostmm.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_public.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_enhancer.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_midi.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_data.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_mixer.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_pcm.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_flog.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_math.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_smf.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_smfdata.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_tcdata.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_tonecontrol.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_voicemgt.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_wtengine.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_wtsynth.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_mixbuf.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_pan.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_pcmdata.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_enhdata.c
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/eas_mididata.c
)

aic_library_sources_ifdef(
    CONFIG_MIDI_TEST_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/midi_test.c
)

aic_library_sources_ifdef(
    CONFIG_MIDI_SUPPORT
    ${SRCS}
)

aic_library_sources_ifdef(
    CONFIG_MIDI_HIGH
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/400kb_8b_32khz.c
)

aic_library_sources_ifdef(
    CONFIG_MIDI_MID
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/170kb_8b_22khz.c
)

aic_library_sources_ifdef(
    CONFIG_MIDI_LOW
    ${CMAKE_CURRENT_LIST_DIR}/lib_src/75k_8b_22k_128.c
)

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/lib_src
    ${CMAKE_CURRENT_LIST_DIR}/host_src
    ${AIC_SOURCE_ROOT}/mm/audio
    ${AIC_SOURCE_ROOT}/mm/resource
    ${AIC_SOURCE_ROOT}/mm/codec
)