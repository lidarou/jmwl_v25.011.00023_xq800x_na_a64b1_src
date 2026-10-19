cmake_minimum_required(VERSION 3.13)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
# Note!!!!!!
# The following code would be open source code
aic_library_named(drivers)

add_subdirectory(arch/${ARCH})

add_subdirectory(bspatch)
add_subdirectory(iodev)
add_subdirectory(misc)
add_subdirectory(pmic)
add_subdirectory(usb)
add_subdirectory(sound)
add_subdirectory(chip_id)

add_subdirectory_ifdef(CONFIG_PMIC_I2C              pmic_i2c)

add_subdirectory_ifdef(CONFIG_AIC_CLK               clk)
add_subdirectory_ifdef(CONFIG_DRIVER_DMA            dma)
add_subdirectory_ifdef(CONFIG_DRIVER_EFUSE          efuse)
add_subdirectory_ifdef(CONFIG_EIPU                  eipu)
add_subdirectory_ifdef(CONFIG_DRIVER_EXAMPLE        example)
add_subdirectory_ifdef(CONFIG_DRIVER_GPD            gpd)
add_subdirectory_ifdef(CONFIG_DRIVER_GPIO           gpio)
add_subdirectory_ifdef(CONFIG_DRIVER_GPO            gpo)
add_subdirectory_ifdef(CONFIG_CRYPTO                crypto)
add_subdirectory_ifdef(CONFIG_FMD                   fmd)
add_subdirectory_ifdef(CONFIG_DRIVER_LZMA_DEC       lzma)
add_subdirectory_ifdef(CONFIG_NET_DRIVER            network)
add_subdirectory_ifdef(CONFIG_PWM                   pwm)
add_subdirectory_ifdef(CONFIG_DRIVER_SPINLOCK       spinlock)
add_subdirectory_ifdef(CONFIG_KPC                   kpc)
add_subdirectory_ifdef(CONFIG_DRIVER_HLZMA          hlzma)
add_subdirectory_ifdef(CONFIG_DRIVER_I2C            i2c)
add_subdirectory_ifdef(CONFIG_IPU                   ipu)
add_subdirectory_ifdef(CONFIG_DRIVER_UART           serial)
add_subdirectory_ifdef(CONFIG_DRIVER_TIMER          timer)
add_subdirectory_ifdef(CONFIG_GRAPHICS              graphics)
add_subdirectory_ifdef(CONFIG_DRIVER_WDT            wdt)
add_subdirectory_ifdef(CONFIG_PIN_CTRL              pin_ctrl)
add_subdirectory_ifdef(CONFIG_DRIVER_IIS            iis)
add_subdirectory_ifdef(CONFIG_DRIVER_SDMMC          sdmmc)
add_subdirectory_ifdef(CONFIG_SSP                   ssp)
add_subdirectory_ifdef(CONFIG_DRIVER_THERMC         thermc)
add_subdirectory_ifdef(CONFIG_MHA                   mha)
add_subdirectory_ifdef(CONFIG_AIC_SYSOFF            sysoff)
add_subdirectory_ifdef(CONFIG_AIC_TRNG              trng)
add_subdirectory_ifdef(CONFIG_AIC_BB_CALI           bb_cali)
add_subdirectory_ifdef(CONFIG_DRIVER_CAN            can)

#
# Create Library and add Curent Library to project library list
# aic_sdk_library is register close source code library
# Note!!!!!!
# The following code would be close source code
aic_sdk_library(cs_drivers)

add_sdk_subdirectory(psram)
add_sdk_subdirectory(debug)

add_sdk_subdirectory_ifdef(CONFIG_ADRMAP                adrmap)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_AMBOX          ambox)
add_sdk_subdirectory_ifdef(CONFIG_CPREMAP               cpremap)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_BUSMON         busmonitor)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_CORESIGHT      coresight)
add_sdk_subdirectory_ifdef(CONFIG_CMUX                  cmux)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_DMC_MPU        dmc_mpu)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_EXT_MODEM      ext_modem)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_MODEMSYS       modem_sys)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_PCDUMP         pcdump)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_PERFORMANCE    perf)
add_sdk_subdirectory_ifdef(CONFIG_SNC_QSPI              qspi)
add_sdk_subdirectory_ifdef(CONFIG_DEEP_SLEEP            sleep)
add_sdk_subdirectory_ifdef(CONFIG_SUBSYS_AP             sound/driver/stream/pdm)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_SUBSYS_CTRL    subsys)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_ZJTAG          zjtag)
add_sdk_subdirectory_ifdef(CONFIG_DRIVER_TEST           zz_test)
