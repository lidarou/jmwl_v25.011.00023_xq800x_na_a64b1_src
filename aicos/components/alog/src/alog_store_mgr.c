/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log storage manager source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x011659
#define LOG_MODULE    AMOD_ALOG

#include <alog_store_mgr.h>
#include <alog_store_api.h>
#include <aic_io_dev.h>
#include <adbg_cfg_api.h>
#include <stdio.h>

/*
 * local macro define.
 */
#if IS_CONF_ENABLED(CONFIG_STORE_ASSERT)
#define STORE_ASSERT        AIC_ASSERT
#else
#define STORE_ASSERT(...)
#endif

#if IS_CONF_ENABLED(CONFIG_STORE_DEBUG)
#define STORE_DEBUG         alog_info
#define STORE_IDDBG         aidx_info
#else
#define STORE_DEBUG(...)
#define STORE_IDDBG(...)
#endif
#define STORE_INFO          alog_info
#define STORE_ERR           alog_error

#define ALOG_STORE_CHECK_FREE_SPACE() do { \
    int ret; \
 \
    ret = store_getfree(STORE_LOG_VOLUME, &free, &total); \
    if (ret != F_SUCCESS) \
        return ret; \
 \
    alog_info("total=0x%llx, free=0x%llx\n", total, free); \
    if (free > MIN_STORE_LEFT_SIZE) \
        return F_SUCCESS; \
    } while (0)

/*
 * local global define.
 */
ALOG_STORAGE_T *s_log_store;
bool    s_sd_mount;

#if IS_CONF_ENABLED(CONFIG_STORE_RW_BUFS)
static u8 s_stor_bufs[STORE_INIT_BUF_SIZE * STORE_INIT_BUF_CNT];
#endif

/*
 * alog_store_write.
 */
static void alog_store_write(fhandle fd, void *data, size_t size, u32 line)
{
    fsize write;

#if IS_CONF_ENABLED(CONFIG_STORE_STATI)
    u32 tick = alog_get_aon_timestamp();
    STORE_DEBUG("store save size=%d\n", size);
#endif

    write = store_fwrite(fd, data, size);
    STORE_ASSERT(size == write);
    if (size != (size_t)write) {
        aicx_error(0x1be247, "ddd", "line(%d) write err, size=%d, write=%d", line, size, write);
    }

#if IS_CONF_ENABLED(CONFIG_STORE_STATI)
    STORE_DEBUG("store save finish, tick=%d\n", alog_get_aon_timestamp() - tick);
#endif
}

/*
 * alog_store_remove_oldest_dir.
 */
static bool alog_store_remove_oldest_dir(ALOG_STORAGE_T *store)
{
    fhandle fd = store->dir_rec_fd;
    char dir[DIRNAME_BUF_SIZE];
    fsize rsize;
    int ret;

    rsize = store_fread(fd, dir, DIRNAME_BUF_SIZE);
    if (rsize <= 0)
        return FALSE;

    STORE_ASSERT(rsize == DIRNAME_BUF_SIZE);
    STORE_ASSERT(dir[STORE_LOG_DIR_LEN] == '\n');
    dir[STORE_LOG_DIR_LEN] = 0;

    /* if delete dir is cur write dir, return. */
    if (0 == strncmp(store->root_dir, dir, STORE_LOG_DIR_LEN)) {
        STORE_DEBUG("skip remove cur write dir %s.\n", dir);
        return TRUE;
    }

    /* update the removed dir offset. */
    store->dir_rec_off += (u32)rsize;

    ret = store_rmdir(dir);
    STORE_ASSERT(ret == F_SUCCESS);
    STORE_DEBUG("remove err=%d, dir %s.\n", ret, dir);

    if (ret == F_SUCCESS)
        store->dir_cnt--;

    return TRUE;
}

/*
 * alog_store_remove_oldest_file.
 */
