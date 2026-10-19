
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CharsetTable.h"
#include "charset.h"

typedef struct _char_set_ref {
    chset_enum      chset_id;
    unsigned char   chset_name_count;
    const char * *  chset_name_array;
} char_set_ref;


typedef struct {
    const Mapping_Struct * pMappingTable;
    const unsigned short * pConversnTable;
    unsigned short  TotalMapTbIndex;
    unsigned char   Input_word_bits;
    unsigned char   Output_word_bits;
    unsigned short  UndefChar;                      /* currently taken undefchar as quesmark */
} chset_info_struct;


#define CAST_PU8(RAW)           ((unsigned char*)(RAW))
#define CAST_CPU8(RAW)          ((const unsigned char*)(RAW))
#define GET_U8_BYTE_TO_U16(RAW,i) ((unsigned short)(CAST_CPU8(RAW)[i]))
#define STR_SIZE(len)           ((len)<<1)
#define STR_AT(RAW,n)           ((void*)(CAST_PU8(RAW)+STR_SIZE(n)))
#define CHR_AT(RAW,n)           ((unsigned short)(GET_U8_BYTE_TO_U16(RAW, STR_SIZE(n))+(GET_U8_BYTE_TO_U16(RAW, STR_SIZE(n)+1)<<8)))
#define SET_CHR_AT(RAW,n,value) do {                                    \
        unsigned char *_p = STR_AT(RAW, n);                             \
        unsigned short v= (unsigned short) (value);                     \
        _p[0] = (unsigned char) (v & 0xff);                             \
        _p[1] = (unsigned char) (v >> 8);                               \
        } while (0)

#define CHR(x)                  ((unsigned short)(x))

#define MERGE_TO_WCHAR(high,low) (high << 8) + ((unsigned char)low)
#define UTF16_HIGH_START        0xD800
#define UTF16_HIGH_END          0xDBFF
#define UTF16_LOW_START         0xDC00
#define UTF16_LOW_END           0xDFFF

#define CHSET_MAX_COUNT         25
#define CHSET_BIT_WORD_16       16
#define CHSET_BIT_WORD_8        8
#define CHSET_TMP_BUFF_SIZE     2048        /* the size of the temp buffer in mmi_chset_convert() */

const key_index_t * g_chset_ucs2_to_encode_key_LSB_index[NO_OF_TEXT_ENCODING] =
{
#ifdef __MMI_CHSET_BIG5__
    g_chset_ucs2_to_big5_key_LSB_index,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_GB2312__
    g_chset_ucs2_to_gb2312_key_LSB_index,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_HKSCS__
    g_chset_ucs2_to_hkscs_key_LSB_index

#else

    NULL
#endif
};


const key_index_t * g_chset_encode_to_ucs2_key_LSB_index[NO_OF_TEXT_ENCODING] =
{
#ifdef __MMI_CHSET_BIG5__
    g_chset_big5_to_ucs2_key_LSB_index,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_GB2312__
    g_chset_gb2312_to_ucs2_key_LSB_index,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_HKSCS__
    g_chset_hkscs_to_ucs2_key_LSB_index

#else

    NULL
#endif
};


const unsigned char * g_chset_ucs2_to_encode_key_MSB[NO_OF_TEXT_ENCODING] =
{
#ifdef __MMI_CHSET_BIG5__
    g_chset_ucs2_to_big5_key_MSB,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_GB2312__
    g_chset_ucs2_to_gb2312_key_MSB,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_HKSCS__
    g_chset_ucs2_to_hkscs_key_MSB

#else

    NULL
#endif
};


const unsigned char * g_chset_encode_to_ucs2_key_MSB[NO_OF_TEXT_ENCODING] =
{
#ifdef __MMI_CHSET_BIG5__
    g_chset_big5_to_ucs2_key_MSB,

#else

    NULL,
#endif

    NULL,                                           //new struct for gb2312 MSB table

#ifdef __MMI_CHSET_HKSCS__
    g_chset_hkscs_to_ucs2_key_MSB

#else

    NULL
#endif
};


const unsigned short * g_chset_ucs2_to_encode_table[NO_OF_TEXT_ENCODING] =
{
#ifdef __MMI_CHSET_BIG5__
    g_chset_ucs2_to_big5_table,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_GB2312__
    g_chset_ucs2_to_gb2312_table,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_HKSCS__
    g_chset_ucs2_to_hkscs_table

#else

    NULL
#endif
};


const unsigned short * g_chset_encode_to_ucs2_table[NO_OF_TEXT_ENCODING] =
{
#ifdef __MMI_CHSET_BIG5__
    g_chset_big5_to_ucs2_table,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_GB2312__
    g_chset_gb2312_to_ucs2_table,

#else

    NULL,
#endif

#ifdef __MMI_CHSET_HKSCS__
    g_chset_hkscs_to_ucs2_table

#else

    NULL
#endif
};


const unsigned short g_chset_unknown_encode_char_no_space[] =
{
    0xFFFF,
    0xFFFF,
    0xFFFF
};


const unsigned short g_chset_unknown_encode_char_space[] =
{
    0x20,
    0x20,
    0x20
};


const unsigned short * g_chset_unknown_encode_char = g_chset_unknown_encode_char_no_space;

static short    g_chset_tbl_is_init;
static chset_info_struct * g_chset_tbl[CHSET_PAIR_TOTAL] =
{
    0
};


/* 
 * UTF-8
 */
#define CHSET_UTF_8
static const unsigned char g_cheset_utf8_bytes_per_char[16] =
{
    1,1,1,1,1,1,1,1,
    0,0,0,0,2,2,3,4                                 /* we don't support UCS4 */
};


static unsigned short chset_is_utf8_BOM(const unsigned char * str)
{
    if (str[0] == 0xEF && str[1] == 0xBB && str[2] == 0xBF) {
        return 1;
        }
    else {
        return 0;
        }
}


static unsigned int chset_ucs2_to_utf8_len(unsigned short ucs2)
{
    if (ucs2 < (unsigned short) 0x80) {
        return 1;
        }
    else if (ucs2 < (unsigned short) 0x800) {
        return 2;
        }
    else {
        return 3;
        }
}


int chset_utf8_strlen(const unsigned char * s)
{
    int             count = 0;
    int             rawlen;
    int             d;

    if (s == NULL) {
        return 0;
        }

    rawlen              = strlen((const char *) s);

    if (rawlen >= 3 && chset_is_utf8_BOM(s)) {
        s                   += 3;
        rawlen              -= 3;
        }

    while (*s != '\0' && rawlen > 0) {
        d                   = g_cheset_utf8_bytes_per_char[*s >> 4];

        if (d == 0) {
            return count;
            }

        if (d > rawlen) {
            return count;
            }

        rawlen              -= d;
        s                   += d;

        count++;
        }

    return count;
}


int hex2dec(char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
        }
    else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
        }
    else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
        }
    else {
        return - 1;
        }
}


void url_decode(char buf[])
{
    int             i   = 0;
    int             len = strlen(buf);
    int             res_len = 0;
    char            res[50];

    for (i = 0; i < len; ++i) {
        char            c   = buf[i];

        if (c != '%') {
            res[res_len++]      = c;
            }
        else {
            char            c1  = buf[++i];
            char            c0  = buf[++i];
            int             num = 0;

            num                 = hex2dec(c1) * 16 + hex2dec(c0);
            res[res_len++]      = num;
            }
        }

    res[res_len]        = '\0';
    strcpy(buf,res);
}


unsigned char app_unicode_to_ucs2encoding(unsigned short unicode,unsigned char * charLength,unsigned char * arrOut)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned char   status = 1;
    unsigned char   index = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (arrOut != 0) {

        if (unicode < 256) {
            arrOut[index++]     = * ((unsigned char *) (&unicode));
            arrOut[index]       = 0;

            }
        else {
            arrOut[index++]     = * ((unsigned char *) (&unicode));
            arrOut[index]       = * (((unsigned char *) (&unicode)) + 1);

            }

        *charLength         = 2;
        }
    else {

        status              = 0;
        }


    return status;
}


unsigned short int app_asc_str_to_ucs2_str(signed char * pOutBuffer,signed char * pInBuffer)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    signed short int count = -1;
    unsigned char   charLen = 0;
    unsigned char   arrOut[2];

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    while (*pInBuffer != '\0') {

        app_unicode_to_ucs2encoding((unsigned short) (* ((unsigned char *) pInBuffer)),&charLen,arrOut);

        // #ifdef MMI_ON_WIN32
        pOutBuffer[++count] = arrOut[0];
        pOutBuffer[++count] = arrOut[1];
        pInBuffer++;

        // #endif
#ifdef __FOR_TESTING /* MMI_ON_HARDWARE_P */
        pOutBuffer[++count] = arrOut[1];            /* arrOut[0]; */
        pOutBuffer[++count] = arrOut[0];            /* arrOut[1]; */
        pInBuffer++;
#endif /* __FOR_TESTING */

        }

    pOutBuffer[++count] = '\0';
    pOutBuffer[++count] = '\0';
    return count + 1;
}


int mmi_asc_to_ucs2(signed char * pOutBuffer,signed char * pInBuffer)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    /* On the Target */
    return (int) app_asc_str_to_ucs2_str((signed char *) pOutBuffer,(signed char *) pInBuffer);
}


int chset_utf8_to_ucs2(unsigned char * dest,unsigned char * utf8)
{
    unsigned char   c   = utf8[0];
    int             count = g_cheset_utf8_bytes_per_char[c >> 4];
    unsigned short  ucs2 = 0xFFFF;                  /* invalid character */

    switch (count)
    {
        case 1:
            ucs2 = (unsigned short) c;
            break;

        case 2:
            if (utf8[1]) {
                ucs2                = ((unsigned short) (c & 0x1f) << 6) | (unsigned short) (utf8[1] ^0x80);
                }

            break;

        case 3:
            if (utf8[1] && utf8[2]) {
                ucs2                =
                     ((unsigned short) (c & 0x0f) << 12) | ((unsigned short) (utf8[1] ^0x80) << 6) | (unsigned short) (utf8[2] ^0x80);
                }

            break;

        case 4:
            break;

        default:
            count = 1; /* the character cannot be converted, return 1 means always consume 1 byte */
            break;
        }

    SET_CHR_AT(dest,0,ucs2);

    return count;
}


int chset_ucs2_to_utf8(unsigned char * utf8,unsigned short ucs2)
{
    int             count;

    if (ucs2 < (unsigned short) 0x80) {
        count               = 1;
        }
    else if (ucs2 < (unsigned short) 0x800) {
        count               = 2;
        }
    else {
        count               = 3;
        }

    switch (count)
    {
        case 3:
            utf8[2] = 0x80 | (ucs2 & 0x3f);
            ucs2 = ucs2 >> 6;
            ucs2 |= 0x800;
        case 2:
            utf8[1] = 0x80 | (ucs2 & 0x3f);
            ucs2 = ucs2 >> 6;
            ucs2 |= 0xc0;
        case 1:
            utf8[0] = (unsigned char) ucs2;
        }

    return count;
}


int chset_utf8_to_ucs2_string_ex(unsigned char * dest,int dest_size,unsigned char * src,unsigned int * src_end_pos)
{
    int             pos = 0;
    int             cnt;
    int             src_len = strlen((const char *) src);

    if (src_len >= 3 && chset_is_utf8_BOM(src)) {
        src                 += 3;
        src_len             -= 3;
        }

    dest_size           -= (dest_size % 2);
    *src_end_pos        = (unsigned int) src;       /* set src_end_pos first */

    if (dest_size < 2) {
        return 0;
        }

    while (*src && pos < dest_size - 2) {
        cnt                 = chset_utf8_to_ucs2(dest + pos,src);

        if ((int) (src - (*src_end_pos)) >= (int) (src_len - cnt) && (* (dest + pos) == 0xFF &&
             * (dest + pos + 1) == 0xFF) && ! (*src == 0xEF && * (src + 1) == 0xBF && * (src + 2) == 0xBF)) {
            //If src isn't 0xEF, 0xBF, 0xBF and its remain length is not enough but dest is 0xFFFF, we will abort the process.
            // dest data is invalid character because src data is not enough to convert 
            break;
            }

        if (cnt == 0) /* abnormal case */
        {
            break;
            }
        else /* normal case */
        {
            src                 += cnt;
            pos                 += 2;
            }
        }

    *src_end_pos        = (unsigned int) src;
    dest[pos]           = 0;
    dest[pos + 1]       = 0;
    return pos + 2;
}


int YxAppUTF8ToUincode(unsigned char * inbuf,int lenin,unsigned char * outbuf,int lenout)
{
    int             i   = 0,j = 0,tempv = 0;

    if ((outbuf) && (!lenout))
        return 0;

    while (i < lenin) {
        if ((i + 1 < lenin) && ((inbuf[i] &0xE0) == 0xC0) && ((inbuf[i + 1] &0xC0) == 0x80)) //two bytes
        {
            if (outbuf) {
                if (j < lenout) {
                    unsigned char   tkv = (inbuf[i + 1] &0x3F) << 2;

                    tempv               = ((inbuf[i] &0x1F) << 6) | tkv;
                    outbuf[j++]         = (unsigned char) (tempv);
                    outbuf[j++]         = (unsigned char) (tempv >> 8);
                    }
                else 
                    return j; //not convert finish
                }
            else 
                j += 2;

            i                   += 1;
            }
        else if ((i + 2 < lenin) && ((inbuf[i] &0xF0) == 0xE0) && ((inbuf[i + 1] &0xC0) == 0x80) &&
             ((inbuf[i + 2] &0xC0) == 0x80)) {
            if (outbuf) {
                if (j < lenout) {
                    unsigned char   hbyte = (inbuf[i] << 4) | ((((inbuf[i + 1] &0x3F) << 2) & 0xF0) >> 4),

                    lbye                = (((((inbuf[i + 1] &0x3F) << 2) & 0x0F) >> 2) << 6) | (inbuf[i + 2] &0x3F);

                    tempv               = (hbyte << 8) | lbye;
                    outbuf[j++]         = (unsigned char) (tempv);
                    outbuf[j++]         = (unsigned char) (tempv >> 8);
                    }
                else 
                    return j; //not convert finish
                }
            else 
                j += 2;

            i                   += 2;
            }
        else if ((inbuf[i] &0x80) == 0x00) {
            if (inbuf[i] == 0)
                break;

            if (outbuf) {
                if (j < lenout) {
                    tempv               = inbuf[i] &0x7F;
                    outbuf[j++]         = (unsigned char) (tempv);
                    outbuf[j++]         = (unsigned char) (tempv >> 8);
                    }
                else 
                    return j; //not convert finish
                }
            else 
                j += 2;
            }

        i++;
        }

    if ((outbuf) && (j + 2 <= lenout)) {
        i                   = j;
        outbuf[j++]         = 0x00;
        outbuf[j++]         = 0x00;
        return i;
        }

    return j;
}


