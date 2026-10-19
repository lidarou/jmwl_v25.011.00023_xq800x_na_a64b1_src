# General configuration options
#
# Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

aic_library_sources(
    ${CMAKE_CURRENT_LIST_DIR}/pmu_core.c
    ${CMAKE_CURRENT_LIST_DIR}/pmu_kit.c
    ${CMAKE_CURRENT_LIST_DIR}/${CONFIG_PMIC_VERSION}/pmu_driver.c
)

aic_library_sources_ifdef(CONFIG_DRIVER_PMIC_PMU_DEMO pmic_pmu_demo.c)
aic_library_sources_ifdef(CONFIG_PMIC_PMU_AT pmu_at.c)