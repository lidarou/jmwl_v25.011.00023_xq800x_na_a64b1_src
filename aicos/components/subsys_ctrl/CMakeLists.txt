
set(SRCS
    ./src/subsys_main.c
    ./src/subsys_loader.c
    ./src/subsys_nv.c
    ./src/subsys_secboot.c
    ./src/subsys_state_proc.c
)

#
# Add source code to library
#
aic_library_sources(${SRCS})
