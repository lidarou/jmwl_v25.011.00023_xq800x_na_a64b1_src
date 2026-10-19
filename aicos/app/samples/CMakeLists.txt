# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_I2C_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/i2c_sample.c
)

aic_library_sources_ifdef(CONFIG_UART_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/uart_sample.c
)

aic_library_sources_ifdef(CONFIG_POWER_SUPPLY_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/power_supply_sample.c
)

aic_library_sources_ifdef(CONFIG_BACKLIGHT_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/backlight_sample.c
)

aic_library_sources_ifdef(CONFIG_SMS_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/sms/sms_sample.c
)

aic_library_sources_ifdef(CONFIG_RADIO_CFUN_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/radio_cfun_sample.c
)

aic_library_sources_ifdef(CONFIG_WIFISCAN_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/wifi_scan/wifi_scan_sample.c
)

aic_library_sources_ifdef(CONFIG_TELE_RADIO_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele/tele_radio_sample.c
)

aic_library_sources_ifdef(CONFIG_TELE_SMS_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele/tele_sms_sample.c
)

aic_library_sources_ifdef(CONFIG_TELE_SMS_EX_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele/tele_sms_ex_sample.c
)

aic_library_sources_ifdef(CONFIG_TELE_PB_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele/tele_pb_samples.c
)

aic_library_sources_ifdef(CONFIG_TELE_SIM_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele/tele_sim_sample.c
)

aic_library_sources_ifdef(CONFIG_TELE_CALL_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele/tele_call_sample.c
)

aic_library_sources_ifdef(CONFIG_WAKEUP_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/wakeup_sample.c
)

aic_library_sources_ifdef(CONFIG_SSP_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/ssp_sample.c
)

aic_library_sources_ifdef(APP_AUDIO_MP3_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/app_audio_mp3_sample.c
)

aic_library_sources_ifdef(CONFIG_AUTO_REG_IMS_SAMPLE
    ${CMAKE_CURRENT_LIST_DIR}/tele_ims_sample.c
)

add_subdirectory(network)