int chset_utf8_to_ucs2_string(unsigned char * dest,int dest_size,unsigned char * src)
{
    return YxAppUTF8ToUincode(src,dest_size,dest,dest_size);

    //return chset_utf8_to_ucs2_string_ex(dest, dest_size, src, &src_end_pos);
}


int chset_ucs2_to_utf8_string_ex(unsigned char * dest,int dest_size,unsigned char * src,unsigned int * src_end_pos)
{
    int             pos = 0;

    while (src[0] || src[1]) {
        if (pos + chset_ucs2_to_utf8_len(CHR_AT(src,0)) >= dest_size) /* leave space of '\0' */
        {
            break;
            }

        pos                 += chset_ucs2_to_utf8(dest + pos,CHR_AT(src,0));
        src                 += 2;

        if (pos >= dest_size - 1) {
            break;
            }
        }

    if (pos >= dest_size) {
        dest[dest_size - 1] = 0;
        }
    else {
        dest[pos]           = 0;
        }

    *src_end_pos        = (unsigned int) src;
    return pos + 1;
}


int chset_ucs2_to_utf8_string(unsigned char * dest,int dest_size,unsigned char * src)
{
    unsigned int    src_end_pos = (unsigned int) src;

    return chset_ucs2_to_utf8_string_ex(dest,dest_size,src,&src_end_pos);
}


int chset_ucs2_to_utf8_length_in_bytes(const unsigned char * raw)
{
    int             len = (dchar_strlen((const char *) raw)) / 2;
    int             utf8_len = 0;
    int             i;

    for (i = 0; i < len; i++) {
        utf8_len            += chset_ucs2_to_utf8_len(CHR_AT(raw,i));
        }

    return utf8_len;
}


int chset_utf8_to_ucs2_length_in_bytes(const unsigned char * raw)
{
    return STR_SIZE(chset_utf8_strlen(raw));
}


char chset_get_utf_byte_order(const char * str_bom)
{
    unsigned short  wc;

    wc                  = MERGE_TO_WCHAR(str_bom[0],str_bom[1]);

    switch (wc)
    {
        case 0xFFFE:
            return CHSET_UTF16LE;

        case 0xFEFF:
            return CHSET_UTF16BE;

        case 0xEFBB:
            if ((char) 0xBF == str_bom[2])
                return CHSET_UTF8;

            else 
                return - 1;

        default:
            return - 1;
        }
}


static int chset_ucs2_to_utf16_string(unsigned char * dest_utf16,int dest_size,chset_enum utf16_type,
    int include_bom,
    unsigned char * src_ucs2,unsigned int * src_end_pos)
{
    int             count = 0;
    unsigned short  wc;

    if (include_bom) {
        if (CHSET_UTF16BE == utf16_type) {
            dest_utf16[0]       = (unsigned char) 0xFF;
            dest_utf16[1]       = (unsigned char) 0xFE;
            }
        else {
            dest_utf16[0]       = (unsigned char) 0xFE;
            dest_utf16[1]       = (unsigned char) 0xFF;
            }

        dest_utf16          += 2;
        dest_size           -= 2;
        count               += 2;
        }

    while ((src_ucs2[0] || src_ucs2[1]) && (count < (dest_size - 2))) {
        wc                  = MERGE_TO_WCHAR(src_ucs2[1],src_ucs2[0]);

        /* UTF-16 surrogate values are illegal in UCS2; 
           0xffff or 0xfffe are both reserved values */
        if (! (wc >= UTF16_HIGH_START && wc <= UTF16_LOW_END)) {
            if (CHSET_UTF16LE == utf16_type) {
                dest_utf16[0]       = src_ucs2[0];
                dest_utf16[1]       = src_ucs2[1];
                }
            else {
                dest_utf16[0]       = src_ucs2[1];
                dest_utf16[1]       = src_ucs2[0];
                }

            dest_utf16          += 2;
            count               += 2;
            }

        src_ucs2            += 2;
        }

    dest_utf16[0]       = '\0';
    dest_utf16[1]       = '\0';

    *src_end_pos        = (unsigned int) src_ucs2;
    return (count + 2);

}


static int chset_utf16_to_ucs2_string(unsigned char * dest_ucs2,int dest_size,unsigned char * src_utf16,
    chset_enum utf16_type,unsigned int * src_end_pos)
{
    char            utf16_bo;
    int             count = 0;
    short           wc1;

    utf16_bo            = chset_get_utf_byte_order((char *) src_utf16);

    if (-1 != utf16_bo) {
        src_utf16           += 2;

        /* needn't convert the beginning BOM */
        }
    else {
        utf16_bo            = utf16_type;
        }

    while ((src_utf16[0] || src_utf16[1]) && (count < (dest_size - 2))) {
        wc1                 = ((CHSET_UTF16BE ==
             utf16_bo) ? MERGE_TO_WCHAR(src_utf16[0],src_utf16[1]): MERGE_TO_WCHAR(src_utf16[1],src_utf16[0]));

        /* we ignore surrogate pair */
        if (((wc1 >= UTF16_HIGH_START) && (wc1 <= UTF16_HIGH_END)) || ((wc1 >= UTF16_LOW_START) &&
             (wc1 <= UTF16_LOW_END))) {
            src_utf16           += 2;
            }
        else {
            /* normal case */
            dest_ucs2[0]        = wc1 & 0xFF;
            dest_ucs2[1]        = wc1 >> 8;
            src_utf16           += 2;
            dest_ucs2           += 2;
            count               += 2;
            }
        }

    dest_ucs2[0]        = '\0';
    dest_ucs2[1]        = '\0';

    *src_end_pos        = (unsigned int) src_utf16;
    return (count + 2);

}


