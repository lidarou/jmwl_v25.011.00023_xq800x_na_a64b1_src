cmake_minimum_required(VERSION 3.13)

#
# Create Library and add Curent Library to project library list
#

aic_library_named(codec)

set(SRCS
    ${CMAKE_CURRENT_LIST_DIR}/codec_mng.c
)

aic_library_sources(
    ${SRCS}
)

aic_library_sources_ifdef(
    CONFIG_HW_CODEC_SUPPORT
    ${CMAKE_CURRENT_LIST_DIR}/hwcodec.c
)

aic_library_include_directories(
    #${AIC_SOURCE_ROOT}/mm/audio
    ${AIC_SOURCE_ROOT}/mm/codec
)

add_subdirectory_ifdef(CONFIG_WAV_SUPPORT wav)
add_subdirectory_ifdef(CONFIG_MIDI_SUPPORT midi)
add_subdirectory_ifdef(CONFIG_MP3_SUPPORT mp3)
add_subdirectory_ifdef(CONFIG_AMR_SUPPORT amr)
