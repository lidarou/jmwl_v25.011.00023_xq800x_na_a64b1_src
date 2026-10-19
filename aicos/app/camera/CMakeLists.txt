# General configuration options
#
# Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.

if(CONFIG_RESOLUTION_128x128)

aic_library_sources_ifdef(CONFIG_APP_CAMERA
        ./camera_128x128.c
)

else()

aic_library_sources_ifdef(CONFIG_APP_CAMERA
        ./camera.c
)

endif()

aic_library_sources_ifdef(CONFIG_APP_CAMERA
	    ./camera_flow.c
	    ./camera_jpg_generator.c
	    ./camera_service.c
)

aic_library_include_directories(
    ${AIC_SOURCE_ROOT}/app/camera/
)
