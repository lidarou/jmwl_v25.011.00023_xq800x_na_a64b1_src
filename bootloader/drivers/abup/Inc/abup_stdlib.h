/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_stdlib.h
* Description: 
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef _ABUP_STDLIB_H_
#define _ABUP_STDLIB_H_
#include "abup_typedef.h"
#include "stdarg.h"

#undef NULL
#define NULL 0                   /* see <stddef.h> */
#define ABUP_ISMASK(x) (_ctype[(abup_int)(abup_uint8)(x)])

#define ABUP_ISALNUM(c)	((ABUP_ISMASK(c)&(ABUP_U|ABUP_L|ABUP_D)) != 0)
#define ABUP_ISALPHA(c)	((ABUP_ISMASK(c)&(ABUP_U|ABUP_L)) != 0)
#define ABUP_ISCNTRL(c)	((ABUP_ISMASK(c)&(ABUP_C)) != 0)
#define ABUP_ISGRAPH(c)	((ABUP_ISMASK(c)&(ABUP_P|ABUP_U|ABUP_L|ABUP_D)) != 0)
#define ABUP_ISLOWER(c)	((ABUP_ISMASK(c)&(ABUP_L)) != 0)
#define ABUP_ISPRINT(c)	((ABUP_ISMASK(c)&(ABUP_P|ABUP_U|ABUP_L|ABUP_D|ABUP_SP)) != 0)
#define ABUP_ISPUNCT(c)	((ABUP_ISMASK(c)&(ABUP_P)) != 0)
#define ABUP_ISSPACE(c)	((ABUP_ISMASK(c)&(ABUP_S)) != 0)
#define ABUP_ISUPPER(c)	((ABUP_ISMASK(c)&(ABUP_U)) != 0)
#define ABUP_ISXDIGIT(c)	((ABUP_ISMASK(c)&(ABUP_D|ABUP_X)) != 0)
#define ABUP_ISDIGIT(c)  ((unsigned)((c) - '0') < 10)
#define ABUP_ISASCII(c) (((abup_uint8)(c))<=0x7f)
#define ABUP_TOASCII(c) (((abup_uint8)(c))&0x7f)

#define ABUP_ZEROPAD     (1 << 0)    /* pad with zero */
#define ABUP_SIGN        (1 << 1)    /* unsigned/signed long */
#define ABUP_PLUS        (1 << 2)    /* show ABUP_PLUS */
#define ABUP_SPACE       (1 << 3)    /* ABUP_SPACE if ABUP_PLUS */
#define ABUP_LEFT        (1 << 4)    /* ABUP_LEFT justified */
#define ABUP_SPECIAL     (1 << 5)    /* 0x */
#define ABUP_LARGE       (1 << 6)    /* use 'ABCDEF' instead of 'abcdef' */

#define ABUP_INT_MAX		((abup_int)(~0U>>1))
#define ABUP_INT_MIN		(-ABUP_INT_MAX - 1)
#define ABUP_UINT_MAX	(~0U)
#define ABUP_LONG_MAX	((abup_int)(~0UL>>1))
#define ABUP_LONG_MIN	(-ABUP_LONG_MAX - 1)
#define ABUP_ULONG_MAX	(~0UL)

#define ABUP_U	0x01	/* upper */
#define ABUP_L	0x02	/* lower */
#define ABUP_D	0x04	/* digit */
#define ABUP_C	0x08	/* cntrl */
#define ABUP_P	0x10	/* punct */
#define ABUP_S	0x20	/* white ABUP_SPACE (ABUP_SPACE/lf/tab) */
#define ABUP_X	0x40	/* hex digit */
#define ABUP_SP	0x80	/* hard ABUP_SPACE (0x20) */
#define abup_strcpy_ext(a,b)  abup_memcpy(a,b,abup_strlen((const char *)b))
#define abup_strcmp_ext(a,b)  abup_strncmp(a,b,abup_strlen((const char *)b))

