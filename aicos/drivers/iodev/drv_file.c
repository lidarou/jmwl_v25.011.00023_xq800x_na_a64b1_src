/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    temp use drv_usart.c befor file driver ready.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1df214
#define LOG_MODULE    AMOD_DRV

#include <drv_file.h>

#ifdef CONFIG_FILE_HAL_DRIVE
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <stdio.h>

#ifdef WIN32
#include <errno.h>
#include <direct.h>
#include <io.h>
#include <Windows.h>
#endif

#ifdef CONFIG_FMD
typedef struct {
    u32 handle;
    u32 rd_offset;
    u32 wt_offset;
} fmd_handle;
#endif

void *file_open(char *filename, char *mode)
{
#ifdef CONFIG_FMD
    fmd_handle *fh;
    int ret;

    fh = (fmd_handle *)aos_zalloc(sizeof(*fh));
    AIC_ASSERT(fh);
    ret = fmd_open(filename, &fh->handle);

    if (ret) {
        aos_free(fh);
        aicx_error(0x0b5a47, "sd", "open %s err=%d!", filename, ret);
        return NULL;
    }

    return (void*)fh;
#endif

#ifdef WIN32
    FILE *file;

    file = fopen(filename, mode);
    return file;
#endif

    return NULL;
}

void file_close(void *handle)
{
#ifdef CONFIG_FMD
    fmd_handle *fh = (fmd_handle *)handle;
    (void)fmd_close(fh->handle);
    aos_free(fh);
#endif

#ifdef WIN32
    fclose((FILE *)handle);
#endif
}

size_t file_read(void *handle, void *buffer, size_t size)
{
#ifdef CONFIG_FMD
    fmd_handle *fh = (fmd_handle *)handle;
    u32 partion_size = 0;
    int ret;

    ret = fmd_get_partition_size(fh->handle, &partion_size);
    AIC_ASSERT(ret == AIC_SUCCESS);

    if (size > partion_size - fh->rd_offset)
        size = partion_size - fh->rd_offset;

    ret = fmd_read(fh->handle, fh->rd_offset, size, (u8 *)buffer);
    if (ret)
        aicx_error(0x0ba569, "xd", "read handle(0x%x) err=%d!", handle, ret);
    else
        fh->rd_offset += size;

    return size;
#endif

#ifdef WIN32
    FILE *file = (FILE *)handle;

    return (size_t)fread(buffer, 1, size, file);
#endif

    return 0;
}

size_t file_write(void *handle, const void *buffer, size_t size)
{
    size_t w_size = 0;

#ifdef CONFIG_FMD
    fmd_handle *fh = (fmd_handle *)handle;
    int ret = fmd_write(fh->handle, fh->wt_offset, size, (u8 *)buffer);
    if (ret)
        aicx_error(0x01200c, "xd", "write handle(0x%x) err=%d!", handle, ret);
    else
        fh->wt_offset += size;

    return size;
#endif

#ifdef WIN32
    FILE *file = (FILE *)handle;

    w_size = fwrite(buffer, 1, size, file);
    fflush(file);
    //AIC_ASSERT(w_size != size);
#endif

    return (int)w_size;
}

int file_access(const char *filename, int mode)
{
#ifdef WIN32
    return _access(filename, _A_NORMAL);
#endif
    return 0;
}

int file_mkdir(const char *path)
{
#ifdef WIN32
    return _mkdir(path);
#endif
    return 0;
}


int file_seek(void *handle, u32 offset)
{

#ifdef CONFIG_FMD
    fmd_handle *fh = (fmd_handle *)handle;
    fh->rd_offset = offset;
#endif

#ifdef WIN32
    FILE *file = (FILE *)handle;

    return fseek(file, offset, 0);
#endif
    return 0;
}

void *file_get_mem_addr(void *handle, u32 offset)
{
#ifdef CONFIG_FMD
    fmd_handle *fh = (fmd_handle *)handle;
    u32 addr;
    int ret;

    ret = fmd_get_physical_address(fh->handle, offset, &addr);
    AIC_ASSERT(!ret);

    return (void *)addr;
#endif

    return NULL;
}

#ifdef WIN32
#define empty_proces_one_find() do {\
    if (fileinfo.name[0] != '.') \
    { \
        if ((fileinfo.attrib & _A_SUBDIR) != 0) \
        { \
            /* process directory */ \
            _chdir(fileinfo.name); \
            empty_directory(); \
            _chdir(".."); \
            _rmdir(fileinfo.name); \
        } else { \
            /* process file */ \
            remove(fileinfo.name); \
        } \
    } \
} while (0)

void empty_directory(void)
{
    struct _finddata_t fileinfo;
    intptr_t iFind;
    memset(&fileinfo, 0x0, sizeof(fileinfo));

    iFind = _findfirst("*", &fileinfo);
    if (iFind == -1)
    {
       aicx_error(0x1a94bc, "", "error: _findfirst failed\n.");
       return;
    }

    /* process the first item */
    empty_proces_one_find();

    while (_findnext(iFind, &fileinfo) == 0)
    {
        /* process one item */
        empty_proces_one_find();
    }

    _findclose(iFind);
}

#endif

int file_rm_all_dir(const char *path)
{
#ifdef WIN32
    int ret;
    char old_path[200 + 1];

    (void)GetCurrentDirectoryA(200, old_path);
    ret = _chdir(path);
    empty_directory();
    ret += _chdir(old_path);
    ret += _rmdir(path);
    return ret;
#endif
    return 0;
}

int file_get_free(int drv, u64 *free_bytes, u64 *total_bytes)
{
#ifdef WIN32
    struct _diskfree_t df;
    int ret;

    if (CUR_DRV == drv)
        drv = _getdrive();

    ret = _getdiskfree(drv, &df);
    if (ret) {
        aicx_error(0x142933, "dd", "get _getdiskfree err, drv=%d, ret=%d\n.", drv, ret);
        return ret;
    }

    if (free_bytes)
        *free_bytes = df.avail_clusters * df.sectors_per_cluster * df.bytes_per_sector;
    if (total_bytes)
        *total_bytes = df.total_clusters * df.sectors_per_cluster * df.bytes_per_sector;

    return 0;
#else
    *free_bytes = 0;
    *total_bytes = 0;
    return AIC_ENOTSUP;
#endif
}
#endif /*CONFIG_FILE_HAL_DRIVE */