static bool alog_store_remove_oldest_file(ALOG_STORAGE_T *store)
{
    fhandle fd = store->file_rec_fd;
    char file_name[NAME_BUF_SIZE];
    char log_str[10], dump_str[10];
    fsize rsize;
    u16 index;
    int ret;

    if (STORE_IS_INVALID_FD(fd))
        return FALSE;

    rsize = store_fread(fd, file_name, NAME_BUF_SIZE);
    if (rsize <= 0)
        return FALSE;

    STORE_ASSERT(rsize == NAME_BUF_SIZE);
    STORE_ASSERT(file_name[STORE_LOG_FILE_LEN] == '\n');
    file_name[STORE_LOG_FILE_LEN] = 0;

    /* if delete file index is cur write file index, return. */
    index = (store->log_index == 0) ?
            (MAX_STORE_LOG_FILE_CNT - 1) : (store->log_index - 1);
    snprintf(log_str, sizeof(log_str), "_%03d.log", index);

    index = (store->dump_index == 0) ?
            (MAX_STORE_LOG_FILE_CNT - 1) : (store->dump_index - 1);
    snprintf(dump_str, sizeof(dump_str), "_%03d.dmp", index);
    if (strstr(file_name, log_str) ||
        strstr(file_name, dump_str)) {
        STORE_DEBUG("skip remove cur write file %s.\n", file_name);
        return TRUE;
    }

    /* update the removed file offset. */
    store->file_rec_off += (u32)rsize;

    ret = store_fremove(file_name);
    STORE_ASSERT(ret == F_SUCCESS);

    STORE_DEBUG("remove err=%d, file %s.\n", ret, file_name);

    return TRUE;
}

/*
 * alog_store_check_fs_left_space.
 *
 * if space is not enough, will try to remove the oldest log.
 */
static int alog_store_check_fs_left_space(ALOG_STORAGE_T *store)
{
    bool b_contiue;
    u64 free, total;

    ALOG_STORE_CHECK_FREE_SPACE();

    /* First, remove old dir. */
    if (STORE_IS_INVALID_FD(store->dir_rec_fd))
        return AIC_ENOSPC;
    store_fseek(store->dir_rec_fd, store->dir_rec_off, F_SEEK_START);
    do {
        b_contiue = alog_store_remove_oldest_dir(store);
        ALOG_STORE_CHECK_FREE_SPACE();
    } while (b_contiue);

    /* Than, remove old file. */
    if (STORE_IS_INVALID_FD(store->file_rec_fd))
        return AIC_ENOSPC;
    store_fseek(store->file_rec_fd, store->file_rec_off, F_SEEK_START);
    do {
        b_contiue = alog_store_remove_oldest_file((store));
        ALOG_STORE_CHECK_FREE_SPACE();
    } while (b_contiue);

    return AIC_ENOSPC;
}

/*
 * alog_store_create_dir_rec_file.
 */
static void alog_store_create_dir_rec_file(ALOG_STORAGE_T *store)
{
    char file_name[DIRNAME_BUF_SIZE];
    char rec_name[DIRNAME_BUF_SIZE];
    char temp_name[DIRNAME_BUF_SIZE];
    fhandle temp_fd;
    bool not_exsit;
    fsize rsize;
    int ret;

    /* create log dir recorder file. */
    snprintf(rec_name, sizeof(rec_name), "%s/log_rec.log", STORE_LOG_ROOT_DIR);
    not_exsit = (F_SUCCESS != store_fexist(rec_name));
    store->dir_rec_fd = store_fopen(rec_name, F_APEND_M);
    store->dir_rec_off = 0;
    STORE_ASSERT(STORE_IS_VALID_FD(store->dir_rec_fd));
    if (STORE_IS_INVALID_FD(store->dir_rec_fd)) {
        aicx_error(0x0f2330, "d", "create log dir rec failed. fd=%d\n", store->dir_rec_fd);
        return;
    }

    /* if not exist, create it, and return.. */
    if (not_exsit)
        return;

    /* create a tmep file. */
    snprintf(temp_name, sizeof(temp_name), "%s/log_rec.tmp", STORE_LOG_ROOT_DIR);
    temp_fd = store_fopen(temp_name, F_WTONLY_M);
    STORE_ASSERT(STORE_IS_VALID_FD(temp_fd));
    if (STORE_IS_INVALID_FD(temp_fd)) {
        aicx_error(0x0a4755, "d", "create tmp dir rec failed. fd=%d\n", store->dir_rec_fd);
        return;
    }

    /* check all the recoder dir is exist..*/
    store_fseek(store->dir_rec_fd, 0, F_SEEK_START);
    do {
        rsize = store_fread(store->dir_rec_fd, file_name, DIRNAME_BUF_SIZE);
        if (rsize <= 0)
            break;

        STORE_ASSERT(rsize == DIRNAME_BUF_SIZE);
        STORE_ASSERT(file_name[STORE_LOG_DIR_LEN] == '\n');

        /* ending the read. */
        if (rsize != DIRNAME_BUF_SIZE)
            break;

        /* dir exist, save it to log dir recorder file. */
        file_name[STORE_LOG_DIR_LEN] = 0;
        if (F_SUCCESS == store_fexist(file_name)) {
            store->dir_cnt++;
            store->dir_index++;
            file_name[STORE_LOG_DIR_LEN] = '\n';
            alog_store_write(temp_fd, file_name, DIRNAME_BUF_SIZE, __LINE__);
        }
    } while (TRUE);

    /* use the temp file. */
    store_fclose(store->dir_rec_fd);
    store_fclose(temp_fd);
    ret = store_fremove(rec_name);
    STORE_ASSERT(ret == F_SUCCESS);
    ret = store_frename(temp_name, rec_name);
    STORE_ASSERT(ret == F_SUCCESS);

    store->dir_rec_fd = store_fopen(rec_name, F_APEND_M);
    store->dir_rec_off = 0;
}