typedef abup_int (*abup_rand)(void);
typedef void (*abup_srand)(abup_uint seed);

abup_extern abup_int abup_vsnprintf(abup_char       *buf,
                        abup_int   datasize,
                        const abup_char *fmt,
                        va_list     args);
abup_extern abup_int abup_snprintf(abup_char * s ,abup_int n,const abup_char * content, ...);

abup_extern void *abup_memmove(void *dest, const void *src, abup_int n);

abup_extern void *abup_memcpy(void * dest, const void * src, abup_int count);
abup_extern void *abup_memcpy1(void * dest,abup_int destlen, const void * src, abup_int srclen);
abup_extern void *abup_memset(void *s, abup_int c, abup_int count);
abup_extern abup_int abup_memcmp(const void *cs, const void *ct, abup_int count);
abup_extern abup_char *abup_strcat(abup_char * dest, const abup_char * src);

abup_extern abup_int abup_strlen(const abup_char *s);
abup_extern abup_int abup_sscanf(const abup_char * buf, const abup_char * fmt, ...);
abup_extern abup_int abup_strncmp(const abup_char *cs, const abup_char *ct, abup_int count);
abup_extern abup_int abup_strcmp(const abup_char * string1, const abup_char * string2 );
abup_extern abup_char *abup_strcpy(abup_char * strDestination, const abup_char * strSource);
abup_extern abup_char *abup_strncpy(abup_char *dst, const abup_char *src, abup_int n);
abup_extern abup_char *abup_strstr(const abup_char * str, const abup_char * sub);
abup_extern abup_uint8 *abup_datadata(const abup_uint8 *buf,abup_int data_len,const abup_uint8 *sub,abup_int sub_len);
abup_extern abup_uint8 *abup_datardata(const abup_uint8 *buf,abup_int data_len,const abup_uint8 *sub,abup_int sub_len);
abup_extern abup_char *abup_strchr(const abup_char *s1, abup_uint8 c);
abup_extern abup_char *abup_strrchr(const abup_char *s1, abup_uint8 c);

abup_extern abup_int abup_atoi(const abup_char *str);
abup_extern abup_long abup_atol(const abup_char *str);
abup_extern abup_char *abup_itoa(abup_int32 num,abup_char* str,abup_int strlen,abup_int base,abup_bool unsignedint);
abup_extern abup_char *abup_get_32bit_dec_tmp(abup_int32 number,abup_bool unsignedint);
abup_extern void abup_memfree(void *p);
abup_extern abup_uint abup_str2uint32(const abup_char * str);
abup_extern void abup_byte2hexstr(const abup_uint8* source, abup_char* dest, abup_int len);
abup_extern void abup_byte2hexstr1(abup_uint8* source, abup_int sourceLen);
abup_extern void abup_hexstr2byte(abup_char* dest, const abup_uint8* source, abup_int sourceLen);
abup_extern void abup_hexstr2byte1(abup_uint8* source, abup_int sourceLen);
abup_extern abup_int abup_c2i(abup_char ch);
abup_extern abup_int abup_hex2dec(abup_char *hex);
abup_extern abup_bool abup_str_del_all(abup_char* str,abup_char *sub,abup_int* len,abup_int sublen);
abup_extern abup_bool abup_str_is_same_char(abup_char* str,abup_int len,abup_char ch);
abup_extern abup_bool AbupIsBigEndian(void);
abup_extern abup_uint64 AbupSW64(abup_uint64 x);
abup_extern abup_uint32 AbupSW32(abup_uint32 x);
abup_extern abup_uint16 AbupSW16(abup_uint16 x);
abup_extern void AbupGetRandStr(abup_char s[],abup_int num,abup_char *str,abup_int str_len,abup_srand srand,abup_rand rand,abup_uint seed);
abup_extern abup_int abup_toupper(abup_int c);
#endif
