# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
add_subdirectory_ifdef(CONFIG_BAT     battery)
add_subdirectory_ifdef(CONFIG_BCD     bcd)
add_subdirectory_ifdef(CONFIG_CHGR    charger)

aic_library_sources_ifdef(
    CONFIG_AIC_POWER_SUPPLY_AT
    ${CMAKE_CURRENT_LIST_DIR}/aic_power_supply_at_handle.c
)

aic_library_sources_ifdef(
    CONFIG_POWER_SUPPLY_CORE
    ${CMAKE_CURRENT_LIST_DIR}/power_supply.c
)