/*
 * alog_store_write_log.
 *
 * save a packet log to log file.
 */
static void alog_store_write_log(ALOG_STORAGE_T *store, void *data, size_t size)
{
    size_t left = size;
    size_t offset = 0;

    /* wait wrtie fd ready. */
    if (STORE_IS_INVALID_FD(store->wr_log_fd) || store->sd_unmount)
        return;

    while (STORE_IS_INVALID_FD(store->wr_log_fd))
    {
        aos_sleep(10);
    }

    /* if pending write, just discard it. */
    if (store->write_pending) {
        store->lost_cnt++;
        STORE_DEBUG("save log pending,lost_cnt=%d\n", store->lost_cnt);
        return;
    }

    /*
     * The log file may has been changed, close the old file,
     * switch to new file and log_size reset to 0.
     */
    if (store->wr_log_fd != store->log_fd) {
        store_fclose(store->wr_log_fd);
        store->wr_log_fd = store->log_fd;
        store->log_size = 0;
    }

    do {
        fsize write = store_fwrite(store->wr_log_fd, (u8 *)data + offset, left);

        STORE_ASSERT((size_t)write == left);
        if (write <= 0 || (size_t)write > left) {
            aicx_error(0x11bb18, "dd", "save log error, left=%d, write=%d", left, write);
            store->lost_cnt++;
            break;
        }
        left -= write;
        offset += write;
        store->log_size += write;
    } while (left);

    /* notify idle task to create a new file. */
    if (store->log_size > MAX_STORE_LOG_FILE_SIZE) {
        aos_put_semaphore(store->idle_sem);
    }
}

/*
 * alog_store_creat_a_new_file.
 */
static void alog_store_creat_a_new_file(ALOG_STORAGE_T *store, u8 type)
{
    char file_name[NAME_BUF_SIZE];
    aos_systime time;
    char *str_type;
    u16 index;
    fhandle fd;
    AOS_INTERRUPT_DECLARE

    /* get index and type_str. */
    if (type == STORE_LOG) {
        index = store->log_index;
        str_type = STORE_LOG_FILE;
    } else {
        index = store->dump_index;
        str_type = STORE_DUMP_FILE;
    }

    STORE_ASSERT(strlen(store->root_dir) == STORE_LOG_DIR_LEN);
    if (strlen(store->root_dir) != STORE_LOG_DIR_LEN)
        return;

    aos_get_systime(&time);
    AOS_INTERRUPT_DISABLE
    snprintf(file_name, sizeof(file_name),
             STORE_LOG_FILE_FMT, store->root_dir,
             time.month, time.day, time.hour,
             time.minute, time.second,
             index, str_type);
    AOS_INTERRUPT_RESTORE

    fd = store_fopen(file_name, F_WTONLY_M);
    STORE_DEBUG("store open a new file, fd=0x%x, dir is %s.\n", fd, file_name);

    STORE_ASSERT(STORE_IS_VALID_FD(fd));
    if (STORE_IS_INVALID_FD(fd))
        return;

    /* updata index. */
    index++;

    /* save fd and index. */
    if (type == STORE_LOG) {
        store->log_fd = fd;
        if (STORE_IS_INVALID_FD(store->wr_log_fd))
            store->wr_log_fd = store->log_fd;
        store->log_index = index;
    } else {
        store->dump_fd = fd;
        store->dump_index = index;
    }

    /* recoder the new log file. */
    STORE_ASSERT(STORE_IS_VALID_FD(store->file_rec_fd));
    if (STORE_IS_VALID_FD(store->file_rec_fd)) {
        store_fseek(store->file_rec_fd, 0, F_SEEK_END);
        file_name[STORE_LOG_FILE_LEN] = '\n';
        alog_store_write(store->file_rec_fd, file_name, NAME_BUF_SIZE, __LINE__);
        store_ffush(store->file_rec_fd);
    }
}

