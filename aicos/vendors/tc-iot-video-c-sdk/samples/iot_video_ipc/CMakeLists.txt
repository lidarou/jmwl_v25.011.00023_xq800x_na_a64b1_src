# set(CMAKE_VERBOSE_MAKEFILE ON)


set(src_sample ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

aic_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/system)
file(GLOB sys_samples_src ${CMAKE_CURRENT_SOURCE_DIR}/system/*.c)
list(APPEND src_sample ${sys_samples_src})

aic_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/model)
file(GLOB model_samples_src ${CMAKE_CURRENT_SOURCE_DIR}/model/*.c)
list(APPEND src_sample ${model_samples_src})

aic_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/virtual_ipc)
file(GLOB vipc_samples_src ${CMAKE_CURRENT_SOURCE_DIR}/virtual_ipc/*.c)
list(APPEND src_sample ${vipc_samples_src})


aic_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/av)
file(GLOB av_samples_src ${CMAKE_CURRENT_SOURCE_DIR}/av/*.c)
list(APPEND src_sample ${av_samples_src})



aic_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cloud_storage)
file(GLOB cs_samples_src ${CMAKE_CURRENT_SOURCE_DIR}/cloud_storage/*.c)
list(APPEND src_sample ${cs_samples_src})

aic_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/upgrade)
file(GLOB upgrade_samples_src ${CMAKE_CURRENT_SOURCE_DIR}/upgrade/*.c)
list(APPEND src_sample ${upgrade_samples_src})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/device_info/voip DESTINATION ${AIC_SOURCE_ROOT}/projects/genesis/ap/common_inc/romfs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/device_info/device_info.json DESTINATION ${AIC_SOURCE_ROOT}/projects/genesis/ap/common_inc/romfs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/device_info/tc_iot_video_cfg.conf DESTINATION ${AIC_SOURCE_ROOT}/projects/genesis/ap/common_inc/romfs)

message("------------ipc demo LIBRARY_OUTPUT_PATH = ${LIBRARY_OUTPUT_PATH}")

aic_library_sources(${src_sample})


