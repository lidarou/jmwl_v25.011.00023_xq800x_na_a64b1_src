
set(SRCS
    ./src/connmgr_main.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(CONFIG_AT_BYPASS_TO_CP
    ./src/connmgr_at_bypass.c
)