/*
 * alog_store_update_root_dir.
 */
static int alog_store_update_root_dir(ALOG_STORAGE_T *store)
{
    char file_name[50 + 1];
    aos_systime time;
    int ret;

    STORE_ASSERT(store);

    if (!store)
        return AIC_ENODEV;

    if (F_SUCCESS != alog_store_check_fs_left_space(store)) {
        aicx_error(0x1b2757, "", "left space is not enough.\n");
        return AIC_ENOSPC;
    }

    STORE_INFO("store update root dir.\n");

    /* create the root dir. */
    do {
        aos_get_systime(&time);
        snprintf(file_name, sizeof(file_name), STORE_LOG_DIR_FMT,
                 time.month, time.day, time.hour, time.minute, time.second);
        ret = store_fexist(file_name);
        if (ret != F_SUCCESS)
            break;

        /* if exist, remove it. */
        ret = store_rmdir(file_name);
        if (ret == F_SUCCESS)
            break;

        STORE_ERR("store update root dir tm %s, ret=%d.\n", file_name, ret);

        aos_sleep(1000);
    } while (TRUE);

    ret = store_mkdir(file_name);
    aicx_info(0x12066e, "ds", "store update root dir, ret=%d, dir is %s.\n",
              ret, file_name);

    STORE_ASSERT(ret == F_SUCCESS);
    if (ret != F_SUCCESS)
        return ret;

    /* check log dir cnt. */
    if (store->dir_cnt > MAX_STORE_LOG_DIR_CNT - 1) {
        store_fseek(store->dir_rec_fd, store->dir_rec_off, F_SEEK_START);
        alog_store_remove_oldest_dir(store);
    }

    strncpy(store->root_dir, file_name, DIRNAME_BUF_SIZE);

    /* recoder the new log dir. */
    if (STORE_IS_VALID_FD(store->dir_rec_fd)) {
        store_fseek(store->dir_rec_fd, 0, F_SEEK_END);
        file_name[STORE_LOG_DIR_LEN] = '\n';
        alog_store_write(store->dir_rec_fd, file_name, DIRNAME_BUF_SIZE, __LINE__);
        store_ffush(store->dir_rec_fd);
    }

    /* create log rec file. */
    if (STORE_IS_VALID_FD(store->file_rec_fd))
        store_fclose(store->file_rec_fd);

    snprintf(file_name, sizeof(file_name),
             "%s/log_file_rec.log", store->root_dir);
    store->file_rec_fd = store_fopen(file_name, F_APEND_M);
    store->file_rec_off = 0;

    STORE_ASSERT(STORE_IS_VALID_FD(store->file_rec_fd));
    if (STORE_IS_INVALID_FD(store->file_rec_fd)) {
        aicx_error(0x1c201c, "d", "create log rec file failed. fd=%d\n", store->file_rec_fd);
        return AIC_ENOENT;
    }

    /* close the assert  file. */
    if (STORE_IS_VALID_FD(store->assert_fd))
        store_fclose(store->assert_fd);

    /* create new log file. */
    store->log_index = 0;
    store->dir_index++;
    store->dir_cnt++;

    alog_store_creat_a_new_file(store, STORE_LOG);
    return AIC_SUCCESS;
}

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
static void alog_store_init_log_bufs(ALOG_STORAGE_T *store)
{
    for (int i = 0; i < STORE_INIT_BUF_CNT; i++) {
#if IS_CONF_ENABLED(CONFIG_STORE_RW_BUFS)
        store->cache[i].buf_ptr = s_stor_bufs + i * STORE_INIT_BUF_SIZE;
#else
        store->cache[i].buf_ptr = aos_malloc(STORE_INIT_BUF_SIZE);
#endif
    }
}

