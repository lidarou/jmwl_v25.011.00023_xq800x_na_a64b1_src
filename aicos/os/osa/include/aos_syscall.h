/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA SYSCALL Interface for WIN32.
 */

#ifndef __AOS_SYSCALL_H__
#define __AOS_SYSCALL_H__

#if 0  /* defined(WIN32) && defined(NET_OSA) */

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <tx_port.h>
/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
#define fclose aos_fclose
#define fopen aos_fopen
#define fwrite aos_fwrite
#define fread aos_fread
#define _lseek aos__lseek
#define remove aos_remove
#define fseek aos_fseek
#define fflush aos_fflush
#define ftell aos_ftell
#undef CreateFile
#define CreateFile aos_CreateFile
#define GetFileSize aos_GetFileSize
#define CloseHandle aos_CloseHandle
#define SetFilePointer aos_SetFilePointer
#define WriteFile aos_WriteFile
#define ReadFile aos_ReadFile
#define SetEndOfFile aos_SetEndOfFile
#undef FindFirstFile
#define FindFirstFile aos_FindFirstFile

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/
__declspec(dllexport) int aos_fclose(FILE *stream);
__declspec(dllexport) FILE * aos_fopen(const char *filename, const char *mode);
__declspec(dllexport) size_t aos_fwrite(const void *buffer, size_t size,
                                        size_t count, FILE *stream);
__declspec(dllexport)  size_t aos_fread(void *buffer, size_t size,
                                        size_t count, FILE *stream);
__declspec(dllexport)  long aos__lseek(int handle, long offset, int origin);
__declspec(dllexport)  int aos_remove(const char *path);
__declspec(dllexport) int aos_fseek(FILE *stream, long offset, int origin);
__declspec(dllexport) int aos_fflush(FILE *stream);
__declspec(dllexport)  long aos_ftell(FILE *stream);
__declspec(dllexport)  HANDLE aos_CreateFile(LPCTSTR, DWORD, DWORD,
                                             LPSECURITY_ATTRIBUTES,
                                             DWORD, DWORD, HANDLE);
__declspec(dllexport)  DWORD aos_GetFileSize(HANDLE hFile,
                                             LPDWORD lpFileSizeHigh);
__declspec(dllexport)  BOOL aos_CloseHandle(HANDLE hObject);
__declspec(dllexport) DWORD aos_SetFilePointer(HANDLE hFile,
                                               LONG lDistanceToMove,
                                               PLONG lpDistanceToMoveHigh,
                                               DWORD dwMoveMethod);
__declspec(dllexport)   BOOL aos_WriteFile(HANDLE, LPCVOID, DWORD,
                                           LPDWORD, LPOVERLAPPED);
__declspec(dllexport)  BOOL aos_ReadFile(HANDLE, LPVOID, DWORD,
                                         LPDWORD, LPOVERLAPPED);
__declspec(dllexport)  BOOL aos_SetEndOfFile(HANDLE hFile);
__declspec(dllexport) HANDLE aos_FindFirstFile(LPCTSTR lpFileName,
                                               LPWIN32_FIND_DATA lpFindFileData);

#define sprintf aos_sprintf
#define printf aos_sys_printf
#define fprintf aos_fprintf
#define vfprintf aos_vfprintf
#define vprintf aos_vprintf
#define vsprintf aos_vsprintf
__declspec(dllexport) int aos_sprintf(char *buffer, const char *format, ...);
__declspec(dllexport) int aos_sys_printf(const char *format, ...);
__declspec(dllexport) int aos_fprintf(FILE *stream, const char *format, ...);
__declspec(dllexport) int aos_vfprintf(FILE *stream, const char *format,
                                       va_list argptr);
__declspec(dllexport) int aos_vprintf(const char *format, va_list argptr);
__declspec(dllexport) int aos_oss_fprintf(FILE *stream, const char *format,...);
__declspec(dllexport) int aos_vsprintf(char *buffer, const char *format,
                                       va_list argptr );

#define malloc aos_sys_malloc
#define free aos_sys_free
#define realloc aos_sys_realloc
__declspec(dllexport) void *aos_sys_malloc(size_t size);
__declspec(dllexport) void aos_sys_free(void *memblock);
__declspec(dllexport) void *aos_sys_realloc(void *memblock, size_t size);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* defined(WIN32) && defined(NET_OSA) */

#endif/* __AOS_SYSCALL_H__ */

