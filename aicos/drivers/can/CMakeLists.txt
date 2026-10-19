cmake_minimum_required(VERSION 3.13)

aic_library_sources_ifdef(
                CONFIG_DRIVER_CAN
                can_drv.c
                )

aic_include_directories(
                ../include
)