static void alog_store_free_log_bufs(ALOG_STORAGE_T *store)
{
    for (int i = 0; i < STORE_INIT_BUF_CNT; i++) {
        if (store->cache[i].buf_ptr) {
#if IS_CONF_DISABLED(CONFIG_STORE_RW_BUFS)
            aos_free(store->cache[i].buf_ptr);
#endif
            store->cache[i].buf_ptr = NULL;
        }
    }
}
#endif

/*
 * alog_store_save_log. must call in task context.
 */
static void alog_store_init_root_dir(ALOG_STORAGE_T *store)
{
    int ret, loop = 0;

    STORE_DEBUG("alog store init root dir!\n");

#ifdef WIN32
    ret = store_chdir(LOG_WORK_DIR);
    if (ret)
        STORE_ERR("alog store chdir err.\n");
#endif

    /* create root dir. */
    do {
        if (store->sd_unmount || AIC_IODEV_FILE != adbg_get_log_dev()) {
            STORE_ERR("init root return, unmount=%d\n.", store->sd_unmount);
            return;
        }

        ret = store_fexist(STORE_LOG_ROOT_DIR);
        if (ret == AIC_SUCCESS)
            break;

        ret = store_mkdir(STORE_LOG_ROOT_DIR);
        if (ret == AIC_SUCCESS)
            break;

        STORE_ERR("alog store mk dir, ret=%d.\n", ret);

        loop++;
        aos_sleep(1000);
    }while (TRUE);

    /* update the last flush tick. */
    store->last_tick = alog_get_aon_timestamp();

    /* create dir rec file. */
    alog_store_create_dir_rec_file(store);

    /* create the root log dir. */
    ret = alog_store_update_root_dir(store);
    STORE_ASSERT(ret == AIC_SUCCESS);
}

#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
/*
 * alog_store_write_thread.
 */
static void alog_store_write_thread(void *p)
{
    ALOG_STORAGE_T *store = p;
    STORE_ITEM_T *item;

    aicx_info(0x196e35, "", "alog store write thread!\n");

    while (!store->b_stop) {
        (void)aos_get_semaphore(store->read_sem, AOS_WAIT_FOREVER);
        /* not empty. */
        while (store->rdptr != store->wtptr) {
            STORE_DEBUG("store write rdptr=%d, wtptr=%d!\n", store->rdptr, store->wtptr);

            item = &store->cache[GET_RING_POS(store->rdptr)];
            alog_store_write_log(store, item->buf_ptr, item->wt_size);
            item->wt_size = 0;
#if IS_CONF_DISABLED(CONFIG_STORE_INIT_BUFS)
            aos_free(item->buf_ptr);
#endif
            /* update ring index */
            store->rdptr++;
            aos_put_semaphore(store->write_sem);
        }
    }
}
#endif /* CONFIG_STORE_LOG_COPY */

/*
 * alog_store_idle_thread.
 */
