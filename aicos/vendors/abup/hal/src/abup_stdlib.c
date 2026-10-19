
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ca6bb
#define LOG_MODULE    AMOD_VENDOR
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "math.h"
#include "abup_typedef.h"
#include "abup_stdlib.h"

//for time stamp
#define  ABUP_DT_UTC_BASE_YEAR  1970
#define  ABUP_DT_MONTH_PER_YEAR    12
#define  ABUP_DT_DAY_PER_YEAR         365
#define  ABUP_DT_SEC_PER_DAY    86400
#define  ABUP_DT_SEC_PER_HOUR      3600
#define  ABUP_DT_SEC_PER_MIN    60
const abup_uint8 s_abup_dt_day_per_mon[ABUP_DT_MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static abup_char abup_32bit_tmp[32];

#define PNULL 0

const abup_uint8 _ctype[] = {
    ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,            /* 0-7 */
    ABUP_C,ABUP_C|ABUP_S,ABUP_C|ABUP_S,ABUP_C|ABUP_S,ABUP_C|ABUP_S,ABUP_C|ABUP_S,ABUP_C,ABUP_C,        /* 8-15 */
    ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,            /* 16-23 */
    ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,ABUP_C,            /* 24-31 */
    ABUP_S|ABUP_SP,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,            /* 32-39 */
    ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,            /* 40-47 */
    ABUP_D,ABUP_D,ABUP_D,ABUP_D,ABUP_D,ABUP_D,ABUP_D,ABUP_D,            /* 48-55 */
    ABUP_D,ABUP_D,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,            /* 56-63 */
    ABUP_P,ABUP_U|ABUP_X,ABUP_U|ABUP_X,ABUP_U|ABUP_X,ABUP_U|ABUP_X,ABUP_U|ABUP_X,ABUP_U|ABUP_X,ABUP_U,    /* 64-71 */
    ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,            /* 72-79 */
    ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,            /* 80-87 */
    ABUP_U,ABUP_U,ABUP_U,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,            /* 88-95 */
    ABUP_P,ABUP_L|ABUP_X,ABUP_L|ABUP_X,ABUP_L|ABUP_X,ABUP_L|ABUP_X,ABUP_L|ABUP_X,ABUP_L|ABUP_X,ABUP_L,    /* 96-103 */
    ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,            /* 104-111 */
    ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,            /* 112-119 */
    ABUP_L,ABUP_L,ABUP_L,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_C,            /* 120-127 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 128-143 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 144-159 */
    ABUP_S|ABUP_SP,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,   /* 160-175 */
    ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,ABUP_P,       /* 176-191 */
    ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,       /* 192-207 */
    ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_P,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_U,ABUP_L,       /* 208-223 */
    ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,       /* 224-239 */
    ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_P,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L,ABUP_L      /* 240-255 */
};

abup_int32 abup_sprintf(abup_char * buffer, const abup_char *format, ...)
{
    abup_int32 iReturn;
    va_list Args;
    va_start(Args,format);
    iReturn = vsprintf(buffer,format,Args);
    va_end(Args);
    return iReturn;
}


abup_int32 abup_snprintf(abup_char * buffer, abup_int32 size,const abup_char *format, ...)
{
    abup_int32 iReturn;
    va_list Args;
    va_start(Args,format);
#ifndef WIN32
    iReturn = vsnprintf(buffer,size,format,Args);
#else
    iReturn = _vsnprintf(buffer,size,format,Args);
#endif
    va_end(Args);
    return iReturn;
}


void * abup_memcpy(void * dest, const void * src, abup_uint32 count)
{
   if((dest!=NULL)&&(src!=NULL)&&(count!=0))
    return memcpy(dest,src,count);
   else
       return NULL;
}

void * abup_memset(void * dest, char c, abup_uint32 count )
{
   if((dest!=NULL)&&(count!=0))
    return memset(dest,c,count);
   else
       return NULL;
}

abup_int32 abup_memcmp(void * buf1, void * buf2, abup_uint32 count)
{
   if((buf1!=NULL)&&(buf2!=NULL)&&(count!=0))
    return memcmp((const void *)buf1,(const void *)buf2,count);
   else
       return 0;
}


abup_char * abup_strcat(abup_char * strDestination, const abup_char * strSource )
{
   if((strDestination!=NULL)&&(strSource!=NULL))
    return strcat(strDestination,strSource);
   else
       return NULL;
}


abup_char * abup_strncat(abup_char * strDestination, const abup_char * strSource,abup_uint32 len)
{
   if((strDestination!=NULL)&&(strSource!=NULL)&&(len!=0))
    return strncat(strDestination,strSource,len);
   else
       return NULL;
}


abup_uint32 abup_strlen(const abup_char * str)
{
   if(str!=NULL)
    return (abup_uint32)strlen(str);
   else
       return 0;
}

abup_int32 abup_strncmp(const abup_char * string1, abup_char * string2, abup_uint32 count)
{
    return strncmp(string1,string2,count);

}

abup_int32 abup_strcmp(const abup_char * string1, const abup_char * string2 )
{
    return strcmp(string1,string2);
}

abup_int32 abup_wstrcmp(const abup_wchar * string1, const abup_wchar * string2)
{
    abup_int32 ret = 0 ;

    if( string1 == PNULL && string2 == PNULL )
    {
        return 0;
    }
    else if( string1 == PNULL && string2 != PNULL )
    {
        return -1;
    }
    else if( string1 != PNULL && string2 == PNULL )
    {
        return 1;
    }
    else
    {
/*lint -save -e613 */
        ret = *(abup_wchar *)string1 - *(abup_wchar *)string2;/*lint !e613 */

        while( (!ret) && *string2 && *string1)/*lint !e613 */
        {
            ++string1;/*lint !e613 */
            ++string2;
            ret = *(abup_wchar *)string1 - *(abup_wchar *)string2;
        }
/*lint -restore */
        return ( ret > 0 ? 1 : ( ret < 0 ? -1 : 0 ) );
    }
}


abup_char * abup_strcpy(abup_char * strDestination, const abup_char * strSource)
{
   if((strDestination!=NULL)&&(strSource!=NULL))
    return strcpy(strDestination, strSource);
   else
       return NULL;
}

abup_char * abup_strncpy(abup_char * strDest, const abup_char * strSource, abup_uint32 count)
{
   if((strDest!=NULL)&&(strSource!=NULL)&&(count!=0))
    return strncpy(strDest,strSource,count);
   else
       return NULL;
}


abup_char * abup_strstr(const abup_char * str, const abup_char * strSearch)
{
   if(str!=NULL)
    return strstr(str,strSearch);
   else
       return NULL;
}

abup_char * abup_strchr(const abup_char * str, abup_int32 ch)
{
   if(str!=NULL)
    return strchr(str,ch);
   else
       return NULL;
}



void abup_itoa(abup_uint32 i, abup_char *buf, abup_uint8 base)
{
    abup_char *s;
    #define ABUP_ITOA_LEN    20
    abup_int32 rem;
    static abup_char rev[ABUP_ITOA_LEN+1];

    rev[ABUP_ITOA_LEN] = 0;
    if (i == 0)
        {
        buf[0] = '0';
        ++(buf);
        return;
        }
    s = &rev[ABUP_ITOA_LEN];
    while (i)
        {
        rem = i % base;
        if (rem < 10)
            *--s = rem + '0';
        else if (base == 16)
            *--s = "abcdef"[rem - 10];
        i /= base;
        }
    while (*s)
        {
        buf[0] = *s++;
        ++(buf);
        }
}

abup_uint32 abup_atoi(abup_char * str)
{
     if(str!=NULL)
        return (abup_uint32)atoi(str);
    else
        return 0;
}


static abup_uint8 abup_dt_is_leap_year(abup_uint16 year)
{
    if ((year % 400) == 0)
    {
        return 1;
    }
    else if ((year % 100) == 0)
    {
        return 0;
    }
    else if ((year % 4) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static abup_uint8 abup_dt_last_day_of_mon(abup_uint8 month, abup_uint16 year)
{
    if ((month == 0) || (month > 12))
    {
        return s_abup_dt_day_per_mon[1] + abup_dt_is_leap_year(year);
      }

    if (month != 2)
    {
        return s_abup_dt_day_per_mon[month - 1];
    }
    else
    {
        return s_abup_dt_day_per_mon[1] + abup_dt_is_leap_year(year);
    }
}

abup_uint32 abup_dt_curtime2utcsec(abup_time_struct *currTime)
{
    abup_uint16 i;
    abup_uint32 no_of_days = 0;
    abup_uint32 utc_time;

    if (!currTime || (currTime->nYear<ABUP_DT_UTC_BASE_YEAR))
    {
        return 0;
    }

    /* year */
    for (i = ABUP_DT_UTC_BASE_YEAR; i < currTime->nYear; i++)
    {
        no_of_days += (365 + abup_dt_is_leap_year(i));
    }

    /* month */
    for (i = 1; i < currTime->nMonth; i++)
    {
        no_of_days += abup_dt_last_day_of_mon((abup_uint8) i, currTime->nYear);
    }

    /* day */
    no_of_days += (currTime->nDay - 1);

    /* sec */
    utc_time = (abup_uint32) no_of_days *ABUP_DT_SEC_PER_DAY + \
                    (abup_uint32) (currTime->nHour * ABUP_DT_SEC_PER_HOUR +
                     currTime->nMin * ABUP_DT_SEC_PER_MIN + currTime->nSec);

    if(currTime->nYear < 2002)
        utc_time += 0x30000000;

    return utc_time;
}
abup_weak ABUP_BOOL AbupIsBigEndian(void)
{
    abup_int16 a = 0x1122;//十六进制，一个数值占4位
    abup_char b =  *(abup_char *)&a;  //通过将short(2字节)强制类型转换成char单字节，b指向a的起始字节（低字节）
    if( b == 0x11)//低字节存的是数据的高字节数据
    {
        //是大端模式
        return ABUP_TRUE;
    }
    else
    {
        //是小端模式
        return ABUP_FALSE;
    }
}

abup_weak abup_uint32 AbupSW32(abup_uint32 x)
{
    union test {
        abup_uint32 a;
        abup_uint8 b[4];
    } y;
    abup_uint8 i = 0;
    abup_uint8 tmp = 0;
    y.a=x;
    for(i=0; i<2; i++)
    {
        tmp = y.b[i];
        y.b[i] = y.b[3-i];
        y.b[3-i] = tmp;
    }
    return y.a;
}

abup_weak abup_int32 divide(abup_int32 *n, abup_int32 base)
{
    abup_int32 res;

    /* optimized for processor which does not support divide instructions. */
    if (base == 10)
    {
        res = ((abup_uint32) * n) % 10U;
        *n = ((abup_uint32) * n) / 10U;
    }
    else
    {
        res = ((abup_uint32) * n) % 16U;
        *n = ((abup_uint32) * n) / 16U;
    }

    return res;
}

abup_weak abup_char *abup_get_32bit_dec_tmp(abup_int32 number,ABUP_BOOL unsignedint)
{
    abup_memset(abup_32bit_tmp,0,32);
    if(unsignedint)
    {
        abup_snprintf(abup_32bit_tmp,32,"%u",number);
    }
    else
    {
        abup_snprintf(abup_32bit_tmp,32,"%d",number);
    }
    return abup_32bit_tmp;
}

abup_int32 abup_toupper(abup_int32 c)
{
    if (ABUP_ISLOWER(c))
    {
        return c -= 'a'-'A';
    }
    else
    {
        return c;
    }
}

abup_weak void abup_hexstr2byte(abup_char* dest, const abup_uint8* source, abup_int32 sourceLen)
{
    abup_int16 i;
    abup_uint8 highByte, lowByte;

    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = abup_toupper(source[i]);
        lowByte  = abup_toupper(source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i>>1] = (highByte << 4) | lowByte;
    }
    return ;
}



