cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_gps
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
#
aic_library_named(${PROJECT_NAME})

add_subdirectory_ifdef(CONFIG_ICOE_GPS    icoe)