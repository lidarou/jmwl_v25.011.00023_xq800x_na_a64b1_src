
aic_include_directories(
    ./nvfile/include
)
add_subdirectory_ifdef(CONFIG_MISC_SUPPORT mac)
add_subdirectory_ifdef(CONFIG_MISC_SUPPORT sn)
add_subdirectory_ifdef(CONFIG_FTL factory)
add_subdirectory_ifdef(CONFIG_MISC_SUPPORT calibration)
add_subdirectory_ifdef(CONFIG_SUBSYS_CP calibration)
add_subdirectory(imei)
