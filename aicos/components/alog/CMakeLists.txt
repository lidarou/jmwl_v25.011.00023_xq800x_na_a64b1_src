
set(SRCS
    ./src/alog_buffer.c
    ./src/alog_cfg.c
    ./src/alog_data.c
    ./src/alog_data_format.c
    ./src/alog_formater.c
    ./src/alog_main.c
    ./src/alog_outputer.c
    ./src/alog_trace.c
    ./src/alog_trace_filter.c
    ./src/alog_trace_format.c
    ./src/alog_diag_cmd.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(CONFIG_ALOG_STORE_FS
    ./src/alog_store_mgr.c
)

add_subdirectory_ifdef(CONFIG_NET_PCAP_DUMP pcap)