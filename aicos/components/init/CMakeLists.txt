
set(SRCS
    ./src/aic_init.c
    ./src/aic_sys_misc.c
    ./src/aic_cmdline.c
    ./src/aic_boot_mode.c
    ./src/aic_sys_cons_cmd.c
    ./src/aic_sys_diag_cmd.c
    ./src/aic_sys_param.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})

aic_library_sources_ifdef(CONFIG_PLAT_THREADX_SAMPLE
    ./src/sample_threadx.c
)

aic_library_sources_ifdef(CONFIG_SUBSYS_AP
    ./src/aic_sys_debug_at.c
)

list(APPEND TOOLCHAIN_LD_FLAGS  "--keep *.o(.auto_init*)")
list(APPEND TOOLCHAIN_LD_FLAGS "--keep *.o(.aiccmd*)")