static void alog_store_idle_thread(void *p)
{
    ALOG_STORAGE_T *store = p;
#if IS_CONF_ENABLED(CONFIG_WIN32_SIM_FS)
    u16 old_index = 0;
#endif

    STORE_DEBUG("alog store idle thread!\n");

#ifdef WIN32
    alog_store_init_root_dir(store);
#endif

    while (!store->b_stop) {
        (void)aos_get_semaphore(store->idle_sem, AOS_WAIT_FOREVER);

        if (store->b_stop)
            return;

        /* update root dir. */
        if (store->init_root_dir) {
            store->init_root_dir = FALSE;
            alog_store_init_root_dir(store);
        }

        /* check log size and log cnt. */
        if (store->log_size > MAX_STORE_LOG_FILE_SIZE) {
            if (store->log_index > MAX_STORE_LOG_FILE_CNT - 1) {
                alog_store_update_root_dir(store);
            } else {
                alog_store_creat_a_new_file(store, STORE_LOG);
            }
        }

        /* check left space. */
        while (!store->b_stop && AIC_SUCCESS != alog_store_check_fs_left_space(store)) {
            /* pending write log.*/
            STORE_DEBUG("The left space is not enough, pending save log.\n");
            store->write_pending = TRUE;

            /* try later */
            aos_sleep(1000);
            continue;
        }

        /* resume save log. */
        if (store->write_pending) {
            STORE_DEBUG("resume save log.\n");
            store->write_pending = FALSE;
        }

#if IS_CONF_ENABLED(CONFIG_WIN32_SIM_FS)
        if (store->dir_index != old_index) {
            old_index = store->dir_index;

            alog_store_save_assert_info("win32 sim assert.\n");
            alog_store_save_dump("dumpdumpdump\n", 13);
        }
#endif
    }
}

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
void alog_store_flush_init_buf(ALOG_STORAGE_T *store)
{
    STORE_ITEM_T *item = &store->cache[GET_RING_POS(store->wtptr)];

    if (item->wt_size) {
        STORE_DEBUG("store flush rdptr=%d, wtptr=%d!\n", store->rdptr, store->wtptr);

        store->wtptr++;
        aos_put_semaphore(store->read_sem);
    }
}

static void alog_store_init_buf_save_log(ALOG_STORAGE_T *store, void *data, u32 size)
{
    size_t left = MIN(STORE_INIT_BUF_SIZE, size) ;
    STORE_ITEM_T *item;
    bool new_buf_cpy;
    u32 tick;

    item = &store->cache[GET_RING_POS(store->wtptr)];
    if (!item)
        return;

    /* if buf has enough space, just copy it. */
    if (item->wt_size + left <= STORE_INIT_BUF_SIZE) {
        memcpy(item->buf_ptr + item->wt_size, data, left);
        item->wt_size += left;
        tick = alog_get_aon_timestamp();

        /* time < FLUSH_LOG_MAX_TIME or ring full return. */
        if (tick - store->last_tick < FLUSH_LOG_MAX_TIME ||
            IS_RING_FULL(store->rdptr, store->wtptr))
            return;

        /* flush it, not a new buffer, don't copy. */
        new_buf_cpy = FALSE;
    } else {
        /* need get a new buffer to copy it. */
        new_buf_cpy = TRUE;
    }

    /* no free buffer, lost log. */
    while (IS_RING_FULL(store->rdptr, store->wtptr)) {
        store->lost_cnt++;
        STORE_DEBUG("buf is full, wait cnt=%d.\n", store->lost_cnt);
        aos_get_semaphore(store->write_sem, AOS_WAIT_FOREVER);
    }

    /* update write ptr, cpy data and put semaphore. */
    STORE_DEBUG("store save rdptr=%d, wtptr=%d!\n", store->rdptr, store->wtptr);
    store->wtptr++;

    /* update the last flush tick. */
    store->last_tick = alog_get_aon_timestamp();

    if (new_buf_cpy) {
        item = &store->cache[GET_RING_POS(store->wtptr)];
        memcpy(item->buf_ptr, data, left);
        item->wt_size = left;
    }
    aos_put_semaphore(store->read_sem);
}
#elif IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
static void alog_store_cpy_save_log(ALOG_STORAGE_T *store, void *data, u32 size)
{
    STORE_ITEM_T *item;
    void * buf;

    /* ring buf full, lost log. */
    if (IS_RING_FULL(store->rdptr, store->wtptr)) {
        store->lost_cnt++;
        aicx_info(0x0dd5a8, "d", "ring  buf is full, lost cnt=%d.\n", store->lost_cnt);
        return;
    }

    /* alloc and copy */
    buf = aos_get_mem(AOS_COMMON_POOL_ID, size);
    STORE_ASSERT(buf);

    /* no free buffer, lost log. */
    if (!buf) {
        aicx_info(0x192445, "d", "get buf failed, lost cnt=%d.\n", store->lost_cnt);
        store->lost_cnt++;
        return;
    }

    item = &store->cache[GET_RING_POS(store->wtptr)];
    memcpy(buf, data, size);
    item->buf_ptr = buf;
    item->wt_size = size;

    /* update write ptr. */
    store->wtptr++;
    aos_put_semaphore(store->read_sem);
}
#endif

