# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
aic_library_sources_ifdef(
    CONFIG_PMIC_2ND_IRQ
    ${CMAKE_CURRENT_LIST_DIR}/pmic_2nd_irq.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_PMIC_VERSION}/pmu_int_phy.c
)