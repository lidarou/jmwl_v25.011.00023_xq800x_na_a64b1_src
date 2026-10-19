cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_osa
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
#
aic_export_library(${PROJECT_NAME})

#############################################################
# Public curent header file to total project
# Add the directory to INTERFACE LIBRARY
#############################################################
#
#the reent and ext libc
aic_include_directories_ifdef(CONFIG_PLAT_RAC_LIBC
    rae_libc/inc
)
if(CONFIG_RTOS_THREADX)
add_subdirectory_ifdef(CONFIG_MEM_MODE_TLSF      properties/source/mm)
endif()

if(CONFIG_RTOS_LITEOS_M)
add_subdirectory(bounds_checking_function)
endif()

add_subdirectory_ifdef(CONFIG_TX_CMSIS      cmsis)

add_subdirectory(common)

add_subdirectory_ifdef(CONFIG_ROM_CALL_SUPPORT     rom_call)

add_subdirectory_ifdef(CONFIG_PLAT_RAC_LIBC
	rae_libc
)
