# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
aic_library_sources_ifdef(
    CONFIG_PMIC_IRQ
    ${CMAKE_CURRENT_LIST_DIR}/pmic_irq.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_DRIVER_PMIC_IPU_VER}/pmic_ipu_drv.c
)