#define BASE64_MAXINDEX         64
static const char base64_data[BASE64_MAXINDEX] =
{
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W',
    'X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w',
    'x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};


static unsigned short direct_chars_ucs2_to_utf(unsigned short number)
{
    if (number > 127) {
        return 0;
        }
    else {
        return 1;
        }
}


static short get_char_index_in_base64array(char char_to_search)
{
    if (char_to_search == '+') {
        return 62;
        }
    else if (char_to_search == '/') {
        return 63;
        }
    else if (char_to_search >= '0' && char_to_search <= '9') {
        return 52 + (char_to_search - '0');
        }
    else if (char_to_search >= 'a' && char_to_search <= 'z') {
        return 26 + (char_to_search - 'a');
        }
    else if (char_to_search >= 'A' && char_to_search <= 'Z') {
        return char_to_search - 'A';
        }
    else {
        return 0;
        }
}


void chset_utf_ucs2_chars(unsigned char * output_chars,unsigned char * current_input_chars,
    unsigned char prev_input_char,int combination_no,int * output_len)
{
    unsigned short  final_char = 0,index = 0,ctr = 0;
    unsigned char   temp_char,prev_bits,current_bits,prev_char = get_char_index_in_base64array(prev_input_char);

    for (ctr = 0; ctr < 3; ctr++) {
        if (current_input_chars[ctr] == 0) {
            current_input_chars[ctr] = 'A';
            }

        index               = get_char_index_in_base64array(current_input_chars[ctr]);

        /*
         * Every UTF7 char would be transfered to 6 bits according "Modified Base 64"
         * Every Unicode is 16 bits (2 bytes)
         * Three UTF7 chars will encode one Unicode in worst case.
         * There are three cases:
         * Case 1: (No previous char)
         *     Need 3 UTF7 chars encode 1 Unicode, and remain 2 bits
         *     - combination_no: 0
         * Case 2: (Previous char remains 2 bits, and still need 3 UTF7 chars)
         *     Need more 3 UTF7 chars to encode 1 Unicode, and remain 4 bits
         *     - combination_no: 1
         * Case 3: (Previous char remains 4 bits, and only need 2 UTF7 chars)
         *     Need more 2 UTF7 chars to encode 1 Unicode. (no remained bits)
         *     - combination_no: 2
         */
        switch (combination_no)
        {
            case 0:
                final_char = final_char | ((unsigned short) (index << 10) >> (6 * ctr));
                break;

            case 1:
                prev_bits = prev_char & 0x03;
                current_bits = (index & 0x3C) >> 2;
                temp_char = (prev_bits << 4) | current_bits;
                final_char = final_char | ((unsigned short) (temp_char << 10) >> (6 * ctr));
                prev_char = index;
                break;

            case 2:
                prev_bits = prev_char & 0x0F;
                current_bits = (index & 0x30) >> 4;
                temp_char = (prev_bits << 2) | current_bits;
                final_char = final_char | ((unsigned short) (temp_char << 10) >> (6 * ctr));
                prev_char = index;
                break;
            }
        }

    memcpy(output_chars + (*output_len),&final_char,sizeof(short));
    *output_len         += 2;
}


int chset_utf7_to_ucs2_str(unsigned char * g_unicode_decoded_buf,unsigned char * g_encoded_buf,int data_size,
    unsigned int * src_end_pos)
{
    int             ctr = 0,output_len = 0,temp_ctr = 0,no_decoded_combinations = 0;
    unsigned char   current_encoded_buf[3] = {
        0
        },
    prev_last_char      = 0,ch;
    int             g_plus_indicator_flag = 0;

    while (g_encoded_buf[ctr] != 0 && output_len < data_size - 2) {
        ch                  = g_encoded_buf[ctr];

        /*
         * When the char is '+', begin to decode UTF7 according "Modified Base 64" (REC 2045).
         * When the char is '-', end of decoding UTF7 according "Modified Base 64".
         */
        if ((ch != '+') && (g_plus_indicator_flag == 0)) {
            /* the octet value should be less than 0x7F */
            g_unicode_decoded_buf[output_len++] = ch;
            g_unicode_decoded_buf[output_len++] = 0;
            }
        else {
            char temp_buff      = 0;                /* For taking care for the unicode + character */

            if ((ch != '-') && (g_plus_indicator_flag == 1)) {
                current_encoded_buf[temp_ctr++] = ch;
                }

            if ((temp_ctr == 3 && no_decoded_combinations != 2) || (temp_ctr == 2 && no_decoded_combinations == 2)) {
                /*
                 * Every UTF7 char would be transfered to 6 bits according "Modified Base 64"
                 * Every Unicode is 16 bits (2 bytes)
                 * Three UTF7 chars will encode one Unicode in worst case.
                 * There are three cases:
                 * Case 1: (No previous char)
                 *     Need 3 UTF7 chars encode 1 Unicode, and remain 2 bits
                 *     - no_decoded_combinations: 0
                 * Case 2: (Previous char remains 2 bits, and still need 3 UTF7 chars)
                 *     Need more 3 UTF7 chars to encode 1 Unicode, and remain 4 bits
                 *     - no_decoded_combinations: 1
                 * Case 3: (Previous char remains 4 bits, and only need 2 UTF7 chars)
                 *     Need more 2 UTF7 chars to encode 1 Unicode. (no remained bits)
                 *     - no_decoded_combinations: 2
                 */
                chset_utf_ucs2_chars(g_unicode_decoded_buf,
                    current_encoded_buf,
                    prev_last_char,
                    no_decoded_combinations,
                    &output_len);
                no_decoded_combinations++;
                prev_last_char      = (no_decoded_combinations) ? current_encoded_buf[2]: 0;

                if (no_decoded_combinations == 3) {
                    no_decoded_combinations = 0;
                    }

                current_encoded_buf[0] = 0;
                current_encoded_buf[1] = 0;
                current_encoded_buf[2] = 0;
                temp_ctr            = 0;
                }

            g_plus_indicator_flag = 1;

            if ((ch == '-') && (temp_ctr != 0)) {
                chset_utf_ucs2_chars(g_unicode_decoded_buf,
                    current_encoded_buf,
                    prev_last_char,
                    no_decoded_combinations % 3,
                    &output_len);

                /* handle this condition in decoder where chars less then 3 */
                current_encoded_buf[0] = 0;
                current_encoded_buf[1] = 0;
                current_encoded_buf[2] = 0;

                prev_last_char      = 0;
                no_decoded_combinations = 0;
                g_plus_indicator_flag = 0;
                temp_ctr            = 0;
                }
            else if ((ch == '-') && (temp_ctr == 0)) {
                current_encoded_buf[0] = 0;
                current_encoded_buf[1] = 0;
                current_encoded_buf[2] = 0;

                prev_last_char      = 0;
                no_decoded_combinations = 0;
                g_plus_indicator_flag = 0;
                temp_ctr            = 0;
                }

            temp_buff           = g_encoded_buf[ctr + 1];

            if (ch == '+' && temp_buff == '-') /* Fot taking care for the unicode '+' character */
            {
                g_unicode_decoded_buf[output_len++] = ch;
                g_unicode_decoded_buf[output_len++] = 0;
                }
            }

        ctr++;
        }

    *src_end_pos        = (unsigned int) (&g_encoded_buf[ctr]);

    g_unicode_decoded_buf[output_len] = 0;
    g_unicode_decoded_buf[output_len + 1] = 0;
    return (2 * (output_len + 1));

}


void chset_ucs2_utf_chars(unsigned char * output_chars,int * output_len,unsigned short current_char,unsigned short prev_char,int no_of_chars,int encode_flag,int data_size)
{
    if (encode_flag) {
        /*
         * Encode UTF7 chars from Unicode
         * Every Unicode is 16 bits (2 bytes)
         * Every UTF7 char would be transfered from 6 bits according "Modified Base 64"
         * One Unicode will encode three UTF7 chars in worst case (lacked bits will be 0).
         * There are three cases:
         * Case 1: (No previous Unicode)
         *     1 Unicode encodes 2 UTF7 chars, and remains 4 bits
         *     - no_of_chars: 0
         * Case 2: (Previous Unicode remains 4 bits)
         *     Previous Unicode(4 bits) and 1 Unicode encode 3 UTF7 chars, and remain 2 bits
         *     - no_of_chars: 1
         * Case 3: (Previous Unicode remains 2 bits)
         *     Previous Unicode(2 bits) and 1 Unicode encode 3 UTF7 chars, and no remained bits
         *     - no_of_chars: 2
         */
        switch (no_of_chars)
        {
            case 0:
                if (*output_len < data_size - 1) {
                    output_chars[(*output_len) ++] = base64_data[(current_char & 0xFC00) >> 10];
                    output_chars[(*output_len) ++] = base64_data[(current_char & 0x03F0) >> 4];
                    }

                break;

            case 1:
                if (*output_len < data_size - 2) {
                    output_chars[(*output_len) ++] =
                         base64_data[((prev_char & 0x000F) << 2) | ((current_char & 0xC000) >> 14)];
                    output_chars[(*output_len) ++] = base64_data[(current_char & 0x3F00) >> 8];
                    output_chars[(*output_len) ++] = base64_data[(current_char & 0x00FC) >> 2];
                    }

                break;

            case 2:
                if (*output_len < data_size - 2) {
                    output_chars[(*output_len) ++] =
                         base64_data[((prev_char & 0x0003) << 4) | ((current_char & 0xF000) >> 12)];
                    output_chars[(*output_len) ++] = base64_data[(current_char & 0x0FC0) >> 6];
                    output_chars[(*output_len) ++] = base64_data[(current_char & 0x003F)];
                    }

                break;
            }
        }
    else {
        /*
         * Encode UTF7 chars with padding 0 bits
         * Every Unicode is 16 bits (2 bytes)
         * Every UTF7 char would be transfered from 6 bits according "Modified Base 64"
         * One Unicode will encode three UTF7 chars in worst case (lacked bits will be 0).
         * There are three cases:
         * Case 1: (Previous Unicode remains 4 bits)
         *     Previous Unicode(4 bits) encodes UTF7 chars with 2 padding bits(0).
         *     - no_of_chars: 1
         * Case 2: (Previous Unicode remains 2 bits)
         *     Previous Unicode(2 bits) encodes UTF7 chars with 4 padding bits(0).
         *     - no_of_chars: 2
         */
        switch (no_of_chars)
        {
            case 1:
                if (*output_len < data_size) {
                    output_chars[(*output_len) ++] = base64_data[((prev_char & 0x000F) << 2)];
                    }

                break;

            case 2:
                if (*output_len < data_size) {
                    output_chars[(*output_len) ++] = base64_data[((prev_char & 0x0003) << 4)];
                    }

                break;
            }
        }
}


int chset_ucs2_to_utf7_str(unsigned char * g_encoded_buf,unsigned char * g_data_buf,int data_size,unsigned int * src_end_pos)
{
    unsigned char temp_char[3];
    unsigned short * current_char,prev_char = 0;
    int output_len      = 0,ucs2_chars_count = 0,ctr = 0,encode_flag = 0;

    data_size           -= 2;
    prev_char           = 0;

    while (! (g_data_buf[ctr] == 0 && g_data_buf[ctr + 1] == 0)) /* ctr < len * 2 ) */
    {
        temp_char[0]        = g_data_buf[ctr];
        temp_char[1]        = g_data_buf[ctr + 1];
        temp_char[2]        = 0;

        current_char        = (unsigned short *) temp_char;

        if (temp_char[0] == '+') {
            if (encode_flag == 1) {
                encode_flag         = 0;

                if (ucs2_chars_count) {
                    chset_ucs2_utf_chars(g_encoded_buf,
                        &output_len,
                        *current_char,
                        prev_char,
                        ucs2_chars_count,
                        encode_flag,
                        data_size);
                    }

                g_encoded_buf[output_len++] = '-';
                }

            if (output_len < data_size - 1) {
                g_encoded_buf[output_len++] = '+';
                g_encoded_buf[output_len++] = '-';
                }
            else {
                break;
                }

            ucs2_chars_count    = 0;
            }
        else if (direct_chars_ucs2_to_utf(*current_char)) {
            /*
             * If Unicode value is less than 0x7F, 
             * leaving "Modified Base 64" encoding and encode char directly.
             */
            if (encode_flag == 1) {
                encode_flag         = 0;

                if (ucs2_chars_count) {
                    chset_ucs2_utf_chars(g_encoded_buf,
                        &output_len,
                        *current_char,
                        prev_char,
                        ucs2_chars_count,
                        encode_flag,
                        data_size);
                    }

                g_encoded_buf[output_len++] = '-';
                }

            if (output_len < data_size) {
                g_encoded_buf[output_len++] = (unsigned char) (*current_char);
                }
            else {
                break;
                }

            ucs2_chars_count    = 0;
            }
        else {
            /*
             * If Unicode value is larger than 0x7F, 
             * entering "Modified Base 64" encoding.
             */
            if (encode_flag == 0) {
                g_encoded_buf[output_len++] = '+';
                encode_flag         = 1;
                }

            /*
             * Encode UTF7 chars from Unicode
             * Every Unicode is 16 bits (2 bytes)
             * Every UTF7 char would be transfered from 6 bits according "Modified Base 64"
             * One Unicode will encode three UTF7 chars in worst case (padding bits in 0).
             * There are three cases:
             * Case 1: (No previous Unicode)
             *     1 Unicode encodes 2 UTF7 chars, and remains 4 bits
             *     - ucs2_chars_count: 0
             * Case 2: (Previous Unicode remains 4 bits)
             *     Previous Unicode(4 bits) and 1 Unicode encode 3 UTF7 chars, and remain 2 bits
             *     - ucs2_chars_count: 1
             * Case 3: (Previous Unicode remains 2 bits)
             *     Previous Unicode(2 bits) and 1 Unicode encode 3 UTF7 chars, and no remained bits
             *     - ucs2_chars_count: 2
             */
            chset_ucs2_utf_chars(g_encoded_buf,
                &output_len,
                *current_char,
                prev_char,
                ucs2_chars_count,
                encode_flag,
                data_size);
            ucs2_chars_count++;

            if (ucs2_chars_count == 3) {
                ucs2_chars_count    = 0;
                }
            }

        ctr                 += 2;
        prev_char           = *current_char;
        }

    *src_end_pos        = (unsigned int) (&g_data_buf[ctr]);

    g_encoded_buf[output_len] = 0;
    return output_len + 1;
}


/* 
 * dual 8bits string operations
 */
#define CHSET_DUAL_8BIT
int dchar_strlen(const char * wstr)
{
    unsigned short * tmp = (unsigned short *) wstr;
    int len             = 0;

    if (tmp != NULL) {
        while (*tmp != 0) {
            tmp++;
            len++;
            }
        }

    return len;
}


int dchar_strcpy(char * des,int des_len,const char * src)
{
    unsigned short * t_des = (unsigned short *) des;
    unsigned short * t_src = (unsigned short *) src;
    int len             = 0;
    unsigned short tmp  = 0;

    des_len--;

    if (t_des == NULL || t_src == NULL || des_len <= 0) {
        return 0;
        }

    while (des_len > 0) {
        tmp                 = *t_des = *t_src;

        if (tmp == 0) {
            break;
            }

        t_des++;
        t_src++;
        len++;
        des_len--;
        }

    if ((des_len == 0) && (tmp != 0)) {
        *t_des              = 0;
        }

    return len;
}


int dchar_strncpy(char * des,int des_len,const char * src,int size)
{
    unsigned short * t_des = (unsigned short *) des;
    unsigned short * t_src = (unsigned short *) src;
    int len             = 0;
    unsigned short tmp  = 0;

    des_len--;

    if (t_des == NULL || t_src == NULL || des_len <= 0 || size <= 0) {
        return 0;
        }

    while (des_len > 0 && size > 0) {
        tmp                 = *t_des = *t_src;

        if (*t_src == 0) {
            break;
            }

        t_des++;
        t_src++;
        len++;

        des_len--;
        size--;
        }

    if (((des_len == 0) || (size == 0)) && (tmp != 0)) {
        *t_des              = 0;
        }

    return len;
}


int dchar_strcat(char * des,int des_len,const char * src)
{
    int len             = 0;

    if (des == NULL || des_len <= 0) {
        return 0;
        }

    len                 = dchar_strlen(des);

    if (src != NULL) {
        len                 += dchar_strcpy(des + len * 2,des_len - len,src);
        }

    return len;
}


int dchar_strcmp(const char * des,const char * src)
{
    unsigned short * tmp1 = (unsigned short *) des;
    unsigned short * tmp2 = (unsigned short *) src;
    int ret             = 0;

    if (tmp1 != NULL && tmp2 != NULL) {
        while (1) {
            ret                 = *tmp1 - *tmp2;

            if ((ret != 0) || (*tmp1 == 0) || (*tmp2 == 0)) {
                break;
                }

            tmp1++;
            tmp2++;
            }
        }

    return ret;
}


int dchar_strncmp(const char * des,const char * src,int size)
{
    unsigned short * tmp1 = (unsigned short *) des;
    unsigned short * tmp2 = (unsigned short *) src;
    int ret             = 0;

    if (tmp1 != NULL && tmp2 != NULL && size > 0) {
        while (size > 0) {
            ret                 = *tmp1 - *tmp2;

            if ((ret != 0) || (*tmp1 == 0) || (*tmp2 == 0)) {
                break;
                }

            tmp1++;
            tmp2++;
            size--;
            }
        }

    return ret;
}


int dchar_nmemcpy(char * des,int des_len,const char * src,int src_len) // len: byte len
{
    unsigned short * tmp1 = (unsigned short *) des;
    unsigned short * tmp2 = (unsigned short *) src;
    int i,len;

    des_len             /= 2;
    src_len             /= 2;
    des_len--;

    if (des_len < src_len) {
        len                 = des_len;
        }
    else {
        len                 = src_len;
        }

    for (i = 0; i < len; i++) {
        *tmp1               = *tmp2;
        tmp1++;
        tmp2++;
        }

    *tmp1               = 0;
    return len;
}


int unicode_to_ucs2encoding(unsigned short unicode,unsigned char * charLength,unsigned char * arrOut)
{
    int status          = 1;
    int index           = 0;

    if (arrOut != 0) {
        if (unicode < 256) {
            arrOut[index++]     = * ((unsigned char *) (&unicode));
            arrOut[index]       = 0;
            }
        else {
            arrOut[index++]     = * ((unsigned char *) (&unicode));
            arrOut[index]       = * (((unsigned char *) (&unicode)) + 1);
            }

        *charLength         = 2;
        }
    else {
        status              = 0;
        }

    return status;
}


int ucs2encoding_to_unicode(unsigned short * pUnicode,unsigned char * arr)
{
    int index           = 0;
    int status          = 1;

    if ((arr != NULL) && (pUnicode != NULL)) {
        * ((unsigned char *) (pUnicode)) = arr[index++];
        * (((unsigned char *) (pUnicode)) + 1) = arr[index];
        }
    else {
        status              = 0;
        }

    return status;
}


// 
// 
void chset_init(void)
{
#ifdef __MMI_CHSET_BIG5__
    {
        static chset_info_struct big5_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_big5_tbl = {
            0
            };

        big5_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        big5_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        big5_ucs2_tbl.UndefChar = 0xFF1F;
        g_chset_tbl[BIG5_TO_UCS2] = &big5_ucs2_tbl;

        ucs2_big5_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_big5_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        ucs2_big5_tbl.UndefChar = 0xA148;
        g_chset_tbl[UCS2_TO_BIG5] = &ucs2_big5_tbl;
        }
#endif /* __MMI_CHSET_BIG5__ */

#if defined(__MMI_CHSET_GB2312__)   || defined(__MMI_CHSET_HKSCS__)
    {
        static chset_info_struct chinese_common_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_chinese_common_tbl = {
            0
            };

        chinese_common_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        chinese_common_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        chinese_common_ucs2_tbl.UndefChar = 0xFF1F;

        ucs2_chinese_common_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_chinese_common_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        ucs2_chinese_common_tbl.UndefChar = 0x233F;

#ifdef __MMI_CHSET_GB2312__
        g_chset_tbl[GB2312_TO_UCS2] = &chinese_common_ucs2_tbl;
        g_chset_tbl[UCS2_TO_GB2312] = &ucs2_chinese_common_tbl;
#endif

#ifdef __MMI_CHSET_HKSCS__
        g_chset_tbl[HKSCS_TO_UCS2] = &chinese_common_ucs2_tbl;
        g_chset_tbl[UCS2_TO_HKSCS] = &ucs2_chinese_common_tbl;
#endif
        }
#endif /* __MMI_CHSET_GB2312__ || __MMI_CHSET_HKSCS__ */

#ifdef __MMI_CHSET_TURKISH_ISO__
    {
        static chset_info_struct turkish_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_turkish_iso_tbl = {
            0
            };

        turkish_iso_ucs2_tbl.pConversnTable = &g_chset_turkish_ucs2_iso[0];
        turkish_iso_ucs2_tbl.pMappingTable = &g_chset_map_turkish_ucs2_iso[0];
        turkish_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_turkish_ucs2_iso_size();
        turkish_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        turkish_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        turkish_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[TURKISH_ISO_TO_UCS2] = &turkish_iso_ucs2_tbl;

        ucs2_turkish_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_turkish_iso[0];
        ucs2_turkish_iso_tbl.pMappingTable = &g_chset_map_ucs2_turkish_iso[0];
        ucs2_turkish_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_turkish_iso_size();
        ucs2_turkish_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_turkish_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_turkish_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_TURKISH_ISO] = &ucs2_turkish_iso_tbl;
        }
#endif /* __MMI_CHSET_TURKISH_ISO__ */

#ifdef __MMI_CHSET_TURKISH_WIN__
    {
        static chset_info_struct turkish_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_turkish_win_tbl = {
            0
            };

        turkish_win_ucs2_tbl.pConversnTable = &g_chset_turkish_ucs2_win[0];
        turkish_win_ucs2_tbl.pMappingTable = &g_chset_map_turkish_ucs2_win[0];
        turkish_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_turkish_ucs2_win_size();
        turkish_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        turkish_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        turkish_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[TURKISH_WINDOWS_TO_UCS2] = &turkish_win_ucs2_tbl;

        ucs2_turkish_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_turkish_win[0];
        ucs2_turkish_win_tbl.pMappingTable = &g_chset_map_ucs2_turkish_win[0];
        ucs2_turkish_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_turkish_win_size();
        ucs2_turkish_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_turkish_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_turkish_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_TURKISH_WINDOWS] = &ucs2_turkish_win_tbl;
        }
#endif /* __MMI_CHSET_TURKISH_WIN__ */

#ifdef __MMI_CHSET_BALTIC_ISO__
    {
        static chset_info_struct baltic_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_baltic_iso_tbl = {
            0
            };

        baltic_iso_ucs2_tbl.pConversnTable = &g_chset_baltic_ucs2_iso[0];
        baltic_iso_ucs2_tbl.pMappingTable = &g_chset_map_baltic_ucs2_iso[0];
        baltic_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_baltic_ucs2_iso_size();
        baltic_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        baltic_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        baltic_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[BALTIC_ISO_TO_UCS2] = &baltic_iso_ucs2_tbl;

        ucs2_baltic_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_baltic_iso[0];
        ucs2_baltic_iso_tbl.pMappingTable = &g_chset_map_ucs2_baltic_iso[0];
        ucs2_baltic_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_baltic_iso_size();
        ucs2_baltic_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_baltic_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_baltic_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_BALTIC_ISO] = &ucs2_baltic_iso_tbl;
        }
#endif /* __MMI_CHSET_BALTIC_ISO__ */

#ifdef __MMI_CHSET_BALTIC_WIN__
    {
        static chset_info_struct baltic_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_baltic_win_tbl = {
            0
            };

        baltic_win_ucs2_tbl.pConversnTable = &g_chset_baltic_ucs2_win[0];
        baltic_win_ucs2_tbl.pMappingTable = &g_chset_map_baltic_ucs2_win[0];
        baltic_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_baltic_win_size();
        baltic_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        baltic_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        baltic_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[BALTIC_WINDOWS_TO_UCS2] = &baltic_win_ucs2_tbl;

        ucs2_baltic_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_baltic_win[0];
        ucs2_baltic_win_tbl.pMappingTable = &g_chset_map_ucs2_baltic_win[0];
        ucs2_baltic_win_tbl.TotalMapTbIndex = mmi_chset_map_baltic_ucs2_win_size();
        ucs2_baltic_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_baltic_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_baltic_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_BALTIC_WINDOWS] = &ucs2_baltic_win_tbl;
        }
