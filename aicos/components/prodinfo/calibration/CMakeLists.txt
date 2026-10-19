aic_include_directories(./inc)

set(SRCS
    ./src/aic_cali_cmd.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(CONFIG_SUBSYS_AP
    ./src/aic_cali_data_cmd.c
    ./src/aic_cali_band_at_cmd.c
)

aic_library_sources_ifdef(CONFIG_SUBSYS_CP
    ./src/aic_cali_band_diag_cmd.c
)
