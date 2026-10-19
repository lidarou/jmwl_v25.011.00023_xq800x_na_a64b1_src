# General configuration options
#
# Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources_ifdef(CONFIG_PMIC_I2C_DRIVER
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c_phy.c
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c_core.c
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c.c
)

aic_library_sources_ifdef(CONFIG_PMIC_I2C_TEST
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c_test.c
)


aic_library_sources_ifdef(CONFIG_PMIC_I2C_GP201
    ${CMAKE_CURRENT_LIST_DIR}/pmic_i2c_slave_test.c
)

aic_include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)