#endif /* __MMI_CHSET_BALTIC_WIN__ */

#ifdef __MMI_CHSET_CEURO_ISO__
    {
        static chset_info_struct ceuro_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_ceuro_iso_tbl = {
            0
            };

        ceuro_iso_ucs2_tbl.pConversnTable = &g_chset_ceuro_ucs2_iso[0];
        ceuro_iso_ucs2_tbl.pMappingTable = &g_chset_map_ceuro_ucs2_iso[0];
        ceuro_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_ceuro_ucs2_iso_size();
        ceuro_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        ceuro_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        ceuro_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[CENTRAL_EUROPEAN_ISO_TO_UCS2] = &ceuro_iso_ucs2_tbl;


        ucs2_ceuro_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_ceuro_iso[0];
        ucs2_ceuro_iso_tbl.pMappingTable = &g_chset_map_ucs2_ceuro_iso[0];
        ucs2_ceuro_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_ceuro_iso_size();
        ucs2_ceuro_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_ceuro_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_ceuro_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_CENTRAL_EUROPEAN_ISO] = &ucs2_ceuro_iso_tbl;
        }
#endif /* __MMI_CHSET_CEURO_ISO__ */

#ifdef __MMI_CHSET_CEURO_WIN__
    {
        static chset_info_struct ceuro_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_ceuro_win_tbl = {
            0
            };

        ceuro_win_ucs2_tbl.pConversnTable = &g_chset_ceuro_ucs2_win[0];
        ceuro_win_ucs2_tbl.pMappingTable = &g_chset_map_ceuro_ucs2_win[0];
        ceuro_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_ceuro_ucs2_win_size();
        ceuro_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        ceuro_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        ceuro_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[CENTRAL_EUROPEAN_WINDOWS_TO_UCS2] = &ceuro_win_ucs2_tbl;


        ucs2_ceuro_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_ceuro_win[0];
        ucs2_ceuro_win_tbl.pMappingTable = &g_chset_map_ucs2_ceuro_win[0];
        ucs2_ceuro_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_ceuro_win_size();
        ucs2_ceuro_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_ceuro_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_ceuro_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_CENTRAL_EUROPEAN_WINDOWS] = &ucs2_ceuro_win_tbl;
        }
#endif /* __MMI_CHSET_CEURO_WIN__ */

#ifdef __MMI_CHSET_CYRILLIC_ISO__
    {
        static chset_info_struct cyrillic_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_cyrillic_iso_tbl = {
            0
            };

        cyrillic_iso_ucs2_tbl.pConversnTable = &g_chset_cyrillic_ucs2_iso[0];
        cyrillic_iso_ucs2_tbl.pMappingTable = &g_chset_map_cyrillic_ucs2_iso[0];
        cyrillic_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_cyrillic_ucs2_iso_size();
        cyrillic_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        cyrillic_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        cyrillic_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[CYRILLIC_ISO_TO_UCS2] = &cyrillic_iso_ucs2_tbl;


        ucs2_cyrillic_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_cyrillic_iso[0];
        ucs2_cyrillic_iso_tbl.pMappingTable = &g_chset_map_ucs2_cyrillic_iso[0];
        ucs2_cyrillic_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_cyrillic_iso_size();
        ucs2_cyrillic_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_cyrillic_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_cyrillic_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_CYRILLIC_ISO] = &ucs2_cyrillic_iso_tbl;
        }
#endif /* __MMI_CHSET_CYRILLIC_ISO__ */

#ifdef __MMI_CHSET_CYRILLIC_WIN__
    {
        static chset_info_struct cyrillic_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_cyrillic_win_tbl = {
            0
            };

        cyrillic_win_ucs2_tbl.pConversnTable = &g_chset_cyrillic_ucs2_win[0];
        cyrillic_win_ucs2_tbl.pMappingTable = &g_chset_map_cyrillic_ucs2_win[0];
        cyrillic_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_cyrillic_ucs2_win_size();
        cyrillic_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        cyrillic_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        cyrillic_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[CYRILLIC_WINDOWS_TO_UCS2] = &cyrillic_win_ucs2_tbl;


        ucs2_cyrillic_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_cyrillic_win[0];
        ucs2_cyrillic_win_tbl.pMappingTable = &g_chset_map_ucs2_cyrillic_win[0];
        ucs2_cyrillic_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_cyrillic_win_size();
        ucs2_cyrillic_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_cyrillic_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_cyrillic_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_CYRILLIC_WINDOWS] = &ucs2_cyrillic_win_tbl;
        }
#endif /* __MMI_CHSET_CYRILLIC_WIN__ */


#ifdef __MMI_CHSET_GREEK_ISO__
    {
        static chset_info_struct greek_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_greek_iso_tbl = {
            0
            };

        greek_iso_ucs2_tbl.pConversnTable = &g_chset_greek_ucs2_iso[0];
        greek_iso_ucs2_tbl.pMappingTable = &g_chset_map_greek_ucs2_iso[0];
        greek_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_greek_ucs2_iso_size();
        greek_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        greek_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        greek_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[GREEK_ISO_TO_UCS2] = &greek_iso_ucs2_tbl;


        ucs2_greek_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_greek_iso[0];
        ucs2_greek_iso_tbl.pMappingTable = &g_chset_map_ucs2_greek_iso[0];
        ucs2_greek_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_greek_iso_size();
        ucs2_greek_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_greek_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_greek_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_GREEK_ISO] = &ucs2_greek_iso_tbl;
        }
#endif /* __MMI_CHSET_GREEK_ISO__ */

#ifdef __MMI_CHSET_GREEK_WIN__
    {
        static chset_info_struct greek_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_greek_win_tbl = {
            0
            };

        greek_win_ucs2_tbl.pConversnTable = &g_chset_greek_ucs2_win[0];
        greek_win_ucs2_tbl.pMappingTable = &g_chset_map_greek_ucs2_win[0];
        greek_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_greek_ucs2_win_size();
        greek_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        greek_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        greek_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[GREEK_WINDOWS_TO_UCS2] = &greek_win_ucs2_tbl;


        ucs2_greek_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_greek_win[0];
        ucs2_greek_win_tbl.pMappingTable = &g_chset_map_ucs2_greek_win[0];
        ucs2_greek_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_greek_win_size();
        ucs2_greek_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_greek_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_greek_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_GREEK_WINDOWS] = &ucs2_greek_win_tbl;
        }
#endif /* __MMI_CHSET_GREEK_WIN__ */

#ifdef __MMI_CHSET_HEBREW_ISO__
    {
        static chset_info_struct hebrew_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_hebrew_iso_tbl = {
            0
            };

        hebrew_iso_ucs2_tbl.pConversnTable = &g_chset_hebrew_ucs2_iso[0];
        hebrew_iso_ucs2_tbl.pMappingTable = &g_chset_map_hebrew_ucs2_iso[0];
        hebrew_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_hebrew_ucs2_iso_size();
        hebrew_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        hebrew_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        hebrew_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[HEBREW_ISO_TO_UCS2] = &hebrew_iso_ucs2_tbl;


        ucs2_hebrew_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_hebrew_iso[0];
        ucs2_hebrew_iso_tbl.pMappingTable = &g_chset_map_ucs2_hebrew_iso[0];
        ucs2_hebrew_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_hebrew_iso_size();
        ucs2_hebrew_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_hebrew_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_hebrew_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_HEBREW_ISO] = &ucs2_hebrew_iso_tbl;
        }
#endif /* __MMI_CHSET_HEBREW_ISO__ */

#ifdef __MMI_CHSET_LATIN_ISO__
    {
        static chset_info_struct latin_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_latin_iso_tbl = {
            0
            };

        latin_iso_ucs2_tbl.pConversnTable = &g_chset_latin_ucs2_iso[0];
        latin_iso_ucs2_tbl.pMappingTable = &g_chset_map_latin_ucs2_iso[0];
        latin_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_latin_ucs2_iso_size();
        latin_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        latin_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        latin_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[LATIN_ISO_TO_UCS2] = &latin_iso_ucs2_tbl;


        ucs2_latin_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_latin_iso[0];
        ucs2_latin_iso_tbl.pMappingTable = &g_chset_map_ucs2_latin_iso[0];
        ucs2_latin_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_latin_iso_size();
        ucs2_latin_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_latin_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_latin_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_LATIN_ISO] = &ucs2_latin_iso_tbl;
        }
#endif /* __MMI_CHSET_LATIN_ISO__ */

#ifdef __MMI_CHSET_NORDIC_ISO__
    {
        static chset_info_struct nordic_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_nordic_iso_tbl = {
            0
            };

        nordic_iso_ucs2_tbl.pConversnTable = &g_chset_nordic_ucs2_iso[0];
        nordic_iso_ucs2_tbl.pMappingTable = &g_chset_map_nordic_ucs2_iso[0];
        nordic_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_nordic_ucs2_iso_size();
        nordic_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        nordic_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        nordic_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[NORDIC_ISO_TO_UCS2] = &nordic_iso_ucs2_tbl;


        ucs2_nordic_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_nordic_iso[0];
        ucs2_nordic_iso_tbl.pMappingTable = &g_chset_map_ucs2_nordic_iso[0];
        ucs2_nordic_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_nordic_iso_size();
        ucs2_nordic_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_nordic_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_nordic_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_NORDIC_ISO] = &ucs2_nordic_iso_tbl;
        }
#endif /* __MMI_CHSET_NORDIC_ISO__ */

#ifdef __MMI_CHSET_SEURO_ISO__
    {
        static chset_info_struct seuro_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_seuro_iso_tbl = {
            0
            };

        seuro_iso_ucs2_tbl.pConversnTable = &g_chset_seuro_ucs2_iso[0];
        seuro_iso_ucs2_tbl.pMappingTable = &g_chset_map_seuro_ucs2_iso[0];
        seuro_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_seuro_ucs2_iso_size();
        seuro_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        seuro_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        seuro_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[SOUTH_EUROPEAN_ISO_TO_UCS2] = &seuro_iso_ucs2_tbl;


        ucs2_seuro_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_seuro_iso[0];
        ucs2_seuro_iso_tbl.pMappingTable = &g_chset_map_ucs2_seuro_iso[0];
        ucs2_seuro_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_seuro_iso_size();
        ucs2_seuro_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_seuro_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_seuro_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_SOUTH_EUROPEAN_ISO] = &ucs2_seuro_iso_tbl;
        }
#endif /* __MMI_CHSET_SEURO_ISO__ */

#ifdef __MMI_CHSET_WESTERN_ISO__
    {
        static chset_info_struct western_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_western_iso_tbl = {
            0
            };

        western_iso_ucs2_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_western_iso[0];
        western_iso_ucs2_tbl.pMappingTable = &g_chset_map_ucs2_western_iso[0];
        western_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_western_iso_size();
        western_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        western_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        western_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[WESTERN_ISO_TO_UCS2] = &western_iso_ucs2_tbl;


        ucs2_western_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_western_iso[0];
        ucs2_western_iso_tbl.pMappingTable = &g_chset_map_ucs2_western_iso[0];
        ucs2_western_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_western_iso_size();
        ucs2_western_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_western_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_western_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_WESTERN_ISO] = &ucs2_western_iso_tbl;
        }
#endif /* __MMI_CHSET_WESTERN_ISO__ */

#ifdef __MMI_CHSET_ARMENIAN_ISO__
    {
        static chset_info_struct armenian_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_armenian_iso_tbl = {
            0
            };

        armenian_iso_ucs2_tbl.pConversnTable = &g_chset_armenian_ucs2_iso[0];
        armenian_iso_ucs2_tbl.pMappingTable = &g_chset_map_armenian_ucs2_iso[0];
        armenian_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_armenian_ucs2_iso_size();
        armenian_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        armenian_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        armenian_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[ARMENIAN_ISO_TO_UCS2] = &armenian_iso_ucs2_tbl;


        ucs2_armenian_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_armenian_iso[0];
        ucs2_armenian_iso_tbl.pMappingTable = &g_chset_map_ucs2_armenian_iso[0];
        ucs2_armenian_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_armenian_iso_size();
        ucs2_armenian_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_armenian_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_armenian_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_ARMENIAN_ISO] = &ucs2_armenian_iso_tbl;
        }
#endif /* __MMI_CHSET_ARMENIAN_ISO__ */

#ifdef __MMI_CHSET_WESTERN_WIN__
    {
        static chset_info_struct western_win_ucs2_tbl;
        static chset_info_struct ucs2_western_win_tbl;

        western_win_ucs2_tbl.pConversnTable = NULL;
        western_win_ucs2_tbl.pMappingTable = &g_chset_map_western_win_ucs2[0];
        western_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_western_win_ucs2_size();
        western_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        western_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        western_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[WESTERN_WINDOWS_TO_UCS2] = &western_win_ucs2_tbl;


        ucs2_western_win_tbl.pConversnTable = NULL;
        ucs2_western_win_tbl.pMappingTable = &g_chset_map_ucs2_western_win[0];
        ucs2_western_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_western_win_size();
        ucs2_western_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_western_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_western_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_WESTERN_WINDOWS] = &ucs2_western_win_tbl;
        }
#endif /* __MMI_CHSET_WESTERN_WIN__ */

#ifdef __MMI_CHSET_ARABIC_ISO__
    {
        static chset_info_struct arabic_iso_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_arabic_iso_tbl = {
            0
            };

        arabic_iso_ucs2_tbl.pConversnTable = &g_chset_arabic_ucs2_iso[0];
        arabic_iso_ucs2_tbl.pMappingTable = &g_chset_map_arabic_ucs2_iso[0];
        arabic_iso_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_arabic_ucs2_iso_size();
        arabic_iso_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        arabic_iso_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        arabic_iso_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[ARABIC_ISO_TO_UCS2] = &arabic_iso_ucs2_tbl;


        ucs2_arabic_iso_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_arabic_iso[0];
        ucs2_arabic_iso_tbl.pMappingTable = &g_chset_map_ucs2_arabic_iso[0];
        ucs2_arabic_iso_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_arabic_iso_size();
        ucs2_arabic_iso_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_arabic_iso_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_arabic_iso_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_ARABIC_ISO] = &ucs2_arabic_iso_tbl;
        }
#endif /* __MMI_CHSET_ARABIC_ISO__ */

#ifdef __MMI_CHSET_ARABIC_WIN__
    {
        static chset_info_struct arabic_windows_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_arabic_windows_tbl = {
            0
            };

        arabic_windows_ucs2_tbl.pConversnTable = &g_chset_arabic_ucs2_win[0];
        arabic_windows_ucs2_tbl.pMappingTable = &g_chset_map_ucs2_common[0];
        arabic_windows_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_common_size();
        arabic_windows_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        arabic_windows_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        arabic_windows_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[ARABIC_WINDOWS_TO_UCS2] = &arabic_windows_ucs2_tbl;


        ucs2_arabic_windows_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_arabic_win[0];
        ucs2_arabic_windows_tbl.pMappingTable = &g_chset_map_ucs2_arabic_win[0];
        ucs2_arabic_windows_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_arabic_win_size();
        ucs2_arabic_windows_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_arabic_windows_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_arabic_windows_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_ARABIC_WINDOWS] = &ucs2_arabic_windows_tbl;
        }
