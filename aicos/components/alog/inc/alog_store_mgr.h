/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log storage manager header file.
 */
#ifndef __ALOG_STORAGE_MGR_H__
#define __ALOG_STORAGE_MGR_H__
#include <fs_api.h>
#include <alog_inner.h>

/*
 * CONFIG_STORE_LOG_COPY - Open this feature will
 * create a thread to copy log and save to file.
 *
 * CONFIG_STORE_INIT_BUFS  - Open this feature will
 * only malloc buffer in alog store init.
 *
 * CONFIG_STORE_RW_BUFS - Open this feature will use rw buffers.
 */
#ifdef CONFIG_ALOG_STORE_ENABLE_COPY
#define CONFIG_STORE_LOG_COPY   1
#define CONFIG_STORE_INIT_BUFS  1
#define CONFIG_STORE_RW_BUFS    0
#else
#define CONFIG_STORE_LOG_COPY   0
#define CONFIG_STORE_INIT_BUFS  0
#define CONFIG_STORE_RW_BUFS    0
#endif

/* if enable store check. */
#define CONFIG_STORE_ASSERT     0

/* if enable store debug. */
#define CONFIG_STORE_DEBUG      0

/* if enable store statitic. */
#define CONFIG_STORE_STATI      0

/* RW buffer size. */
#define STORE_INIT_BUF_SIZE     SIZE_64K

/* must be the power of 2. */
#define STORE_INIT_BUF_CNT      4

/* max flush log time 1s. */
#define FLUSH_LOG_MAX_TIME      30

#ifdef WIN32
#define LOG_WORK_DIR            "../../../../out/win32/Debug/"
#define FS_SUCCESS             0

/*
 * if enable win32 sim fs api.
 * enable: use fs api.
 * disable: use win32 api
 */
#define CONFIG_WIN32_SIM_FS     0

#if IS_CONF_ENABLED(CONFIG_WIN32_SIM_FS)
#define CONFIG_USE_FS_API       1
#else
#define CONFIG_USE_FS_API       0
#endif
#else

/*
 * not win32, must disable CONFIG_WIN32_SIM_FS
 * and enable .CONFIG_USE_FS_API
 */
#define CONFIG_WIN32_SIM_FS     0
#define CONFIG_USE_FS_API       1
#endif

#if IS_CONF_ENABLED(CONFIG_USE_FS_API)
#define F_RDONLY_M                  (FS_O_RDONLY)
#define F_WTONLY_M                  (FS_O_CREATE | FS_O_WRONLY | FS_O_TRUNC)
#define F_RDWT_M                    (FS_O_CREATE  | FS_O_RDWR | FS_O_TRUNC)
#define F_APEND_M                   (FS_O_CREATE | FS_O_RDWR | FS_O_APPEND)

#define STORE_INVALID_FD            -1
#define STORE_IS_VALID_FD(fd)       (fd >= 0)
#define STORE_IS_INVALID_FD(fd)     (fd < 0)

#define F_SEEK_START                FS_SEEK_BEGIN
#define F_SEEK_CUR                  FS_SEEK_CUR
#define F_SEEK_END                  FS_SEEK_END

#define F_SUCCESS                   AIC_SUCCESS

/* data define */
#define fsize                       ssize_t
#define fhandle                     int

/* api define */
#define store_fopen                 fs_open
#define store_fclose                (void)fs_close
#define store_fread                 fs_read
#define store_fwrite                fs_write
#define store_ffush                 fs_flush
#define store_fexist                fs_pathisexist
#define store_fremove               fs_delete
#define store_frename               fs_rename
#define store_fseek                 fs_seek
#define store_mkdir                 fs_createdir
#define store_chdir                 fs_setcurrentdir
#define store_rmdir                 fs_rmdir
#define store_getfree               fs_getdiskfreespace

/*
 * xxx_mmdd_hhmmss,
 * like: D:/aiclog/0302_083027
 */
#define STORE_LOG_VOLUME            "/D"
#define STORE_LOG_ROOT_DIR          "/D/aiclog"
#define STORE_LOG_DIR_FMT           "/D/aiclog/%02d%02d_%02d%02d%02d"
#define STORE_LOG_DIR_LEN           21
#define DIRNAME_BUF_SIZE           (STORE_LOG_DIR_LEN + 1)

/*
 * xxx_mmdd_hhmmss/mmdd_hhm_index.type,
 * like: D:/aiclog/0302_083027/0302_093027_001.data
 *       D:/aiclog/0302_083027/0302_093027_001.dump
 */
#define STORE_LOG_FILE_FMT          "%s/%02d%02d_%02d%02d%02d_%03d.%s"
#define STORE_LOG_FILE_LEN          42
#define NAME_BUF_SIZE               (STORE_LOG_FILE_LEN + 1)

/* some store paramete define. */
#define MAX_STORE_LOG_FILE_SIZE     1024*1024*300
#define MIN_STORE_LEFT_SIZE         1024*1024*500
#define MAX_STORE_LOG_FILE_CNT      100
#define MAX_STORE_LOG_DIR_CNT       100
#else
#include <errno.h>
#include <direct.h>
#include <io.h>
#include <drv_file.h>

/* macro define */
#define F_RDONLY_M                  "rb"
#define F_WTONLY_M                  "wb"
#define F_RDWT_M                    "wb+"
#define F_APEND_M                   "ab+"

#define F_SEEK_START                SEEK_SET
#define F_SEEK_CUR                  SEEK_CUR
#define F_SEEK_END                  SEEK_END

#define F_SUCCESS                   0

