# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(
    CONFIG_DRIVER_LCDC
    ${CMAKE_CURRENT_LIST_DIR}/core.c
    ${CMAKE_CURRENT_LIST_DIR}/lcdc/lcdc_drv.c
)

aic_library_sources_ifdef(
    CONFIG_ST7789
    ${CMAKE_CURRENT_LIST_DIR}/panel/st7789/panel.c
)

aic_library_sources_ifdef(
    CONFIG_GC9306
    ${CMAKE_CURRENT_LIST_DIR}/panel/gc9306/panel.c
)

aic_library_sources_ifdef(
    CONFIG_GC9307
    ${CMAKE_CURRENT_LIST_DIR}/panel/gc9307/panel.c
)

aic_library_sources_ifdef(
    CONFIG_IPS9107
    ${CMAKE_CURRENT_LIST_DIR}/panel/ips9107/panel.c
)

aic_library_sources_ifdef(
    CONFIG_ST7315
    ${CMAKE_CURRENT_LIST_DIR}/panel/st7315/panel.c
)

aic_library_sources_ifdef(
    CONFIG_ST7365
    ${CMAKE_CURRENT_LIST_DIR}/panel/st7365/panel.c
)

aic_library_sources_ifdef(
    CONFIG_ST7567
    ${CMAKE_CURRENT_LIST_DIR}/panel/st7567/panel.c
)

aic_library_sources_ifdef(
    CONFIG_ST7735
    ${CMAKE_CURRENT_LIST_DIR}/panel/st7735/panel.c
)

aic_library_sources_ifdef(
    CONFIG_AXS15231B
    ${CMAKE_CURRENT_LIST_DIR}/panel/axs15231b/panel.c
)

aic_library_sources_ifdef(
    CONFIG_DISPLAY_SHOWLOGO
    ${CMAKE_CURRENT_LIST_DIR}/garage/display_showlogo.c
)

aic_library_sources_ifdef(
    CONFIG_AIC_DISPLAY_AT
    ${CMAKE_CURRENT_LIST_DIR}/garage/aic_display_at_handle.c
)

aic_library_sources_ifdef(
    CONFIG_KPLED_BACKLIGHT
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_kpled.c
)

aic_library_sources_ifdef(
    CONFIG_LED_RGBW_BACKLIGHT
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_led_rgbw.c
)

aic_library_sources_ifdef(
    CONFIG_PWM_BACKLIGHT
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_pwm.c
)
aic_library_sources_ifdef(
    CONFIG_PULSE_DIMMING_BACKLIGHT
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_pulse_dimming.c
)

aic_library_sources_ifdef(
    CONFIG_BACKLIGHT_DEMO
    ${CMAKE_CURRENT_LIST_DIR}/backlight/backlight_demo.c
)

aic_library_sources_ifdef(
    CONFIG_BL6XX3
    ${CMAKE_CURRENT_LIST_DIR}/tp/bl6xx3/fw_update.c
    ${CMAKE_CURRENT_LIST_DIR}/tp/bl6xx3/tp_ts.c
)

