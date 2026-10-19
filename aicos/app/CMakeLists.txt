cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_apps
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
#
aic_library_named(${PROJECT_NAME})

file(GLOB SOURCES ./*.c)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/app/
    ${AIC_SOURCE_ROOT}/app/util
    ${AIC_SOURCE_ROOT}/app/util/backlight/
    ${AIC_SOURCE_ROOT}/app/util/factoryreset/
    ${AIC_SOURCE_ROOT}/app/util/audio/
    ${AIC_SOURCE_ROOT}/app/util/sleep/
    ${AIC_SOURCE_ROOT}/app/em/
    ${AIC_SOURCE_ROOT}/app/idle/
    ${AIC_SOURCE_ROOT}/app/call/
    ${AIC_SOURCE_ROOT}/app/settings/
    ${AIC_SOURCE_ROOT}/app/photo/
    ${AIC_SOURCE_ROOT}/app/camera/
    ${AIC_SOURCE_ROOT}/app/audiofocus/
    ${AIC_SOURCE_ROOT}/app/recorder/
    ${AIC_SOURCE_ROOT}/app/fota/
    ${AIC_SOURCE_ROOT}/app/resource
    ${AIC_SOURCE_ROOT}/app/resource/ring
    ${AIC_SOURCE_ROOT}/app/sms_test
    ${AIC_SOURCE_ROOT}/app/notify_center
    ${AIC_SOURCE_ROOT}/app/usb
)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/app/widgets/
    ${AIC_SOURCE_ROOT}/app/idle_fp/
    ${AIC_SOURCE_ROOT}/app/home_fp/
    ${AIC_SOURCE_ROOT}/app/phonebook_fp/
    ${AIC_SOURCE_ROOT}/app/sms_fp/
    ${AIC_SOURCE_ROOT}/app/settings_fp/
    ${AIC_SOURCE_ROOT}/app/settings_fp/flow
    ${AIC_SOURCE_ROOT}/app/calllog_fp/
    ${AIC_SOURCE_ROOT}/app/call_fp/
    ${AIC_SOURCE_ROOT}/app/tools_fp/
    ${AIC_SOURCE_ROOT}/app/multimedia_fp/
    ${AIC_SOURCE_ROOT}/app/remind_fp/
)

aic_library_sources_ifdef(
    CONFIG_APP_ENTRY_NOGUI
    ./app_nogui.c
    ./app_msg.c
)

aic_library_sources_ifndef(
    CONFIG_APP_ENTRY_NOGUI
    ./app.c
    ./app_msg.c
)

if(NOT CONFIG_APP_ENTRY_NOGUI)

    add_subdirectory(resource)
    add_subdirectory(bootanimation)
    add_subdirectory(lvgl_demo_apps)
    add_subdirectory(em)
    add_subdirectory(util)
    add_subdirectory(idle)
    add_subdirectory(notify_center)
    add_subdirectory(call)
    add_subdirectory(filebrowser)
    add_subdirectory(fota)
    add_subdirectory(usb)
    add_subdirectory(app_comp)
    add_subdirectory(photo)
    add_subdirectory(camera)
    add_subdirectory(audiofocus)
    add_subdirectory(recorder)
    add_subdirectory(settings)
    add_subdirectory(sms_test)

    add_subdirectory(widgets)
    add_subdirectory(idle_fp)
    add_subdirectory(home_fp)
    add_subdirectory(phonebook_fp)
    add_subdirectory(sms_fp)
    add_subdirectory(settings_fp)
    add_subdirectory(calllog_fp)
    add_subdirectory(call_fp)
    add_subdirectory(tools_fp)
    add_subdirectory(multimedia_fp)
    add_subdirectory(remind_fp)

#
# Create Library and add Curent Library to project library list
# aic_sdk_library is register close source code library
# Note!!!!!!
# The following code would be close source code
aic_sdk_library(app_flow)

if(NOT CONFIG_SDK_ENABLE)
    aic_library_include_directories(
        ${AIC_SOURCE_ROOT}/app/
        ${AIC_SOURCE_ROOT}/app/util
        ${AIC_SOURCE_ROOT}/app/util/backlight/
        ${AIC_SOURCE_ROOT}/app/util/factoryreset/
        ${AIC_SOURCE_ROOT}/app/util/audio/
        ${AIC_SOURCE_ROOT}/app/util/sleep/
        ${AIC_SOURCE_ROOT}/app/idle/flow/
        ${AIC_SOURCE_ROOT}/app/call/flow/
        ${AIC_SOURCE_ROOT}/app/settings/flow/
        ${AIC_SOURCE_ROOT}/app/resource/ring
        ${AIC_SOURCE_ROOT}/app/usb/flow/
        ${AIC_SOURCE_ROOT}/app/call
        ${AIC_SOURCE_ROOT}/app/app_comp/statusbar/flow
    )

    set(SRCS
        ${AIC_SOURCE_ROOT}/app/idle/flow/idle_flow.c
        ${AIC_SOURCE_ROOT}/app/call/flow/call_flow.c
        ${AIC_SOURCE_ROOT}/app/settings/flow/settings_flow.c
        ${AIC_SOURCE_ROOT}/app/usb/flow/usb_flow.c
        ${AIC_SOURCE_ROOT}/app/app_comp/statusbar/flow/statusbar_flow.c
    )

    aic_library_sources(${SRCS})
endif()
endif()

#add_subdirectory(samples)
add_subdirectory(myproj)
