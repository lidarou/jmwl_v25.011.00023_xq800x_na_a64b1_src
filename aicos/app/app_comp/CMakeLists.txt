# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/app/app_comp/statusbar
    ${AIC_SOURCE_ROOT}/app/app_comp/softkey
    ${AIC_SOURCE_ROOT}/app/app_comp/camera_preview
    ${AIC_SOURCE_ROOT}/app/app_comp/msgbox
)


add_subdirectory(statusbar)
add_subdirectory(softkey)
add_subdirectory(camera_preview)
add_subdirectory(msgbox)

