aic_library_include_directories(inc)

aic_library_sources_ifdef(CONFIG_FS_TEST ./src/fs_test.c )
aic_library_sources_ifdef(CONFIG_FS_POSIX_TEST ./src/fs_posix_test.c ) 
aic_library_sources_ifdef(CONFIG_FS_LIBC_TEST ./src/fs_libc_test.c ) 