# General configuration options
#
# Copyright 2023 by AIC Micro Corporation. All rights reserved.

aic_include_directories(
    ${AIC_SOURCE_ROOT}/os/osa/properties/include
)

if(NOT CONFIG_ROM_CALL_SUPPORT)
	aic_library_named(los_tlsf)
	
	aic_library_include_directories(
		${AIC_SOURCE_ROOT}/os/osa/properties/include/utils
	)

	set(LOS_SRCS
			./los_memory.c
	)

	#
	# Add source code to library
	#
	aic_library_sources(${LOS_SRCS})
endif()