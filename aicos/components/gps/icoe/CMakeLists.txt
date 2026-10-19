set(SRCS
    ./gps_service.c
    ./src/icoe_gps_serial.c
    ./src/icoe_gps_if.c
    ./src/icoe_firmware_loader.c
    ./src/icoe_gps_hal.c
    ./src/icoe_gps_msg.c
    ./src/icoe_gps_nmea.c
    ./src/icoe_gps_os.c
    ./src/icoe_gps_rt.c
    ./src/icoe_http_decoder.c
)

aic_library_sources_ifdef(CONFIG_ICOE_GPS_SRC ${SRCS})

aic_include_directories(
    ./include
    ./inc
)

if(CONFIG_ICOE_GPS_FW)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bin/gnss_bl.bin     ${AIC_OUT_DIR}/bin/gnss_bl.bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bin/gnss_fw.bin     ${AIC_OUT_DIR}/bin/gnss_fw.bin
)
endif()

