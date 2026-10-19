#
# Add source code to library
#
aic_library_sources(
    src/syscalls.c
    src/assert.c
    src/fcntl.c
    src/ioctl.c
    src/mem_r.c
    src/mem_ex.c
#    src/random.c
    src/time/time.c
)

aic_library_sources_ifdef(
    CONFIG_PLAT_RAC_LIBC_TIME
    src/time/gmtime.c
    src/time/strftime.c
    src/time/localtime_r.c
)