#endif /* __MMI_CHSET_ARABIC_WIN__ */

    /* PMT START PERSIAN */
#ifdef __MMI_CHSET_PERSIAN_CP1097__
    {
        static chset_info_struct persian_cp1097_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_persian_cp1097_tbl = {
            0
            };

        persian_cp1097_ucs2_tbl.pConversnTable = &g_chset_persian_ucs2_cp1097[0];
        persian_cp1097_ucs2_tbl.pMappingTable = &g_chset_map_persian_ucs2_cp1097[0];
        persian_cp1097_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_persian_ucs2_cp1097_size();
        persian_cp1097_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        persian_cp1097_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        persian_cp1097_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[PERSIAN_CP1097_TO_UCS2] = &persian_cp1097_ucs2_tbl;


        ucs2_persian_cp1097_tbl.pConversnTable = &g_chset_ucs2_persian_cp1097[0];
        ucs2_persian_cp1097_tbl.pMappingTable = &g_chset_map_ucs2_persian_cp1097[0];
        ucs2_persian_cp1097_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_persian_cp1097_size();
        ucs2_persian_cp1097_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_persian_cp1097_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_persian_cp1097_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_PERSIAN_CP1097] = &ucs2_persian_cp1097_tbl;
        }
#endif /* __MMI_CHSET_PERSIAN_CP1097__ */

#ifdef __MMI_CHSET_PERSIAN_CP1098__
    {
        static chset_info_struct persian_cp1098_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_persian_cp1098_tbl = {
            0
            };

        persian_cp1098_ucs2_tbl.pConversnTable = &g_chset_persian_ucs2_cp1098[0];
        persian_cp1098_ucs2_tbl.pMappingTable = &g_chset_map_persian_ucs2_cp1098[0];
        persian_cp1098_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_persian_ucs2_cp1098_size();
        persian_cp1098_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        persian_cp1098_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        persian_cp1098_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[PERSIAN_CP1098_TO_UCS2] = &persian_cp1098_ucs2_tbl;


        ucs2_persian_cp1098_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_persian_cp1098[0];
        ucs2_persian_cp1098_tbl.pMappingTable = &g_chset_map_ucs2_persian_cp1098[0];
        ucs2_persian_cp1098_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_persian_cp1098_size();
        ucs2_persian_cp1098_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_persian_cp1098_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_persian_cp1098_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_PERSIAN_CP1098] = &ucs2_persian_cp1098_tbl;
        }
#endif /* __MMI_CHSET_PERSIAN_CP1098__ */

    /* PMT END PERSIAN */
#ifdef __MMI_CHSET_HEBREW_WIN__
    {
        static chset_info_struct hebrew_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_hebrew_win_tbl = {
            0
            };

        hebrew_win_ucs2_tbl.pConversnTable = &g_chset_hebrew_ucs2_win[0];
        hebrew_win_ucs2_tbl.pMappingTable = &g_chset_map_hebrew_ucs2_win[0];
        hebrew_win_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_hebrew_ucs2_win_size();
        hebrew_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        hebrew_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        hebrew_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[HEBREW_WINDOWS_TO_UCS2] = &hebrew_win_ucs2_tbl;


        ucs2_hebrew_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_hebrew_win[0];
        ucs2_hebrew_win_tbl.pMappingTable = &g_chset_map_ucs2_hebrew_win[0];
        ucs2_hebrew_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_hebrew_win_size();
        ucs2_hebrew_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_hebrew_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_hebrew_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_HEBREW_WINDOWS] = &ucs2_hebrew_win_tbl;
        }
#endif /* __MMI_CHSET_HEBREW_WIN__ */

#ifdef __MMI_CHSET_EUCKR_WIN__
    {
        static chset_info_struct euckr_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_euckr_win_tbl = {
            0
            };

        euckr_win_ucs2_tbl.pConversnTable = &g_chset_euckr_ucs2_win[0];
        euckr_win_ucs2_tbl.pMappingTable = &g_chset_map_euckr_ucs2_win[0];
        euckr_win_ucs2_tbl.TotalMapTbIndex = g_chset_map_euckr_ucs2_win_size();
        euckr_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        euckr_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        euckr_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[EUCKR_WINDOWS_TO_UCS2] = &euckr_win_ucs2_tbl;


        ucs2_euckr_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_euckr_win[0];
        ucs2_euckr_win_tbl.pMappingTable = &g_chset_map_ucs2_euckr_win[0];
        ucs2_euckr_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_euckr_win_size();
        ucs2_euckr_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_euckr_win_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        ucs2_euckr_win_tbl.UndefChar = 0x003F;
        g_chset_tbl[UCS2_TO_EUCKR_WINDOWS] = &ucs2_euckr_win_tbl;
        }
#endif /* __MMI_CHSET_EUCKR_WIN__ */

#ifdef __MMI_CHSET_THAI_WINDOWS__
    {
        static chset_info_struct thai_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_thai_win_tbl = {
            0
            };

        thai_win_ucs2_tbl.pConversnTable = &g_chset_thai_ucs2_win[0];
        thai_win_ucs2_tbl.pMappingTable = &g_chset_map_thai_ucs2_win[0];
        thai_win_ucs2_tbl.TotalMapTbIndex = g_chset_map_thai_ucs2_win_size();
        thai_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        thai_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        thai_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[THAI_WINDOWS_TO_UCS2] = &thai_win_ucs2_tbl;


        ucs2_thai_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_thai_win[0];
        ucs2_thai_win_tbl.pMappingTable = &g_chset_map_ucs2_thai_win[0];
        ucs2_thai_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_thai_win_size();
        ucs2_thai_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_thai_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_thai_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_THAI_WINDOWS] = &ucs2_thai_win_tbl;
        }
#endif /* __MMI_CHSET_THAI_WINDOWS__ */

#ifdef __MMI_CHSET_VIETNAMESE_WINDOWS__
    {
        static chset_info_struct vietnamese_win_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_vietnamese_win_tbl = {
            0
            };

        vietnamese_win_ucs2_tbl.pConversnTable = &g_chset_vietnamese_ucs2_win[0];
        vietnamese_win_ucs2_tbl.pMappingTable = &g_chset_map_vietnamese_ucs2_win[0];
        vietnamese_win_ucs2_tbl.TotalMapTbIndex = g_chset_map_vietnamese_ucs2_win_size();
        vietnamese_win_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        vietnamese_win_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        vietnamese_win_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[VIETNAMESE_WINDOWS_TO_UCS2] = &vietnamese_win_ucs2_tbl;


        ucs2_vietnamese_win_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_vietnamese_win[0];
        ucs2_vietnamese_win_tbl.pMappingTable = &g_chset_map_ucs2_vietnamese_win[0];
        ucs2_vietnamese_win_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_vietnamese_win_size();
        ucs2_vietnamese_win_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_vietnamese_win_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_vietnamese_win_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_VIETNAMESE_WINDOWS] = &ucs2_vietnamese_win_tbl;
        }
#endif /* __MMI_CHSET_VIETNAMESE_WINDOWS__ */

#ifdef __MMI_CHSET_KOI8_R__
    {
        static chset_info_struct koi8_r_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_koi8_r_tbl = {
            0
            };

        koi8_r_ucs2_tbl.pConversnTable = &g_chset_koi8_r_ucs2[0];
        koi8_r_ucs2_tbl.pMappingTable = &g_chset_map_koi8_r_ucs2[0];
        koi8_r_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_koi8_r_ucs2_size();
        koi8_r_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        koi8_r_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        koi8_r_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[KOI8_R_TO_UCS2] = &koi8_r_ucs2_tbl;


        ucs2_koi8_r_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_koi8_r[0];
        ucs2_koi8_r_tbl.pMappingTable = &g_chset_map_ucs2_koi8_r[0];
        ucs2_koi8_r_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_koi8_r_size();
        ucs2_koi8_r_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_koi8_r_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_koi8_r_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_KOI8_R] = &ucs2_koi8_r_tbl;
        }
#endif /* __MMI_CHSET_KOI8_R__ */

#ifdef __MMI_CHSET_TIS_620__
    {
        static chset_info_struct tis_620_ucs2_tbl = {
            0
            };
        static chset_info_struct ucs2_tis_620_tbl = {
            0
            };

        tis_620_ucs2_tbl.pConversnTable = &g_chset_tis_620_ucs2[0];
        tis_620_ucs2_tbl.pMappingTable = &g_chset_map_tis_620_ucs2[0];
        tis_620_ucs2_tbl.TotalMapTbIndex = mmi_chset_map_tis_620_ucs2_size();
        tis_620_ucs2_tbl.Input_word_bits = CHSET_BIT_WORD_8;
        tis_620_ucs2_tbl.Output_word_bits = CHSET_BIT_WORD_16;
        tis_620_ucs2_tbl.UndefChar = 0x003F;
        g_chset_tbl[TIS_620_TO_UCS2] = &tis_620_ucs2_tbl;


        ucs2_tis_620_tbl.pConversnTable = (const unsigned short *) &g_chset_ucs2_tis_620[0];
        ucs2_tis_620_tbl.pMappingTable = &g_chset_map_ucs2_tis_620[0];
        ucs2_tis_620_tbl.TotalMapTbIndex = mmi_chset_map_ucs2_tis_620_size();
        ucs2_tis_620_tbl.Input_word_bits = CHSET_BIT_WORD_16;
        ucs2_tis_620_tbl.Output_word_bits = CHSET_BIT_WORD_8;
        ucs2_tis_620_tbl.UndefChar = 0x3F;
        g_chset_tbl[UCS2_TO_TIS_620] = &ucs2_tis_620_tbl;
        }
#endif /* __MMI_CHSET_TIS_620__ */

    g_chset_tbl_is_init = 1;
}


static short chset_binary_search(const unsigned char key,const unsigned char * lookup_table,short start,short end)
{
    while (start <= end) {
        short mid           = (start + end) / 2;

        if (key > lookup_table[mid]) {
            start               = mid + 1;
            }
        else if (key < lookup_table[mid]) {
            end                 = mid - 1;
            }
        else /* key == target */
        {
            return mid;
            }
        }

    return - 1;
}


static short chset_binary_search_MSB_mapping_table(const unsigned char key,const MSB_Mapping_Struct * lookup_table,unsigned int tableSize,short start,short end)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned short StartIndex = 0,MidIndex = 0,LastIndex = 0;
    unsigned short StartValue = 0,MidValue = 0,LastValue = 0;
    unsigned short i;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    //binary search for get start_index from start
    LastValue           = tableSize;

    while (StartValue <= LastValue) {
        MidValue            = (StartValue + LastValue) / 2;

        if (start > lookup_table[MidValue].index) {
            StartValue          = MidValue + 1;
            }
        else if (start < lookup_table[MidValue].index) {
            LastValue           = MidValue - 1;
            }
        else /* start == target */
        {
            StartIndex          = MidValue;
            break;
            }
        }

    //get last_endex from end
    i                   = StartIndex;

    while (end > lookup_table[i].index + lookup_table[i].end_value - lookup_table[i].start_value) {
        i++;
        }

    LastIndex           = i;

    if (StartIndex == LastIndex) {
        return key - lookup_table[StartIndex].start_value + lookup_table[StartIndex].index;
        }
    else {
        for (MidIndex = ((StartIndex + LastIndex) / 2); StartIndex <= LastIndex;
             MidIndex = (StartIndex + LastIndex) / 2) {
            if (key > lookup_table[MidIndex].end_value) {
                StartIndex          = MidIndex + 1;
                }
            else if (key < lookup_table[MidIndex].start_value) {
                LastIndex           = MidIndex - 1;
                }
            else {
                return key - lookup_table[MidIndex].start_value + lookup_table[MidIndex].index;
                }
            }
        }

    return - 1;
}


//#if defined(__MMI_CHSET_ALPHABET_SUPPORT__) || defined(__MMI_CHSET_IDEOGRAM_SUPPORT__)
int chset_ucs2_to_ascii(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,unsigned int * src_end_pos)
{
    unsigned short count = 0;
    *src_end_pos        = (unsigned int) pInBuffer;

    while (! ((*pInBuffer == 0) && (* (pInBuffer + 1) == 0)) && (dest_size > count + 1)) {
        if (((unsigned short) (*pInBuffer) <= 0x7F) && ((unsigned short) (* (pInBuffer + 1)) == 0)) {
            *pOutBuffer         = * (pInBuffer);
            }
        else {
            *pOutBuffer         = (char) 0xFF;
            }

        pInBuffer           += 2;
        pOutBuffer++;
        count++;
        }

    *pOutBuffer         = 0;
    *src_end_pos        = (unsigned int) pInBuffer;
    return count + 1;
}


int chset_ascii_to_ucs2(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,unsigned int * src_end_pos)
{
    int count           = 0;
    *src_end_pos        = (unsigned int) pInBuffer;

    while (*pInBuffer != '\0' && dest_size >= (count + 4)) {
        if (*pInBuffer > (char) 0x7F) {
            pOutBuffer[count++] = (char) 0xFF;
            pOutBuffer[count++] = (char) 0xFF;
            }
        else {
            pOutBuffer[count++] = *pInBuffer;
            pOutBuffer[count++] = '\0';
            }

        pInBuffer++;
        }

    pOutBuffer[count++] = '\0';
    pOutBuffer[count++] = '\0';
    *src_end_pos        = (unsigned int) pInBuffer;
    return count;
}


