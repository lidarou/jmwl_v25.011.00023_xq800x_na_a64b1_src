cmake_minimum_required(VERSION 3.13)

#
# Create Library and add Curent Library to project library list
#
add_subdirectory(pwr_key)
add_subdirectory(status_light)
add_subdirectory_ifdef(CONFIG_PMIC_2ND_IRQ          pmic_2nd_irq)
add_subdirectory_ifdef(CONFIG_PMIC_IRQ              pmic_irq)
add_subdirectory_ifdef(CONFIG_PMIC_GPIO             gpio)
add_subdirectory_ifdef(CONFIG_PMIC_RTC              rtc)
add_subdirectory_ifdef(CONFIG_PMIC_UPS              ups)
add_subdirectory_ifdef(CONFIG_POWER_SUPPLY          power_supply)
add_subdirectory_ifdef(CONFIG_PMIC_EFUSE            efuse)
add_subdirectory_ifdef(CONFIG_PMIC_PMU              pmu)
add_subdirectory_ifdef(CONFIG_CLK_CALI              clk_cali)
add_subdirectory_ifdef(CONFIG_TSC                   tsc)
add_subdirectory_ifdef(CONFIG_PMIC_WDT              wdt)
add_subdirectory_ifdef(CONFIG_ADC                   adc)
add_subdirectory_ifdef(CONFIG_KPLED                 kpled)
add_subdirectory_ifdef(CONFIG_PMIC_CLK              pmic_clk)
add_subdirectory_ifdef(CONFIG_LAMP                  lamp)
add_subdirectory_ifdef(CONFIG_LPR                   lpr)