static void alog_store_init_log_dir(ALOG_STORAGE_T *store)
{
    store->init_root_dir = TRUE;
    aos_put_semaphore(store->idle_sem);
}

bool alog_store_is_ready(void)
{
    ALOG_STORAGE_T *store = s_log_store;

    return (store && !store->sd_unmount
#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
            && store->cache[0].buf_ptr
#endif
            );
}

/*
 * alog_store_save_log.
 */
void alog_store_save_log(void *data, u32 size)
{
    ALOG_STORAGE_T *store = s_log_store;

    STORE_ASSERT(store);
    if (!store)
        return;

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
    alog_store_init_buf_save_log(store, data, size);
#elif IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
    alog_store_cpy_save_log(store, data, size);
#else
    alog_store_write_log(store, data, size);
#endif
}

/*
 * alog_store_save_dump.
 */
void alog_store_save_dump(void *data, u32 size, bool finish)
{
    ALOG_STORAGE_T *store = s_log_store;

    STORE_ASSERT(store);
    if (!store || store->sd_unmount)
        return;

    if (STORE_IS_INVALID_FD(store->dump_fd)) {
        /* create a new dump file. */
        alog_store_creat_a_new_file(store, STORE_DUMP);
    }

    STORE_ASSERT(STORE_IS_VALID_FD(store->dump_fd));

    /* dump no need do in write task. */
    alog_store_write(store->dump_fd, data, size, __LINE__);

    if (finish) {
        char txt_finish[20];

        (void)store_fclose(store->dump_fd);
        store->dump_fd = STORE_INVALID_FD;
        snprintf(txt_finish, sizeof(txt_finish),
                 "The %d dump file finish!", store->dump_index);
        alog_store_save_assert_info(txt_finish, 0, TRUE);

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
        alog_store_flush_init_buf(store);
#endif
    }
}

/*
 * alog_store_save_assert_info.
 */
void alog_store_save_assert_info(char *info, size_t size, bool end_z)
{
    ALOG_STORAGE_T *store = s_log_store;
    int len;

    STORE_ASSERT(store);
    if (!store || store->sd_unmount)
        return;

    if (strlen(store->root_dir)) {
        /* save assert info. */
        if (STORE_IS_INVALID_FD(store->assert_fd)) {
            char file_name[NAME_BUF_SIZE];
            AOS_INTERRUPT_DECLARE

            AOS_INTERRUPT_DISABLE
            snprintf(file_name, sizeof(file_name),
                     "%s/cp_assert_stati.log", store->root_dir);
            AOS_INTERRUPT_RESTORE

            store->assert_fd = store_fopen(file_name, F_WTONLY_M);

            STORE_ASSERT(STORE_IS_VALID_FD(store->assert_fd));
            if (STORE_IS_INVALID_FD(store->assert_fd)) {
                aicx_error(0x11d593, "ds", "create assert file failed, fd=%d, file=%s.\n.",
                           store->assert_fd, file_name);
                return;
            }
        }

        if (end_z) {
            len = strlen(info);
            info[len] = '\n';
            size = len;
        }

        alog_store_write(store->assert_fd, info, size, __LINE__);
        store_ffush(store->assert_fd);
    }
}

/*
 * alog_store_init.
 */
