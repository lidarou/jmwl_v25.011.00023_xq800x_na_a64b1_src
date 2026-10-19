/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file realize the AIC OSA SYSCALL function for WIN32.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x110eae
#define LOG_MODULE    AMOD_AOS

#if 0/* #if defined(WIN32) && defined(NET_OSA) */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x110eae

#include <aic_osa.h>
#include <aos_syscall.h>

aos_mutex_ptr g_aos_syscall_mutex = AOS_NULL;
boolean g_aos_syscall_flag = AOS_FALSE;

aos_status aos_syscall_init()
{
    g_aos_syscall_mutex = aos_create_mutex("syscall_mutex");
    AOS_ASSERT(g_aos_syscall_mutex);

    g_aos_syscall_flag = AOS_TRUE;
    return AOS_SUCCESS;
}

aos_status aos_syscall_exit()
{
    g_aos_syscall_flag = AOS_FALSE;
    return AOS_SUCCESS;
}

/*****************************************************************/
__declspec(dllexport) int aos_fclose(FILE *stream)
{
/* #undef fclose */
    int result;

    if (!g_aos_syscall_flag)
        return fclose(stream);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = fclose(stream);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport) FILE * aos_fopen(const char *filename, const char *mode)
{
/* #undef fopen */
    FILE *pf;

    if (!g_aos_syscall_flag)
        return fopen(filename, mode);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    pf = fopen(filename, mode);
    aos_put_mutex(g_aos_syscall_mutex);
    return pf;
}

__declspec(dllexport) size_t aos_fwrite(const void *buffer, size_t size,
                                        size_t count, FILE *stream)
{
/* #undef fwrite */
    size_t result;

    if (!g_aos_syscall_flag)
        return fwrite(buffer, size, count, stream);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = fwrite(buffer, size, count, stream);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}
__declspec(dllexport)  size_t aos_fread(void *buffer, size_t size,
                                        size_t count, FILE *stream)
{
/* #undef fread */
    size_t result;

    if (!g_aos_syscall_flag)
        return fread(buffer, size, count, stream);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = fread(buffer, size, count, stream);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}

__declspec(dllexport)  long aos__lseek(int handle, long offset, int origin)
{
/* #undef _lseek */
    long resulr;

    if (!g_aos_syscall_flag)
        return _lseek(handle, offset, origin);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    resulr = _lseek(handle, offset, origin);
    aos_put_mutex(g_aos_syscall_mutex);
    return resulr;
}

__declspec(dllexport)  int aos_remove(const char *path)
{
/* #undef remove */
    int result;

    if (!g_aos_syscall_flag)
        return remove(path );

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = remove(path);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}

__declspec(dllexport) int aos_fseek(FILE *stream, long offset, int origin)
{
/* #undef fseek */
    int result;

    if (!g_aos_syscall_flag)
        return fseek(stream,  offset, origin);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = fseek(stream,  offset, origin);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}

__declspec(dllexport) int aos_fflush(FILE *stream)
{
/* #undef fflush */
    int result;

    if (!g_aos_syscall_flag)
        return fflush(stream);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = fflush(stream);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}

__declspec(dllexport)  long aos_ftell(FILE *stream)
{
/* #undef ftell */
    long result;

    if (!g_aos_syscall_flag)
        return ftell(stream);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = ftell(stream);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport)  HANDLE aos_CreateFile(
  LPCTSTR lpFileName,                         /* file name */
  DWORD dwDesiredAccess,                      /* access mode */
  DWORD dwShareMode,                          /* share mode */
  LPSECURITY_ATTRIBUTES lpSecurityAttributes, /* SD */
  DWORD dwCreationDisposition,                /* how to create */
  DWORD dwFlagsAndAttributes,                 /* file attributes */
  HANDLE hTemplateFile                        /* handle to template file */
  )
{
/* #undef CreateFile */
    long result;

    if (!g_aos_syscall_flag)
        return CreateFile(lpFileName, dwDesiredAccess, dwShareMode,
                          lpSecurityAttributes, dwCreationDisposition,
                          dwFlagsAndAttributes, hTemplateFile);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = CreateFile(lpFileName, dwDesiredAccess, dwShareMode,
                        lpSecurityAttributes, dwCreationDisposition,
                        dwFlagsAndAttributes, hTemplateFile);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport)  DWORD aos_GetFileSize(
  HANDLE hFile,           /* handle to file */
  LPDWORD lpFileSizeHigh  /* high-order word of file size */
)
{
/* #undef GetFileSize */
    long result;

    if (!g_aos_syscall_flag)
        return GetFileSize(hFile, lpFileSizeHigh);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = GetFileSize(hFile, lpFileSizeHigh);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}


