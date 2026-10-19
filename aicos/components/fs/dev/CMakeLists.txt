aic_library_include_directories(inc)



aic_library_sources_ifdef(CONFIG_FS_DEV_FLASH
    ./src/fs_dev_fmd.c
)

aic_library_sources_ifdef(CONFIG_FS_DEV_FTL
    ./src/fs_dev_ftl.c
)

aic_library_sources_ifdef(CONFIG_FS_DEV_FILE
    ./src/fs_dev_file.c
)

aic_library_sources_ifdef(CONFIG_FS_DEV_RAM
    ./src/fs_dev_ram.c
)

aic_library_sources_ifdef(CONFIG_FS_DEV_WIN32
    ./src/fs_dev_win32.c
)

aic_library_sources_ifdef(CONFIG_SDMMC_SD
    ./src/fs_dev_sdmmc.c
)

aic_library_sources_ifdef(CONFIG_FS_DEV_BD
    ./src/fs_dev_bd.c
)

