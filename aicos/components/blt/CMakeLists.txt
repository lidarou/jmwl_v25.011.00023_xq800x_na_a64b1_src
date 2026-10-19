cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(aic_blt
    VERSION 1.0.0
    LANGUAGES C
)

#
# Create Library and add Curent Library to project library list
# aic_library_named is register open source code library
#
aic_library_named(${PROJECT_NAME})
aic_library_include_directories(inc)

aic_library_include_directories_ifdef(
    CONFIG_BL6XX3
    ${AIC_SOURCE_ROOT}/drivers/graphics/tp/bl6xx3/
)
aic_library_sources_ifdef(CONFIG_SUBSYS_AP
    ./src/aic_blt_at.c
)