__declspec(dllexport)  boolean aos_CloseHandle(
  HANDLE hObject   /* handle to object */
)
{
/* #undef CloseHandle */
    long result;

    if (!g_aos_syscall_flag)
        return CloseHandle(hObject);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = CloseHandle(hObject);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport)  DWORD aos_SetFilePointer(
  HANDLE hFile,                /* handle to file */
  LONG lDistanceToMove,        /* bytes to move pointer */
  PLONG lpDistanceToMoveHigh,  /* bytes to move pointer */
  DWORD dwMoveMethod           /* starting point */
)
{
/* #undef SetFilePointer */
    long result;

    if (!g_aos_syscall_flag)
        return SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh,
                              dwMoveMethod);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh,
                            dwMoveMethod);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport)   boolean aos_WriteFile(
  HANDLE hFile,                    /* handle to file */
  LPCVOID lpBuffer,                /* data buffer */
  DWORD nNumberOfBytesToWrite,     /* number of bytes to write */
  LPDWORD lpNumberOfBytesWritten,  /* number of bytes written */
  LPOVERLAPPED lpOverlapped        /* overlapped buffer */
)
{
/*#undef WriteFile */
    long result;

    if (!g_aos_syscall_flag)
        return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite,
                         lpNumberOfBytesWritten, lpOverlapped);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite,
                       lpNumberOfBytesWritten, lpOverlapped);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport)  boolean aos_ReadFile(
  HANDLE hFile,                /* handle to file */
  LPVOID lpBuffer,             /* data buffer */
  DWORD nNumberOfBytesToRead,  /* number of bytes to read */
  LPDWORD lpNumberOfBytesRead, /* number of bytes read */
  LPOVERLAPPED lpOverlapped    /* overlapped buffer */
)
{
/* #undef ReadFile */
    long result;

    if (!g_aos_syscall_flag)
        return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                        lpNumberOfBytesRead, lpOverlapped);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                      lpNumberOfBytesRead, lpOverlapped);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport)  boolean aos_SetEndOfFile(
  HANDLE hFile   /* handle to file */
)
{
/* #undef SetEndOfFile */
    long result;

    if (!g_aos_syscall_flag)
        return SetEndOfFile(hFile);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = SetEndOfFile(hFile);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

__declspec(dllexport) HANDLE aos_FindFirstFile(LPCTSTR lpFileName,
                                               LPWIN32_FIND_DATA lpFindFileData)
{
    long result;

    if (!g_aos_syscall_flag)
        return FindFirstFile(lpFileName, lpFindFileData);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = FindFirstFile(lpFileName, lpFindFileData);
    aos_put_mutex(g_aos_syscall_mutex);

    return result;
}

/*****************************************************************/

__declspec(dllexport)  int aos_sprintf(char *buffer, const char *format,...)
{
/* #undef sprintf */
    va_list sa;
    int len;

    if (!g_aos_syscall_flag) {
        va_start(sa,format);
        len = _vsnprintf((char *)&buffer[0],(size_t)250, (char *)format, sa);
        va_end(sa);
        return len;
    }

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);

    va_start(sa,format);
    len = _vsnprintf((char *)&buffer[0],(size_t)250, (char *)format, sa);
    va_end(sa);

    aos_put_mutex(g_aos_syscall_mutex);

    return len;
}