#define STORE_INVALID_FD            NULL
#define STORE_IS_VALID_FD(fd)       (fd != NULL)
#define STORE_IS_INVALID_FD(fd)     (fd == NULL)

/* data define */
#define fsize                       size_t
#define fhandle                     FILE *

/* api define */
#define store_fopen(fd, mode)       fopen(fd, mode)
#define store_fclose(fd)            fclose(fd)
#define store_fread(fd, buf, size)  fread(buf, 1, size, fd)
#define store_fwrite(fd, buf, size) fwrite(buf, 1, size, fd)
#define store_ffush(fd)             fflush(fd)
#define store_fexist(path)          _access(path, _A_NORMAL)
#define store_fremove(path)         remove(path)
#define store_frename(old_p, new_p) rename(old_p, new_p)
#define store_fseek(fd, off, mode)  fseek(fd, off, mode)
#define store_mkdir(path)           _mkdir(path)
#define store_chdir(path)           _chdir(path)
#define store_rmdir(path)           file_rm_all_dir(path)
#define store_getfree(v, f, t)      file_get_free(CUR_DRV, f, t)

/*
 * xxx_mmdd_hhmmss,
 * like: /aiclog/0302_083027
 */
#define STORE_LOG_VOLUME            "./"
#define STORE_LOG_ROOT_DIR          "./aiclog"
#define STORE_LOG_DIR_FMT           "./aiclog/%02d%02d_%02d%02d%02d"
#define STORE_LOG_DIR_LEN          20
#define DIRNAME_BUF_SIZE           (STORE_LOG_DIR_LEN + 1)

/*
 * xxx_mmdd_hhmmss/mmdd_hhm_index.type,
 * like: aiclog/0302_083027/0302_093027_001.data
 *       aiclog/0302_083027/0302_093027_001.dump
 */
#define STORE_LOG_FILE_FMT          "%s/%02d%02d_%02d%02d%02d_%03d.%s"
#define STORE_LOG_FILE_LEN          42
#define NAME_BUF_SIZE               (STORE_LOG_FILE_LEN + 1)

/* some store paramete define. */
#define MAX_STORE_LOG_FILE_SIZE     1024*1024*300
#define MIN_STORE_LEFT_SIZE         1024*1024*300
#define MAX_STORE_LOG_FILE_CNT      100
#define MAX_STORE_LOG_DIR_CNT       100
#endif /* CONFIG_USE_FS_API */

/* win32 simulator fs, space is very small. */
#if IS_CONF_ENABLED(CONFIG_WIN32_SIM_FS)
#undef MAX_STORE_LOG_FILE_SIZE
#undef MIN_STORE_LEFT_SIZE
#undef MAX_STORE_LOG_FILE_CNT
#undef MAX_STORE_LOG_DIR_CNT
#define MAX_STORE_LOG_FILE_SIZE     1024*16
#define MIN_STORE_LEFT_SIZE         1024*16
#define MAX_STORE_LOG_FILE_CNT      5
#define MAX_STORE_LOG_DIR_CNT       5
#endif /* CONFIG_WIN32_SIM_FS */

/*
 * store idle thread.
 */
#define STORE_IDLE_THREAD_NAME      "store_idle"
#define STORE_IDLE_THREAD_PRI       15
#define STORE_IDLE_THREAD_STACK     2048

/*
 * store type: log and dump.
 */
typedef enum {
    STORE_LOG = 0,
    STORE_DUMP
} STORE_TYPE_T;

#define STORE_DUMP_FILE             "dump"
#define STORE_LOG_FILE              "data"

#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
/* must be the power of 2. */
#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
#define MAX_STORE_ITEM              STORE_INIT_BUF_CNT
#else
#define MAX_STORE_ITEM              32
#endif /* CONFIG_STORE_INIT_BUFS */

#define GET_RING_POS(index)         (index & (MAX_STORE_ITEM - 1))
#define IS_RING_FULL(rd, wt)        ((u32)(rd + MAX_STORE_ITEM) == wt)

/*
 * store thread define.
 */
#define STORE_WRITE_THREAD_NAME     "store_write"
#define STORE_WRITE_THREAD_PRI      2
#define STORE_WRITE_THREAD_STACK    2048

/*
 * store item define.
 */
typedef struct STORE_ITEM_TYPE {
    u8      *buf_ptr;
    size_t  wt_size;
} STORE_ITEM_T;
#endif /* CONFIG_STORE_LOG_COPY */

/*
 * alog storage manager data,
 */
typedef struct ALOG_STORAGE_DATA {
#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
    aos_thread_id       write_thread;
    aos_semaphore_ptr   write_sem;
    aos_semaphore_ptr   read_sem;
    u32 volatile        rdptr;
    u32 volatile        wtptr;
    STORE_ITEM_T        cache[MAX_STORE_ITEM];
#endif /* CONFIG_STORE_LOG_COPY */

    aos_thread_id       idle_thread;
    aos_semaphore_ptr   idle_sem;

    u32                 lost_cnt;
    u16                 log_index;
    u16                 dir_index;
    u16                 dir_cnt;
    u16                 dump_index;
    volatile u8         b_stop;

    fhandle volatile    log_fd;
    fhandle volatile    wr_log_fd;
    fhandle             dump_fd;
    fhandle             assert_fd;

    fhandle             dir_rec_fd;
    fhandle             file_rec_fd;
    long                dir_rec_off;
    long                file_rec_off;

    bool                write_pending;
    volatile bool       sd_unmount;

    bool                init_root_dir;
    u32                 log_size;
    u32                 last_tick;
    char                root_dir[DIRNAME_BUF_SIZE];
} ALOG_STORAGE_T;

#endif /* __ALOG_STORAGE_MGR_H__ */