//#endif
//#ifdef __MMI_CHSET_IDEOGRAM_SUPPORT__
unsigned short chset_text_to_ucs2_str_ex(unsigned char * dest,int dest_size,unsigned char * src,TextEncodingEnum encoding,unsigned int * src_end_pos)
{
    const key_index_t * key_LSB_index_table = g_chset_encode_to_ucs2_key_LSB_index[encoding];
    const unsigned char * key_MSB_table = g_chset_encode_to_ucs2_key_MSB[encoding];

#ifdef __MMI_CHSET_GB2312__
    const MSB_Mapping_Struct * gb2312_key_MSB_table = g_chset_gb2312_to_ucs2_key_MSB;
#endif

    const unsigned short * ucs2_table = g_chset_encode_to_ucs2_table[encoding];

    unsigned char key_LSB;
    unsigned char key_MSB;
    short start;
    short end;
    short pos           = 0;
    short index;
    unsigned char * src_end = src + strlen((char *) src);

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (key_LSB_index_table == NULL || ucs2_table == NULL || (key_MSB_table == NULL

#ifdef __MMI_CHSET_GB2312__
    && gb2312_key_MSB_table == NULL
#endif

)) {
        return 0;
        }




    while (* (src) != 0) /* I do not know how we assume that this is end of data.. Currently we are taking only 
                chinese encodings for
                                       the optimized algo so we can take it as 2 bytes */
    {
        if (* (src) <= 0x7f) {
            * (dest + pos)      = *src;
            pos++;
            * (dest + pos)      = 0;
            pos++;
            src                 += 1;
            }
        else {
            if (src_end - src <= 1) {
                /* can't encoding the character. The data may be segmented. */
                break;
                }

            key_LSB             = (unsigned char) (* (src));
            start               = key_LSB_index_table[key_LSB].start;

            if (start < 0) {

                dest[pos]           = (unsigned char) g_chset_unknown_encode_char[encoding];
                dest[pos + 1]       = (unsigned char) (g_chset_unknown_encode_char[encoding] >> 8);
                src                 += 1;
                }
            else {
                key_MSB             = (unsigned char) (* (src + 1));
                end                 = key_LSB_index_table[key_LSB].end;

#ifdef __MMI_CHSET_GB2312__

                if (GB2312_ENCODING_TYPE == encoding) {
                    index               =
                         chset_binary_search_MSB_mapping_table(key_MSB,gb2312_key_MSB_table,mmi_chset_gb2312_to_ucs2_MSB_size (),start,end);
                    }
                else 
#endif

                {
                    index               = chset_binary_search(key_MSB,key_MSB_table,start,end);
                    }

                if (index < 0) /* key MSB not found */
                {
                    dest[pos]           = (unsigned char) g_chset_unknown_encode_char[encoding];
                    dest[pos + 1]       = (unsigned char) (g_chset_unknown_encode_char[encoding] >> 8);
                    }
                else {
                    dest[pos]           = (unsigned char) ucs2_table[index];
                    dest[pos + 1]       = (unsigned char) (ucs2_table[index] >> 8);
                    }

                src                 += 2;
                }

            pos                 += 2;
            }

        if (pos >= dest_size - 2) {
            break;
            }
        }

    *src_end_pos        = (unsigned int) src;
    dest[pos]           = 0;
    dest[pos + 1]       = 0;
    return pos + 2;
}


unsigned short chset_ucs2_to_text_str_ex(unsigned char * dest,int dest_size,unsigned char * src,TextEncodingEnum encoding,unsigned int * src_end_pos)
{
    const key_index_t * key_LSB_index_table = g_chset_ucs2_to_encode_key_LSB_index[encoding];
    const unsigned char * key_MSB_table = g_chset_ucs2_to_encode_key_MSB[encoding];
    const unsigned short * encode_table = g_chset_ucs2_to_encode_table[encoding];
    unsigned char key_LSB;
    unsigned char key_MSB;
    short start;
    short end;
    short index;
    short pos           = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (key_LSB_index_table == NULL || key_MSB_table == NULL || encode_table == NULL) {
        return 0;
        }

    while (*src || * (src + 1)) {
        if (*src <= 0x7f && * (src + 1) == 0) {
            * (dest + pos)      = * (src);
            pos++;
            }
        else {
            /*
             * The original conversion tables of mmi_chset_ucs2_to_text_str() seem to assume the src is big-endian,
             * but the src should be little-endian. We need to change the byte order.
             */
            key_LSB             = (unsigned char) (* (src + 1));
            start               = key_LSB_index_table[key_LSB].start;

            if (start < 0) {
                dest[pos]           = (unsigned char) (g_chset_unknown_encode_char[encoding] >> 8);
                dest[pos + 1]       = (unsigned char) g_chset_unknown_encode_char[encoding];
                }
            else {
                key_MSB             = (unsigned char) (* (src));
                end                 = key_LSB_index_table[key_LSB].end;

                if ((index = chset_binary_search(key_MSB,key_MSB_table,start,end)) < 0) /* key MSB not found */
                {
                    dest[pos]           = (unsigned char) (g_chset_unknown_encode_char[encoding] >> 8);
                    dest[pos + 1]       = (unsigned char) g_chset_unknown_encode_char[encoding];
                    }
                else {
                    dest[pos]           = (unsigned char) (encode_table[index] >> 8);
                    dest[pos + 1]       = (unsigned char) encode_table[index];
                    }

                }

            pos                 += 2;
            }

        src                 += 2;

        if (pos >= dest_size - 1) {
            break;
            }
        }

    *src_end_pos        = (unsigned int) src;
    dest[pos]           = 0;
    return pos + 1;

}


unsigned short chset_encode_decode_algo2(chset_pair_enum Scheme,unsigned char * pOutBuffer,unsigned char * pInBuffer,int nBuffsize,unsigned int * src_end_pos)
{
    switch (Scheme)
    {
#ifdef __MMI_CHSET_GB2312__
        case GB2312_TO_UCS2:
            return chset_text_to_ucs2_str_ex((unsigned char *) pOutBuffer,
                nBuffsize,
                (unsigned char *) pInBuffer,
                GB2312_ENCODING_TYPE,
                src_end_pos);

        case UCS2_TO_GB2312:
            return chset_ucs2_to_text_str_ex((unsigned char *) pOutBuffer,
                nBuffsize,
                (unsigned char *) pInBuffer,
                GB2312_ENCODING_TYPE,
                src_end_pos);

#endif /* __MMI_CHSET_GB2312__ */

#ifdef __MMI_CHSET_BIG5__
        case BIG5_TO_UCS2:
            return chset_text_to_ucs2_str_ex((unsigned char *) pOutBuffer,
                nBuffsize,
                (unsigned char *) pInBuffer,
                BIG_5_ENCODING_TYPE,
                src_end_pos);

        case UCS2_TO_BIG5:
            return chset_ucs2_to_text_str_ex((unsigned char *) pOutBuffer,
                nBuffsize,
                (unsigned char *) pInBuffer,
                BIG_5_ENCODING_TYPE,
                src_end_pos);

#endif /* __MMI_CHSET_BIG5__ */

#ifdef __MMI_CHSET_HKSCS__
        case HKSCS_TO_UCS2:
            return chset_text_to_ucs2_str_ex((unsigned char *) pOutBuffer,
                nBuffsize,
                (unsigned char *) pInBuffer,
                HKSCS_ENCODING_TYPE,
                src_end_pos);

        case UCS2_TO_HKSCS:
            return chset_ucs2_to_text_str_ex((unsigned char *) pOutBuffer,
                nBuffsize,
                (unsigned char *) pInBuffer,
                HKSCS_ENCODING_TYPE,
                src_end_pos);

#endif /* __MMI_CHSET_HKSCS__ */

        default:
            return 0;
        }
}


//#endif
#ifdef __MMI_CHSET_EUCKR_WIN__
static unsigned short chset_encode_decode_algo3(chset_pair_enum Scheme,unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,unsigned int * src_end_pos)
{
    unsigned short UndefChar = 0;
    int count           = 0,temp_count = 0;
    int SearchedValueIndex = -1;
    unsigned char InputWordBits,OutputWordBits;
    unsigned short index;
    const Mapping_Struct * pMappingTable = 0;
    const unsigned short * pConversnTable = 0;

    if (g_chset_tbl_is_init == 0) {
        chset_init();
        }

    pConversnTable      = g_chset_tbl[Scheme]->pConversnTable;
    pMappingTable       = g_chset_tbl[Scheme]->pMappingTable;
    InputWordBits       = g_chset_tbl[Scheme]->Input_word_bits;
    OutputWordBits      = g_chset_tbl[Scheme]->Output_word_bits;
    UndefChar           = g_chset_tbl[Scheme]->UndefChar;

    if (Scheme == EUCKR_WINDOWS_TO_UCS2) {
        if (pConversnTable != 0 && pMappingTable != 0) /* If mapping table exist */
        {
            for (count = 0;
                 (count < dest_size - 2) && (pInBuffer[count - temp_count] != 0 || pInBuffer[count - temp_count + 1] !=
                 0); count = count + 2) {
                unsigned short TempChar;

                if (pInBuffer[count - temp_count] < 0xA1) {
                    pOutBuffer[count]   = pInBuffer[count - temp_count];
                    pOutBuffer[count + 1] = 0;
                    temp_count++;
                    continue;
                    }

                TempChar            = (unsigned char) pInBuffer[count - temp_count + 1];
                TempChar <<= 8;
                TempChar            |= (unsigned char) (pInBuffer[count - temp_count]);

                if (TempChar < (pMappingTable[0].start_value)) {
                    SearchedValueIndex  = -1;
                    }
                else {
                    SearchedValueIndex  = BinarySearchForEncodings(TempChar,Scheme);
                    }

                if (SearchedValueIndex >= 0) {
                    index               = TempChar - pMappingTable[(unsigned short) SearchedValueIndex].start_value;
                    index               = pMappingTable[(unsigned short) SearchedValueIndex].index + index;

                    pOutBuffer[count]   = (unsigned char) pConversnTable[index];
                    pOutBuffer[count + 1] = (unsigned char) (pConversnTable[index] >> 8);
                    }
                else {
                    /* can't encoding the character. */
                    if (pInBuffer[count - temp_count + 2] == 0 && pInBuffer[count - temp_count + 3] == 0) {
                        /* The data may be segmented at the end of pInBuffer[]. */
                        break;
                        }



                    pOutBuffer[count]   = (unsigned char) pInBuffer[count - temp_count + 1]; /* (unsigned char)
                                                                UndefChar; */

                    pOutBuffer[count + 1] = (unsigned char) (pInBuffer[count - temp_count]); /* (unsigned char)(
                                                                UndefChar>>8); */
                    }
                }

            *src_end_pos        = (unsigned int) (pInBuffer + count);
            pOutBuffer[count]   = 0;
            pOutBuffer[++count] = 0;
            return count + 1;
            }
        }
    else {
        temp_count          = 0;

        if (pConversnTable != 0 && pMappingTable != 0) /* If mapping table exist */
        {
            for (count = 0; pInBuffer[count] != 0 || pInBuffer[count + 1] != 0; count = count + 2) {
                unsigned short TempChar;

                /* check for maximum output bufer size */
                if (temp_count >= dest_size - 2) {
                    temp_count          = (unsigned short) dest_size - 2;
                    break;
                    }

                TempChar            = (unsigned char) pInBuffer[count + 1];
                TempChar <<= 8;
                TempChar            |= (unsigned char) (pInBuffer[count]);

                if (TempChar < 0xA1) {
                    pOutBuffer[temp_count] = TempChar;
                    temp_count++;
                    continue;
                    }

                if (TempChar < (pMappingTable[0].start_value)) {
                    SearchedValueIndex  = -1;
                    }
                else {
                    SearchedValueIndex  = BinarySearchForEncodings(TempChar,Scheme);
                    }

                if (SearchedValueIndex >= 0) {
                    index               = TempChar - pMappingTable[(unsigned short) SearchedValueIndex].start_value;
                    index               = pMappingTable[(unsigned short) SearchedValueIndex].index + index;

                    pOutBuffer[temp_count] = (unsigned char) pConversnTable[index];
                    pOutBuffer[temp_count + 1] = (unsigned char) (pConversnTable[index] >> 8);
                    }
                else {
                    /* can't encoding the character. */
                    if (pInBuffer[count + 2] == 0 && pInBuffer[count + 3] == 0) {
                        /* The data may be segmented at the end of pInBuffer[]. */
                        break;
                        }

                    pOutBuffer[temp_count] = (unsigned char) UndefChar;
                    pOutBuffer[temp_count + 1] = (unsigned char) (UndefChar >> 8);
                    }

                temp_count          += 2;
                }

            *src_end_pos        = (unsigned int) (pInBuffer + count);

            pOutBuffer[temp_count] = 0;
            pOutBuffer[++temp_count] = 0;
            return temp_count + 1;
            }
        }

    return 0;
}


#endif /* __MMI_CHSET_EUCKR_WIN__ */

#ifdef __MMI_CHSET_ALPHABET_SUPPORT__
unsigned short chset_binary_search_conversn_table(const unsigned short index,const Alphabet_Mapping_Struct * lookup_table,unsigned int tableSize)
{
    unsigned short StartValue = 0,MidValue = 0,LastValue = 0;
    unsigned short ret  = 0x3f;                     //undef char

    LastValue           = tableSize;
    MidValue            = (StartValue + LastValue) / 2;

    for (; StartValue <= LastValue; MidValue = (StartValue + LastValue) / 2) {
        if (index > lookup_table[MidValue].end_index) {
            StartValue          = MidValue + 1;
            }
        else if (MidValue != 0 && index <= lookup_table[MidValue - 1].end_index) {
            LastValue           = MidValue - 1;
            }
        else {
            if (MidValue != 0) {
                ret                 =
                     (unsigned short) (lookup_table[MidValue].base_value + index - (lookup_table[MidValue - 1].end_index + 1));
                }
            else {
                ret                 = (unsigned short) (lookup_table[MidValue].base_value + index);
                }

            break;
            }
        }

    return ret;
}


int BinarySearchForEncodings(unsigned short CharToSearch,chset_pair_enum Scheme)
{
    unsigned short StartValue = 0,MidValue = 0,LastValue = 0;
    const Mapping_Struct * pMappingTable = 0;

    LastValue           = g_chset_tbl[Scheme]->TotalMapTbIndex;
    pMappingTable       = g_chset_tbl[Scheme]->pMappingTable;

    for (MidValue = ((StartValue + LastValue) / 2); StartValue <= LastValue;
         MidValue = (StartValue + LastValue) / 2) {

        if (CharToSearch > pMappingTable[MidValue].end_value) {
            StartValue          = MidValue + 1;
            }
        else if (CharToSearch < pMappingTable[MidValue].start_value) {
            LastValue           = MidValue - 1;
            }
        else {
            return (int) MidValue;
            }
        }

    return - 1;
}


