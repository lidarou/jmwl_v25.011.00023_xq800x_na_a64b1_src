
set(SRCS
    ./src/diag_cmd_brr.c
    ./src/diag_cmd_codec.c
    ./src/diag_cmd_proc.c
    ./src/diag_cmd_rsp.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(CONFIG_SUBSYS_AP
    ./src/diag_cmd_agency.c
)

aic_library_sources_ifdef(CONFIG_SUBSYS_CP
    ./src/diag_cmd_input.c
)