void alog_store_init(void)
{
    ALOG_STORAGE_T *store;
    AOS_INTERRUPT_DECLARE

    /* has init, just return. */
    if (s_log_store)
        return;

    STORE_DEBUG("alog store init!\n");

    store = (ALOG_STORAGE_T *)aos_zalloc_init(sizeof(*store));
    AIC_ASSERT(store);

    /* init fd to invalid fd. */
    store->assert_fd = STORE_INVALID_FD;
    store->dump_fd = STORE_INVALID_FD;
    store->log_fd = STORE_INVALID_FD;
    store->wr_log_fd = STORE_INVALID_FD;
    store->dir_rec_fd = STORE_INVALID_FD;
    store->file_rec_fd = STORE_INVALID_FD;

    /* create idle thread, and idle semaphore. */
    store->idle_thread =
        aos_create_thread_nostart(STORE_IDLE_THREAD_NAME,
                                  alog_store_idle_thread, store,
                                  STORE_IDLE_THREAD_STACK,
                                  STORE_IDLE_THREAD_PRI);
    AIC_ASSERT(store->idle_thread != AOS_INVALID_THREAD_ID);
    store->idle_sem = aos_create_semaphore_simply("store_idle");
    AIC_ASSERT(store->idle_sem);

    /* create write thread, and write semaphore. */
#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
    store->write_thread =
        aos_create_thread_nostart(STORE_WRITE_THREAD_NAME,
                                  alog_store_write_thread, store,
                                  STORE_WRITE_THREAD_STACK,
                                  STORE_WRITE_THREAD_PRI);
    AIC_ASSERT(store->write_thread != AOS_INVALID_THREAD_ID);
    store->read_sem = aos_create_semaphore_simply("store_read");
    AIC_ASSERT(store->read_sem);
    store->write_sem = aos_create_semaphore_simply("store_write");
    AIC_ASSERT(store->write_sem);
#endif

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
    alog_store_init_log_bufs(store);
#endif

    AOS_INTERRUPT_DISABLE

    store->sd_unmount = !s_sd_mount;
    s_log_store = store;

    /* if has mounted, start init log dir here, else do it in mount callback. */
    if (s_sd_mount)
        alog_store_init_log_dir(store);

    AOS_INTERRUPT_RESTORE

    /* until here start thread. */
    aos_start_thread(store->idle_thread);
#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
    aos_start_thread(store->write_thread);
#endif
}

/*
 * alog_store_deinit.
 */
void alog_store_deinit(void)
{
    ALOG_STORAGE_T *store = s_log_store;
    if (!store)
        return;

    STORE_DEBUG("alog store deinit!\n");

    /* firstly ,set NULL. */
    s_log_store = NULL;

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
    alog_store_flush_init_buf(store);
#endif

#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
    /* wait empty. */
    while (store->rdptr != store->wtptr) {
        STORE_DEBUG("alog wait rdptr=%d, wtptr=%d!\n", store->rdptr, store->wtptr);
        aos_sleep(100);
    }
#endif

#if IS_CONF_ENABLED(CONFIG_STORE_INIT_BUFS)
    alog_store_free_log_bufs(store);
#endif

    /* close all files. */
    if (STORE_IS_VALID_FD(store->assert_fd))
        store_fclose(store->assert_fd);

    if (STORE_IS_VALID_FD(store->dump_fd))
        store_fclose(store->dump_fd);

    if (STORE_IS_VALID_FD(store->log_fd))
        store_fclose(store->log_fd);

    if (STORE_IS_VALID_FD(store->dir_rec_fd))
        store_fclose(store->dir_rec_fd);

    if (STORE_IS_VALID_FD(store->file_rec_fd))
        store_fclose(store->file_rec_fd);

    /* delete idle idle_thread and idle semaphore. */
    store->b_stop = 1;
    aos_delete_thread(store->idle_thread);
    aos_delete_semaphore(store->idle_sem);

#if IS_CONF_ENABLED(CONFIG_STORE_LOG_COPY)
    aos_delete_thread(store->write_thread);
    aos_delete_semaphore(store->read_sem);
    aos_delete_semaphore(store->write_sem);
#endif

    aos_free(store);
}

static void alog_store_proc_mount(bool mount)
{
    ALOG_STORAGE_T *store = s_log_store;

    s_sd_mount = mount;

    if (store) {
        store->sd_unmount = !mount;

        if (AIC_IODEV_FILE != adbg_get_log_dev())
            return;

        alog_store_init_log_dir(store);
    }
}


void alog_store_fs_mount_cb(struct fs_mount_info *mntinfo, void *param)
{
    STORE_DEBUG("mount cb, volum=%s, mount=%d\n",
              mntinfo->target, mntinfo->mount);

    if (!strstr(mntinfo->target, STORE_LOG_VOLUME))
        return;

    alog_store_proc_mount(mntinfo->mount);
}

void alog_store_pre_init(void)
{
    fs_register_mount_hook(alog_store_fs_mount_cb, NULL);
}