static unsigned short chset_encode_decode(chset_pair_enum Scheme,unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,unsigned int * src_end_pos)
{
    unsigned short UndefChar = 0;
    int count           = 0;
    int SearchedValueIndex = -1;
    unsigned char InputWordBits,OutputWordBits;
    unsigned short index;
    const Mapping_Struct * pMappingTable = 0;

#ifdef __MMI_CHSET_WESTERN_WIN__
    const Alphabet_Mapping_Struct * pWesternWinConversnTable = 0;
    getWesternWinTableSize getTableSize = 0;
#endif

    const unsigned short * pConversnTable = 0;

    if (g_chset_tbl_is_init == 0) {
        chset_init();
        }

#ifdef __MMI_CHSET_WESTERN_WIN__

    if (Scheme == UCS2_TO_WESTERN_WINDOWS) {
        pWesternWinConversnTable = g_chset_ucs2_western_win;
        getTableSize        = mmi_chset_ucs2_western_win_size;
        }
    else if (Scheme == WESTERN_WINDOWS_TO_UCS2) {
        pWesternWinConversnTable = g_chset_western_win_ucs2;
        getTableSize        = mmi_chset_western_win_ucs2_size;
        }

#endif

    pConversnTable      = g_chset_tbl[Scheme]->pConversnTable;
    pMappingTable       = g_chset_tbl[Scheme]->pMappingTable;
    InputWordBits       = g_chset_tbl[Scheme]->Input_word_bits;
    OutputWordBits      = g_chset_tbl[Scheme]->Output_word_bits;
    UndefChar           = g_chset_tbl[Scheme]->UndefChar;

    if ((pConversnTable != 0

#ifdef __MMI_CHSET_WESTERN_WIN__
    || pWesternWinConversnTable != 0
#endif

) && pMappingTable != 0) /* If mapping table exist */
    {
        if (InputWordBits == CHSET_BIT_WORD_16) {
            for (count = 0; pInBuffer[count] != 0 || pInBuffer[count + 1] != 0; count = count + 2) {
                unsigned short TempChar;

                /* check for maximum output bufer size */
                if ((OutputWordBits == CHSET_BIT_WORD_16) && (count >= dest_size - 2)) {
                    count               = (unsigned short) dest_size - 2;
                    break;
                    }
                else if ((OutputWordBits == CHSET_BIT_WORD_8) && (count / 2 >= dest_size - 1)) {
                    count               = (unsigned short) (dest_size - 1) * 2;
                    break;
                    }

                TempChar            = (unsigned char) pInBuffer[count + 1];
                TempChar <<= 8;
                TempChar            |= (unsigned char) (pInBuffer[count]);

                if (TempChar < (pMappingTable[0].start_value)) {
                    SearchedValueIndex  = -1;
                    }
                else {
                    SearchedValueIndex  = BinarySearchForEncodings(TempChar,Scheme);
                    }

                if (SearchedValueIndex >= 0) {
                    unsigned short tempValue;
                    index               = TempChar - pMappingTable[(unsigned short) SearchedValueIndex].start_value;
                    index               = pMappingTable[(unsigned short) SearchedValueIndex].index + index;

#ifdef __MMI_CHSET_WESTERN_WIN__

                    if (Scheme == UCS2_TO_WESTERN_WINDOWS || Scheme == WESTERN_WINDOWS_TO_UCS2) {
                        tempValue           =
                             mmi_chset_binary_search_conversn_table(index,pWesternWinConversnTable,getTableSize ());
                        }
                    else 
#endif

                    {
                        tempValue           = (unsigned short) ((OutputWordBits ==
                             CHSET_BIT_WORD_16) ? pConversnTable[index]: ((unsigned char *) pConversnTable)[index]);
                        }

                    if (OutputWordBits == CHSET_BIT_WORD_16) {
                        pOutBuffer[count]   = (unsigned char) tempValue;
                        pOutBuffer[count + 1] = (unsigned char) (tempValue >> 8);
                        }
                    else {
                        pOutBuffer[count / 2] = (unsigned char) tempValue;
                        }
                    }
                else {
                    /* can't encoding the character. */
                    if (OutputWordBits == CHSET_BIT_WORD_16) {
                        pOutBuffer[count]   = (unsigned char) UndefChar;
                        pOutBuffer[count + 1] = (unsigned char) (UndefChar >> 8);
                        }
                    else {
                        pOutBuffer[count / 2] = (unsigned char) UndefChar;
                        }

                    if (pInBuffer[count + 2] == 0 && pInBuffer[count + 3] == 0) {
                        /* The data may be segmented at the end of pInBuffer[]. */
                        break;
                        }
                    }
                }

            *src_end_pos        = (unsigned int) (pInBuffer + count);

            if (OutputWordBits == CHSET_BIT_WORD_16) {
                pOutBuffer[count]   = 0;
                pOutBuffer[++count] = 0;
                return count + 1;
                }
            else {
                pOutBuffer[count / 2] = 0;
                return (count / 2 + 1);
                }
            } /* if InputWordBits=CHSET_BIT_WORD_16 */
        else /* if InputWordBits=CHSET_BIT_WORD_8 */
        {
            unsigned char TempChar;

            for (count = 0; pInBuffer[count] != 0; count++) {
                TempChar            = pInBuffer[count];

                /* check for maximum output bufer size */
                if ((OutputWordBits == CHSET_BIT_WORD_16) && (count * 2 >= dest_size - 2)) {
                    count               = ((unsigned short) dest_size - 2) / 2;
                    break;
                    }
                else if ((OutputWordBits == CHSET_BIT_WORD_8) && (count >= dest_size - 1)) {
                    count               = (unsigned short) dest_size - 1;
                    break;
                    }

                if (TempChar < (pMappingTable[0].start_value)) {
                    SearchedValueIndex  = -1;
                    }
                else {
                    SearchedValueIndex  = BinarySearchForEncodings(TempChar,Scheme);
                    }

                if (SearchedValueIndex >= 0) {
                    unsigned short tempValue;
                    index               = TempChar - pMappingTable[(unsigned short) SearchedValueIndex].start_value;
                    index               = pMappingTable[(unsigned short) SearchedValueIndex].index + index;

#ifdef __MMI_CHSET_WESTERN_WIN__

                    if (Scheme == UCS2_TO_WESTERN_WINDOWS || Scheme == WESTERN_WINDOWS_TO_UCS2) {
                        tempValue           =
                             chset_binary_search_conversn_table(index,pWesternWinConversnTable,getTableSize ());
                        }
                    else 
#endif

                    {
                        tempValue           = (unsigned short) ((Scheme != WESTERN_ISO_TO_UCS2 &&
                             OutputWordBits == CHSET_BIT_WORD_16) ? pConversnTable[index]: ((unsigned char *) pConversnTable)[index]);
                        }



                    if (Scheme == WESTERN_ISO_TO_UCS2) /* Special case, since both side conversions share single table
                                                                 */
                    {
                        pOutBuffer[count * 2] = (unsigned char) tempValue;
                        pOutBuffer[count * 2 + 1] = 0;
                        }
                    else if (OutputWordBits == CHSET_BIT_WORD_16) {
                        pOutBuffer[count * 2] = (unsigned char) tempValue;
                        pOutBuffer[count * 2 + 1] = (unsigned char) (tempValue >> 8);
                        }
                    else {
                        pOutBuffer[count]   = (unsigned char) tempValue;
                        }
                    }
                else {
                    if (OutputWordBits == CHSET_BIT_WORD_16) {
                        pOutBuffer[count * 2] = (unsigned char) UndefChar;
                        pOutBuffer[count * 2 + 1] = (unsigned char) (UndefChar >> 8);
                        }
                    else {
                        pOutBuffer[count]   = (unsigned char) UndefChar;
                        }
                    }
                }

            *src_end_pos        = (unsigned int) (pInBuffer + count);

            if (OutputWordBits == CHSET_BIT_WORD_16) {
                pOutBuffer[count * 2] = 0;
                pOutBuffer[(count * 2) + 1] = 0;
                return (count * 2 + 2);
                }
            else {
                pOutBuffer[count]   = 0;
                return (count + 1);
                }
            } /* if InputWordBits=CHSET_BIT_WORD_8 */
        } /* If mapping table exist */

    return 0;
}


#endif /* __MMI_CHSET_ALPHABET_SUPPORT__ */

#ifdef __MMI_CHSET_SJIS__
unsigned int chset_sjis_to_ucs2(unsigned char * dest,int dest_size,unsigned char * src,unsigned int * src_end_pos)
{
    const key_index_t * key_LSB_index_table = g_chset_sjis_to_ucs2_key_LSB_index;
    const unsigned char * key_MSB_table = g_chset_sjis_to_ucs2_key_MSB;
    const unsigned short * ucs2_table = g_chset_sjis_to_ucs2_table;

    unsigned char key_LSB;
    unsigned char key_MSB;
    short start;
    short end;
    unsigned int pos    = 0;
    short index;
    unsigned char * src_end = src + strlen((char *) src);

    if (key_LSB_index_table == NULL || key_MSB_table == NULL || ucs2_table == NULL) {
        return 0;
        }

    while (* (src) != 0) {
        if (* (src) <= 0x7f) {
            * (dest + pos)      = *src;
            pos++;
            * (dest + pos)      = 0;
            pos++;
            src                 += 1;
            }
        else if ((* (src) >= 0xA1) && (* (src) <= 0xDF)) {
            * (dest + pos)      = 0x61 + (* (src) - 0xA1);
            pos++;
            * (dest + pos)      = 0xFF;
            pos++;
            src                 += 1;
            }
        else {
            key_LSB             = (unsigned char) (* (src + 1));
            start               = key_LSB_index_table[key_LSB].start;

            if (start < 0) {
                if (src_end - src <= 1) {
                    /* can't encoding the character. The data may be segmented. */
                    break;
                    }
                else {
                    dest[pos]           = (unsigned char) 0xFF;
                    dest[pos + 1]       = (unsigned char) 0xFF;
                    }
                }
            else {
                key_MSB             = (unsigned char) (* (src));
                end                 = key_LSB_index_table[key_LSB].end;

                if ((index = chset_binary_search(key_MSB,key_MSB_table,start,end)) < 0) /* key MSB not found */
                {
                    if (src_end - src <= 1) {
                        /* can't encoding the character. The data may be segmented. */
                        break;
                        }
                    else {
                        dest[pos]           = (unsigned char) 0xFF;
                        dest[pos + 1]       = (unsigned char) 0xFF;
                        }
                    }
                else {
                    dest[pos]           = (unsigned char) ucs2_table[index];
                    dest[pos + 1]       = (unsigned char) (ucs2_table[index] >> 8);

                    }
                }

            src                 += 2;
            pos                 += 2;
            }

        if (pos >= dest_size - 2) {
            break;
            }
        }

    *src_end_pos        = (unsigned int) src;
    dest[pos]           = 0;
    dest[pos + 1]       = 0;
    return pos + 2;
}


unsigned int chset_ucs2_to_sjis(unsigned char * dest,int dest_size,unsigned char * src,unsigned int * src_end_pos)
{
    const key_index_t * key_LSB_index_table = g_chset_ucs2_to_sjis_key_LSB_index;
    const unsigned char * key_MSB_table = g_chset_ucs2_to_sjis_key_MSB;
    const unsigned short * encode_table = g_chset_ucs2_to_sjis_table;
    unsigned char key_LSB;
    unsigned char key_MSB;
    short start;
    short end;
    short index;
    unsigned int pos    = 0;

    if (key_LSB_index_table == NULL || key_MSB_table == NULL || encode_table == NULL) {
        return 0;
        }

    while (*src || * (src + 1)) {
        if (*src <= 0x7f && * (src + 1) == 0) {
            * (dest + pos)      = * (src);
            pos++;
            }
        else if ((* (src + 1) == 0xFF) && ((*src >= 0x61) && (*src <= 0x9F))) {
            * (dest + pos)      = 0xA1 + (* (src) - 0x61);
            pos++;
            }
        else {
            key_LSB             = (unsigned char) (* (src));
            start               = key_LSB_index_table[key_LSB].start;

            if (start < 0) {
                dest[pos]           = (unsigned char) 0xFF;
                dest[pos + 1]       = (unsigned char) 0xFF;
                }
            else {
                key_MSB             = (unsigned char) (* (src + 1));
                end                 = key_LSB_index_table[key_LSB].end;

                if ((index = chset_binary_search(key_MSB,key_MSB_table,start,end)) < 0) /* key MSB not found */
                {
                    dest[pos]           = (unsigned char) 0xFF;
                    dest[pos + 1]       = (unsigned char) 0xFF;
                    }
                else {
                    dest[pos]           = (unsigned char) (encode_table[index] >> 8);
                    dest[pos + 1]       = (unsigned char) encode_table[index];
                    }

                }

            pos                 += 2;
            }

        src                 += 2;

        if (pos >= dest_size - 2) {
            break;
            }
        }

    *src_end_pos        = (unsigned int) src;
    dest[pos]           = 0;
    dest[pos + 1]       = 0;
    return pos + 2;
}


#endif /* __MMI_CHSET_SJIS__ */

#ifdef __MMI_CHSET_GB18030__
static int chset_gb18030_to_ucs2_2_byte_search(unsigned short src,unsigned short * dest)
{
    int start,end,mid;
    unsigned short result;

    start               = 0;
    end                 = mmi_chset_gb18030_ucs2_2byte_size() - 1;

    while (start <= end) {
        if ((end - start) == 1) /* to prevent missed-comaprison */
        {
            mid                 = end;
            }
        else {
            mid                 = (start + end) / 2;
            }

        result              = g_chset_gb18030_to_ucs2_2_byte_tbl[mid].gb_code;

        if (src > result) {
            start               = mid + 1;
            }
        else if (src < result) {
            end                 = mid - 1;
            }
        else /* found */
        {
            *dest               = g_chset_gb18030_to_ucs2_2_byte_tbl[mid].ucs2_code;
            return 1;
            }
        }

    return 0;
}


static int chset_gb18030_to_ucs2_4_byte_search(unsigned int src,unsigned short * dest)
{
    int start,end,mid;
    unsigned int result;

    start               = 0;
    end                 = mmi_chset_gb18030_ucs2_4byte_size() - 1;

    while (start <= end) {
        if ((end - start) == 1) /* to prevent missed-comaprison */
        {
            mid                 = end;
            }
        else {
            mid                 = (start + end) / 2;
            }

        result              =
             (unsigned int) ((g_chset_gb18030_to_ucs2_4_byte_tbl[mid].gb_code_high << 16) + (g_chset_gb18030_to_ucs2_4_byte_tbl[mid].gb_code_low));

        if (src > result) {
            start               = mid + 1;
            }
        else if (src < result) {
            end                 = mid - 1;
            }
        else /* found */
        {
            *dest               = g_chset_gb18030_to_ucs2_4_byte_tbl[mid].ucs2_code;
            return 1;
            }
        }

    return 0;
}


unsigned short chset_gb18030_to_ucs2(unsigned char * pOutBuffer,unsigned char * pInBuffer,int inLen)
{
    unsigned char c1,c2,c3,c4;

    /* pre-insert unknown character */
    pOutBuffer[0]       = 0xff;
    pOutBuffer[1]       = 0xff;

    c1                  = pInBuffer[0];

    if (c1 < 0x80) /* 1 byte */
    {
        pOutBuffer[0]       = pInBuffer[0];
        pOutBuffer[1]       = 0x00;
        return 1; /* consume 1 byte */
        }

    if (inLen >= 2) {
        c2                  = pInBuffer[1];
        }
    else {
        return 1; /* consume 1 byte */
        }

    /* 2 bytes */
    if ((c1 >= 0x81 && c1 <= 0xfe) && ((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0x80 && c2 <= 0xfe))) {
        unsigned short src,dest;

        src                 = (((c1 << 8) & 0xff00) +c2);

        /* binary search 2 bytes table */
        if (chset_gb18030_to_ucs2_2_byte_search(src,&dest)) {
            pOutBuffer[0]       = (unsigned char) (dest & 0xff);
            pOutBuffer[1]       = (unsigned char) (dest >> 8);
            return 2;
            }
        else {
            return 1; /* consume 1 byte */
            }
        }

    if (inLen >= 4) /* 4 bytes */
    {
        c3                  = pInBuffer[2];
        c4                  = pInBuffer[3];

        if (c1 >= 0x81 && c2 <= 0xfe)
            if (c2 >= 0x30 && c2 <= 0x39)
                if (c3 >= 0x81 && c3 <= 0xfe)
                    if (c4 >= 0x30 && c4 <= 0x39) {
                        unsigned int src;
                        unsigned short dest;

                        /* binary search 4 bytes table */
                        src                 = (unsigned int) ((c1 << 24) + (c2 << 16) + (c3 << 8) +c4);

                        if (chset_gb18030_to_ucs2_4_byte_search(src,&dest)) {
                            pOutBuffer[0]       = (unsigned char) (dest & 0xff);
                            pOutBuffer[1]       = (unsigned char) (dest >> 8);
                            return 4;
                            }
                        else {
                            return 1; /* consume 1 byte */
                            }
                        }
        }

    return 1;
}


