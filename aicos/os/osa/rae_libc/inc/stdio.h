/* stdio.h */

/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>
#include <stddef.h>
#include <sys/reent.h>
#include <sys/_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(EOF)
#define EOF  (-1)
#endif

#if !defined(__FILE_defined)
typedef __FILE FILE;
# define __FILE_defined
#endif

typedef _fpos_t fpos_t;
#ifdef __LARGE64_FILES
typedef _fpos64_t fpos64_t;
#endif

#ifndef _OFF_T_DECLARED
typedef __off_t off_t;
#define	_OFF_T_DECLARED
#endif

#ifndef _SSIZE_T_DECLARED
typedef _ssize_t ssize_t;
#define	_SSIZE_T_DECLARED
#endif

#include <sys/stdio.h>

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#define	TMP_MAX		26

#define stdin  ((FILE *) 1)
#define stdout ((FILE *) 2)
#define stderr ((FILE *) 3)

#define _stdin_r(x)	((x)->_stdin)
#define _stdout_r(x)	((x)->_stdout)
#define _stderr_r(x)	((x)->_stderr)

#define __printf_like(f, a) __attribute__((format(printf, f, a)))

int __printf_like(1, 2) printf(const char *__restrict format, ...);
int __printf_like(3, 4) snprintf(char *__restrict str, size_t len,
                                 const char *__restrict format, ...);
int __printf_like(2, 3) sprintf(char *__restrict str,
                                const char *__restrict format, ...);
int __printf_like(2, 3) fprintf(FILE * __restrict stream,
                                const char *__restrict format, ...);


int __printf_like(1, 0) vprintf(const char *__restrict format,
                                va_list list);
int __printf_like(3, 0) vsnprintf(char *__restrict str, size_t len,
                                  const char *__restrict format,
                                  va_list list);
int __printf_like(2, 0) vsprintf(char *__restrict str,
                                 const char *__restrict format,
                                 va_list list);
int __printf_like(2, 0) vfprintf(FILE * __restrict stream,
                                 const char *__restrict format,
                                 va_list ap);

int sscanf(const char *buf, const char *fmt, ...);

int puts(const char *s);

int fputc(int c, FILE *stream);
int fputs(const char *__restrict s, FILE *__restrict stream);
static inline int putc(int c, FILE *stream)
{
    return fputc(c, stream);
}
static inline int putchar(int c)
{
    return putc(c, stdout);
}

int fclose (FILE *__restrict);
int fflush (FILE *__restrict);
int fgetc (FILE *__restrict);
char *fgets (char *__restrict, int, FILE *__restrict);
size_t fread (void *__restrict, size_t _size, size_t _n, FILE *__restrict);
size_t fwrite (const void *__restrict , size_t _size, size_t _n, FILE *);
int fgetpos (FILE *__restrict, fpos_t *__restrict);
int fseek (FILE *__restrict, long, int);
int fsetpos (FILE *__restrict, const fpos_t *__restrict);
long ftell ( FILE *__restrict);
void rewind (FILE *__restrict);
int feof (FILE *__restrict);
int ferror (FILE *__restrict);
FILE *fopen (const char *__restrict _name, const char *__restrict _type);
int rename (const char *__restrict, const char *__restrict);

#ifdef __cplusplus
}
#endif

#endif /* __STDIO_H__ */
