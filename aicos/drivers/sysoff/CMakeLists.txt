cmake_minimum_required(VERSION 3.13)

aic_library_sources_ifdef(
    CONFIG_AIC_SYSOFF
    ./sysoff_manage.c
)

aic_library_sources_ifdef(
    CONFIG_AIC_SYSOFF_AT
    ./aic_sysoff_at_handle.c
)

