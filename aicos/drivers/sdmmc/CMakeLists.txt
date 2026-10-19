# General configuration options
#
# Copyright 2022 by AIC Micro Corporation. All rights reserved.

aic_library_sources(
    sdmmc_core.c
    common/sdmmc_common.c
    common/sdhost_global.c
)

aic_library_sources_ifdef(
    CONFIG_SDMMC_SD
    sd/sd_drv.c
    sdmmc_dev.c
)

aic_library_sources_ifdef(
    CONFIG_SDMMC_SDIO
    sdio/sdio_drv.c
)

aic_library_sources_ifdef(
    CONFIG_SDMMC_EMMC
    emmc/emmc_drv.c
    emmc/sdhost_emmc.c
    sdmmc_dev.c
)

aic_library_sources_ifdef(
    CONFIG_SDMMC_ASICPHY
    common/dfi_asicphy.c
)

aic_library_sources_ifdef(
    CONFIG_SDMMC_FPGAPHY
    common/dfi_fpgaphy.c
    common/fpga_combo_phy.c
)

aic_include_directories(
    common
    sd
    emmc
    include
    ../include
    ../../components/fs/device/inc
)

