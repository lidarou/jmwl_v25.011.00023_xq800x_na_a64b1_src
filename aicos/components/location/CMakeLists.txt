# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

if(CONFIG_CUSTOMER_CM_AT OR CONFIG_GX_AT_V2)
aic_library_sources(location_at_cm.c)
endif()

