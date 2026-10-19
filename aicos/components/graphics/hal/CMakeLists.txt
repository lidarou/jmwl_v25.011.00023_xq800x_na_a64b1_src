cmake_minimum_required(VERSION 3.13)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/drivers/graphics/tp/bl6xx3/
)

aic_library_sources_ifdef(CONFIG_LV_CONF_MINIMAL
    ./lv_graphics_hal.c
    ./lv_aicos.c
    ./lv_port_indev.c
)
aic_library_sources_ifdef(CONFIG_KPC
    ./lv_port_indev_keypad.c
)
aic_library_sources_ifdef(CONFIG_TSC
    ./lv_port_indev_touchpad.c
    ./lv_port_indev_touchpad_cali.c
)