static unsigned short chset_gb18030_to_ucs2_str(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,unsigned int * src_end_pos)
{
    unsigned char * in_cursor = pInBuffer;
    unsigned char * out_cursor = pOutBuffer;
    unsigned char tmp_result[2];

    unsigned short diget_len;
    int in_len          = strlen((char *) pInBuffer);
    int out_len         = 0;

    while (in_len && dest_size > (out_len + 2)) {
        diget_len           = chset_gb18030_to_ucs2((unsigned char *) &tmp_result,(unsigned char *) in_cursor,in_len);

        if (tmp_result[0] == 0xFF && tmp_result[1] == 0xFF && in_len == diget_len) {
            /* the last remained data of pInBuffer may be segmented */
            break;
            }

        in_len              -= diget_len;
        in_cursor           += diget_len;

        out_cursor[0]       = tmp_result[0];
        out_cursor[1]       = tmp_result[1];
        out_cursor          += 2;
        out_len             += 2;
        }

    *src_end_pos        = (unsigned int) in_cursor;

    pOutBuffer[out_len] = 0;
    pOutBuffer[++out_len] = 0;
    return (out_len + 1);
}


static unsigned short chset_gb18030_to_ucs2_str_with_src_length(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,int in_len)
{
    unsigned char * in_cursor = pInBuffer;
    unsigned char * out_cursor = pOutBuffer;
    unsigned char tmp_result[2];

    unsigned short diget_len;
    int out_len         = 0;

    while (in_len && dest_size > (out_len + 2)) {
        diget_len           = chset_gb18030_to_ucs2((unsigned char *) &tmp_result,(unsigned char *) in_cursor,in_len);
        in_len              -= diget_len;
        in_cursor           += diget_len;

        out_cursor[0]       = tmp_result[0];
        out_cursor[1]       = tmp_result[1];
        out_cursor          += 2;
        out_len             += 2;
        }

    return (out_len);
}


static int chset_ucs2_to_gb18030_2_byte_search(unsigned short src,unsigned int * dest)
{
    int start,end,mid;
    unsigned short result;

    start               = 0;
    end                 = mmi_chset_ucs2_gb18030_2byte_size() - 1;

    while (start <= end) {
        if ((end - start) == 1) /* to prevent missed-comaprison */
        {
            mid                 = end;
            }
        else {
            mid                 = (start + end) / 2;
            }

        result              = g_chset_ucs2_to_gb18030_2_byte_tbl[mid].ucs2_code;

        if (src > result) {
            start               = mid + 1;
            }
        else if (src < result) {
            end                 = mid - 1;
            }
        else /* found */
        {
            *dest               = g_chset_ucs2_to_gb18030_2_byte_tbl[mid].gb_code;
            return 1;
            }
        }

    return 0;

}


static int chset_ucs2_to_gb18030_4_byte_search(unsigned short src,unsigned int * dest)
{
    int start,end,mid;
    unsigned short result;

    start               = 0;
    end                 = mmi_chset_gb18030_ucs2_4byte_size() - 1;

    while (start <= end) {
        if ((end - start) == 1) /* to prevent missed-comaprison */
        {
            mid                 = end;
            }
        else {
            mid                 = (start + end) / 2;
            }

        result              = g_chset_gb18030_to_ucs2_4_byte_tbl[mid].ucs2_code;

        if (src > result) {
            start               = mid + 1;
            }
        else if (src < result) {
            end                 = mid - 1;
            }
        else /* found */
        {
            *dest               =
                 (unsigned int) ((g_chset_gb18030_to_ucs2_4_byte_tbl[mid].gb_code_high << 16) +g_chset_gb18030_to_ucs2_4_byte_tbl[mid].gb_code_low);
            return 1;
            }
        }

    return 0;

}


unsigned short chset_ucs2_to_gb18030(unsigned char * pOutBuffer,unsigned char * pInBuffer)
{
    unsigned short ucs2_char;
    unsigned int gb_char;

    /* single byte */
    if (pInBuffer[1] == 0 && pInBuffer[0] < 0x80) {
        pOutBuffer[0]       = pInBuffer[0];
        return 1;
        }

    ucs2_char           = ((pInBuffer[1] << 8) +pInBuffer[0]);

    /* search 2 bytes table */
    if (chset_ucs2_to_gb18030_2_byte_search(ucs2_char,&gb_char)) {
        pOutBuffer[0]       = (unsigned char) ((gb_char >> 8) & 0xff);
        pOutBuffer[1]       = (unsigned char) (gb_char & 0xff);
        return 2;
        }

    /* search 4 bytes table */
    if (chset_ucs2_to_gb18030_4_byte_search(ucs2_char,&gb_char)) {
        pOutBuffer[0]       = (unsigned char) ((gb_char >> 24) & 0xff);
        pOutBuffer[1]       = (unsigned char) ((gb_char >> 16) & 0xff);
        pOutBuffer[2]       = (unsigned char) ((gb_char >> 8) & 0xff);
        pOutBuffer[3]       = (unsigned char) (gb_char & 0xff);
        return 4;
        }

    /* unknown character */
    pOutBuffer[0]       = 0xff;
    return 1;
}


static unsigned short chset_ucs2_to_gb18030_str(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,unsigned int * src_end_pos)
{
    unsigned char * in_cursor = pInBuffer;
    unsigned char * out_cursor = pOutBuffer;
    unsigned char tmp_result[4];

    int out_len         = 0;
    int i;
    unsigned short result_len;

    while ((in_cursor[0] != 0) || (in_cursor[1] != 0) && (dest_size > (out_len + 2))) {
        result_len          = chset_ucs2_to_gb18030((unsigned char *) &tmp_result,(unsigned char *) in_cursor);

        for (i = (result_len - 1); i >= 0; i--) {
            out_cursor[i]       = tmp_result[i];
            }

        in_cursor           += 2;
        out_cursor          += result_len;
        out_len             += result_len;
        }

    *src_end_pos        = (unsigned int) in_cursor;

    pOutBuffer[out_len] = 0;
    pOutBuffer[++out_len] = 0;
    return (out_len + 1);
}


static unsigned short chset_ucs2_to_gb18030_str_with_src_length(unsigned char * pOutBuffer,unsigned char * pInBuffer,int dest_size,int src_size)
{
    unsigned char * in_cursor = pInBuffer;
    unsigned char * out_cursor = pOutBuffer;
    unsigned char tmp_result[4];

    int out_len         = 0;
    int i;
    unsigned short result_len;
    int src_index       = 0;

    while (src_index < src_size && (dest_size > (out_len + 2))) {
        result_len          = chset_ucs2_to_gb18030((unsigned char *) &tmp_result,(unsigned char *) in_cursor);

        for (i = (result_len - 1); i >= 0; i--) {
            out_cursor[i]       = tmp_result[i];
            }

        in_cursor           += 2;
        out_cursor          += result_len;
        out_len             += result_len;
        src_index           += 2;
        }

    return (out_len);
}


int chset_convert_with_src_length(chset_enum src_type,chset_enum dest_type,char * src_buff,int src_size,char * dest_buff,int dest_size)
{
    if (src_type == CHSET_GB18030 && dest_type == CHSET_UCS2) {
        return chset_gb18030_to_ucs2_str_with_src_length((unsigned char *) dest_buff,(unsigned char *) src_buff,dest_size,src_size);
        }
    else if (src_type == CHSET_UCS2 && dest_type == CHSET_GB18030) {
        return chset_ucs2_to_gb18030_str_with_src_length((unsigned char *) dest_buff,(unsigned char *) src_buff,dest_size,src_size);
        }

    /* only support GB18030 for now */
    return 0;

}


#endif /* __MMI_CHSET_GB18030__ */

int chset_convert_from_native_ucs2(chset_enum dest_type,unsigned char * src_buff,unsigned char * dest_buff,int dest_size,unsigned int * src_end_pos)
{
    int result          = 0;

    switch (dest_type)
    {
        case CHSET_ASCII:
            result = chset_ucs2_to_ascii(dest_buff,src_buff,dest_size,src_end_pos);
            break;

        case CHSET_UTF8:
            result =
                 chset_ucs2_to_utf8_string_ex((unsigned char *) dest_buff,dest_size,(unsigned char *) src_buff,src_end_pos);
            break;

        case CHSET_UTF16LE:
            result = chset_ucs2_to_utf16_string(dest_buff,dest_size,CHSET_UTF16LE,0,src_buff,src_end_pos);
            break;

        case CHSET_UTF16BE:
            result = chset_ucs2_to_utf16_string(dest_buff,dest_size,CHSET_UTF16BE,0,src_buff,src_end_pos);
            break;

#ifdef __MMI_CHSET_BIG5__
        case CHSET_BIG5:

            result = chset_encode_decode_algo2(UCS2_TO_BIG5,dest_buff,src_buff,dest_size,src_end_pos); /* Length 
                            is 
                                        hardcode for now to maximum */
            break;

#endif

#ifdef __MMI_CHSET_GB2312__
        case CHSET_GB2312:

            result = chset_encode_decode_algo2(UCS2_TO_GB2312,dest_buff,src_buff,dest_size,src_end_pos); /* Length
                             is 
                                        hardcode for now to maximum */
            break;

#endif

#ifdef __MMI_CHSET_HKSCS__
        case CHSET_HKSCS:

            result = chset_encode_decode_algo2(UCS2_TO_HKSCS,dest_buff,src_buff,dest_size,src_end_pos); /* Length 
                            is 
                                        hardcode for now to maximum */
            break;

#endif

#ifdef __MMI_CHSET_UTF7__
        case CHSET_UTF7:
            result = chset_ucs2_to_utf7_str(dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_GB18030__
        case CHSET_GB18030:
            result = chset_ucs2_to_gb18030_str(dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_SJIS__
        case MMI_CHSET_SJIS:
            result = chset_ucs2_to_sjis((unsigned char *) dest_buff,dest_size,(unsigned char *) src_buff,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_EUCKR_WIN__
        case CHSET_EUCKR:
            result = chset_encode_decode_algo3(UCS2_TO_EUCKR_WINDOWS,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_THAI_WINDOWS__
        case MMI_CHSET_THAI_WIN:
            result = chset_encode_decode(UCS2_TO_THAI_WINDOWS,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_VIETNAMESE_WINDOWS__
        case CHSET_VIETNAMESE_WIN:
            result = chset_encode_decode(UCS2_TO_VIETNAMESE_WINDOWS,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_KOI8_R__
        case CHSET_KOI8_R:
            result = chset_encode_decode(UCS2_TO_KOI8_R,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_TIS_620__
        case CHSET_TIS_620:
            result = chset_encode_decode(UCS2_TO_TIS_620,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

        default:
#ifdef __MMI_CHSET_ALPHABET_SUPPORT__

            /* For all rest of the encodings. */
            result =
                 chset_encode_decode((chset_pair_enum) (BASE_ENCODING_SCHEME + (dest_type - CHSET_BASE) * 2),dest_buff,src_buff,dest_size,src_end_pos);

#else /* __MMI_CHSET_ALPHABET_SUPPORT__ */

            result = 0;
#endif

            break;
        }

    return result;

}


int chset_convert_to_native_ucs2(chset_enum src_type,unsigned char * src_buff,unsigned char * dest_buff,int dest_size,unsigned int * src_end_pos)
{
    int result;

    switch (src_type)
    {
        case CHSET_ASCII:
            result = chset_ascii_to_ucs2(dest_buff,src_buff,dest_size,src_end_pos);
            break;

        case CHSET_UTF8:
            result = chset_utf8_to_ucs2_string_ex(dest_buff,dest_size,src_buff,src_end_pos);
            break;

        case CHSET_UTF16LE:
            result = chset_utf16_to_ucs2_string(dest_buff,dest_size,src_buff,CHSET_UTF16LE,src_end_pos);
            break;

        case CHSET_UTF16BE:
            result = chset_utf16_to_ucs2_string(dest_buff,dest_size,src_buff,CHSET_UTF16BE,src_end_pos);
            break;

#ifdef __MMI_CHSET_BIG5__
        case CHSET_BIG5:

            result = chset_encode_decode_algo2(BIG5_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos); /* Length 
                            is 
                                        hardcode for now to maximum */
            break;

#endif

#ifdef __MMI_CHSET_GB2312__
        case CHSET_GB2312:

            result = chset_encode_decode_algo2(GB2312_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos); /* Length
                             is 
                                        hardcode for now to maximum */
            break;

#endif

#ifdef __MMI_CHSET_HKSCS__
        case CHSET_HKSCS:

            result = chset_encode_decode_algo2(HKSCS_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos); /* Length 
                            is 
                                        hardcode for now to maximum */
            break;

#endif

#ifdef __MMI_CHSET_UTF7__
        case CHSET_UTF7:
            result = chset_utf7_to_ucs2_str(dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_GB18030__
        case CHSET_GB18030:
            result = chset_gb18030_to_ucs2_str(dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_SJIS__
        case CHSET_SJIS:
            result = chset_sjis_to_ucs2((unsigned char *) dest_buff,dest_size,(unsigned char *) src_buff,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_EUCKR_WIN__
        case CHSET_EUCKR:
            result = chset_encode_decode_algo3(EUCKR_WINDOWS_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_THAI_WINDOWS__
        case CHSET_THAI_WIN:
            result = chset_encode_decode(THAI_WINDOWS_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_VIETNAMESE_WINDOWS__
        case CHSET_VIETNAMESE_WIN:
            result = chset_encode_decode(VIETNAMESE_WINDOWS_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_KOI8_R__
        case MMI_CHSET_KOI8_R:
            result = chset_encode_decode(KOI8_R_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

#ifdef __MMI_CHSET_TIS_620__
        case CHSET_TIS_620:
            result = chset_encode_decode(TIS_620_TO_UCS2,dest_buff,src_buff,dest_size,src_end_pos);
            break;

#endif

        /* For all rest of the encodings. */
        default:
#ifdef __MMI_CHSET_ALPHABET_SUPPORT__
            result =
                 chset_encode_decode((chset_pair_enum) (BASE_ENCODING_SCHEME + (src_type - CHSET_BASE) * 2 - 1),dest_buff,src_buff,dest_size,src_end_pos);

#else

            result = 0;
#endif

            break;
        }

    return result;
}