__declspec(dllexport) int aos_sys_printf(const char *format ,...)
{
/* #undef printf */
    va_list sa;
    u8 str_buff[255];
    int len;

    if (!g_aos_syscall_flag)
    {
        va_start(sa,format);
        len = _vsnprintf((char *)&str_buff[0],(size_t)(sizeof(str_buff)-1),
                         (char *)format,sa);
        va_end(sa);

        str_buff[254] = '\0';

        printf("%s\n", &str_buff[0]);

        return len;
    }

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);

    va_start(sa,format);
    len = _vsnprintf((char *)&str_buff[0],(size_t)(sizeof(str_buff)-1),
                     (char *)format,sa);
    va_end(sa);

    str_buff[254] = '\0';
    printf("%s\n", &str_buff[0]);
    aos_put_mutex(g_aos_syscall_mutex);

    return len;
}


__declspec(dllexport) int aos_oss_fprintf(FILE *stream, const char *format ,...)
{
/* #undef fprintf */
    va_list sa;
    u8 str_buff[255];
    int len;

    if (!g_aos_syscall_flag) {
        va_start(sa,format);
        len = _vsnprintf((char *)&str_buff[0],(size_t)(sizeof(str_buff)-1),
                         (char *)format,sa);
        va_end(sa);

        str_buff[254] = '\0';
        fprintf(stdout, "%s", &str_buff[0]);

        return len;
    }

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);

    va_start(sa,format);
    len = _vsnprintf((char *)&str_buff[0],(size_t)(sizeof(str_buff)-1),
                     (char *)format,sa);
    va_end(sa);

    str_buff[254] = '\0';

    fprintf(stdout, "%s", &str_buff[0]);

    aos_put_mutex(g_aos_syscall_mutex);

    return len;

}

__declspec(dllexport) int aos_fprintf(FILE *stream, const char *format ,...)
{
/* #undef fprintf */
    va_list sa;
    u8 str_buff[255];
    int len;

    if (!g_aos_syscall_flag) {
        va_start(sa,format);
        len = _vsnprintf((char *)&str_buff[0],(size_t)(sizeof(str_buff)-1),
                         (char *)format,sa);
        va_end(sa);

        str_buff[254] = '\0';

        fprintf(stream, "%s\r\n", &str_buff[0]);

        return len;
    }

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);

    va_start(sa,format);
    len = _vsnprintf((char *)&str_buff[0],(size_t)(sizeof(str_buff)-1),
                     (char *)format,sa);
    va_end(sa);

    str_buff[254] = '\0';

    fprintf(stream, "%s\r\n", &str_buff[0]);

    aos_put_mutex(g_aos_syscall_mutex);

    return len;
}

__declspec(dllexport)  int aos_vfprintf(FILE *stream, const char *format,
                                        va_list argptr)
{
/* #undef vfprintf */
    int result;

    if (!g_aos_syscall_flag)
        return vfprintf(stream, format, argptr);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = vfprintf(stream, format, argptr);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}

__declspec(dllexport) int aos_vprintf(const char *format, va_list argptr)
{
/* #undef vprintf */

    int result;

    if (!g_aos_syscall_flag)
        return vprintf(format, argptr);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = vprintf(format, argptr);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}

__declspec(dllexport)  int aos_vsprintf(char *buffer, const char *format,
                                        va_list argptr)
{
/* #undef vsprintf */
    int result;

    if (!g_aos_syscall_flag)
        return vsprintf(buffer, format, argptr);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    result = vsprintf(buffer, format, argptr);
    aos_put_mutex(g_aos_syscall_mutex);
    return result;
}


/*****************************************************************/
__declspec(dllexport) void *aos_sys_malloc(size_t size)
{
/* #undef malloc */
    void *p;

    if (!g_aos_syscall_flag)
        return aos_malloc(size);

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    p = aos_malloc(size);
    aos_put_mutex(g_aos_syscall_mutex);

    return p;
}

__declspec(dllexport)  void aos_sys_free(void *memblock)
{
/* #undef free */

    if (!g_aos_syscall_flag) {
        aos_free(memblock);
        return ;
    }

    aos_get_mutex(g_aos_syscall_mutex, AOS_WAIT_FOREVER);
    aos_free(memblock);
    aos_put_mutex(g_aos_syscall_mutex);
}

__declspec(dllexport)  void *aos_sys_realloc(void *memblock, size_t size)
{
/* #undef realloc */

    AOS_ASSERT(AOS_FALSE);
    return AOS_NULL;
}

#endif/* defined(WIN32) && defined(NET_OSA) */

