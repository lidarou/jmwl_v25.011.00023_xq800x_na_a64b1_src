# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_named(ivtts)

set(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tts_ifly.c
)

aic_library_sources(${SOURCES})

aic_library_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/lib
	${AIC_SOURCE_ROOT}/mm/include
)

aic_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
)

aic_library_import(libjrtc0
    ${CMAKE_CURRENT_LIST_DIR}/lib/libaisound50.a
)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/lib/ivTTS_irf.bin DESTINATION ${PROJECT_BINARY_DIR}/bin)