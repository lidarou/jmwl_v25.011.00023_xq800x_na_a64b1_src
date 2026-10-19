/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_str.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * @brief  常见unicode(UCS-2)转utf-8
 * @note   暂不支持UCS-4 (4字节)编码
 * @param  *src: 输入unicode指针(被转换成unsigned short类型进行识别)
 * @param  slen: 输出unicode字节数
 * @param  *dst: 输出utf-8缓冲区
 * @param  dlen: utf-8缓冲区长度
 * @retval <0:错误，>=0:输出utf-8字节数
 */
int str_unicode_to_utf8(unsigned char *src, int slen, unsigned char *dst,int dlen)
{
    int olen = 0;
    unsigned short *p = (unsigned short *)src;
    unsigned short unicode = 0;
    int len = slen / 2;

    if (!src || !dst || !dlen) {
        return -1;
    }

    // 长度必须为2的倍数，需转换成unsigned short类型
    if (slen % 2) {
        return -2;
    }

    while (len--) {
        unicode = *p++;
        if (unicode == 0) {
            break;
        }

         // 预留3个字节，不足则错误
        if (olen > (dlen-3)) {
            olen = -3;
            break;
        }
        if (unicode <= 0x0000007F) {
            // 1个字节
            // U-00000000 - U-0000007F:  0xxxxxxx
            *dst++ = (unicode & 0x7F);
            olen += 1;
        } else if (unicode >= 0x00000080 && unicode <= 0x000007FF) {
            // 2个字节
            // U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            *dst++ = ((unicode >> 6) & 0x1F) | 0xC0;
            *dst++ = (unicode & 0x3F) | 0x80;
            olen += 2;
        } else if (unicode >= 0x00000800 && unicode <= 0x0000FFFF) {
            // 3个字节
            // U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            *dst++ = ((unicode >> 12) & 0x0F) | 0xE0;
            *dst++ = ((unicode >>  6) & 0x3F) | 0x80;
            *dst++ = (unicode & 0x3F) | 0x80;
            olen += 3;
        } else {
            return -4;
        }
    };
    return olen;
}

/**
 * @brief  utf-8转unicode(UCS-2)
 * @note   暂不支持UCS-4 (4字节)编码
 * @param  src: 输入utf-8指针
 * @param  dst: 输出Unicode缓冲区
 * @param  dlen: Unicode缓冲区长度
 * @retval <0:错误，>=0:输出uinicode字节数
 */
int str_utf8_to_unicode(char *src, char *dst, int dlen)
{
    // 输出Unicode字节数
    int olen = 0;

    while (*src){
        // 预留2个字节，不足则错误
        if (olen > (dlen-2)) {
            olen = -1;
            break;
        }
        if (*src > 0x00 && *src <= 0x7F) {
            // 单字节UTF8字符（英文字母、数字）
            *dst++ = *src++;
            *dst++ = 0; //小端法表示，在高地址填补0
        } else if (((*src) & 0xE0) == 0xC0) {
            // 双字节UTF8字符
            char high = *src++;
            char low = *src++;
            if ((low & 0xC0) != 0x80) {
                // 检查是否为合法的UTF8字符
                return -1;
            }
            *dst++ = (high << 6) + (low & 0x3F);
            *dst++ = (high >> 2) & 0x07;
        } else if (((*src) & 0xF0) == 0xE0) {
            // 三字节UTF8字符
            char high   = *src++;
            char middle = *src++;
            char low    = *src++;
            if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80)) {
                // 检测是否合法
                return -1;
            }
            // 取出middle的低两位与low的低6位，组合成unicode字符的低8位
            *dst++ = (middle << 6) + (low & 0x3F);
            // 取出high的低四位与middle的中间四位，组合成unicode字符的高8位
            *dst++ = (high << 4) + ((middle >> 2) & 0x0F);
        } else {
            // 对于其他字节数的UTF8字符不进行处理 printf("[%s](%d) err\n",__func__,__LINE__);
            return -1;
        }
        olen +=2;
    }

    // unicode字符串后面，有两个\0
    if (olen > (dlen-2)) {
        olen = -1;
    } else {
        *dst++ = 0;
        *dst++ = 0;
    }
    return olen;
}

