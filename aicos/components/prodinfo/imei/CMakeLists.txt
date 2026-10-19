aic_library_include_directories(inc)

#
# Add source code to library
#
aic_library_sources_ifdef(CONFIG_SUBSYS_AP
    ./src/imei_at.c
)

aic_library_sources_ifdef(CONFIG_SUBSYS_CP
    ./src/imei_diag.c
)
