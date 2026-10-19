/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_tool.c
**文件描述:        设备平台工具函数模块
**当前版本:        V1.0
*********************************************************************/
#include "my_common.h"


/************************************************************************
**函数名称:  get_str_to_ken_ex
**入口参数:  szInput            ---       输入的字符串
**           iPos               ---       分隔符的位置
             cSplit             ---       分隔符字符编码
             szOutBuf           ---       分割之后的输出字符
             iBuffLen           ---       缓冲区数据的长度
**出口参数:  无
**函数功能:  从指定的字符串中指定位置提取字符串
*************************************************************************/
bool get_str_to_ken_ex(char *szInput, uint16 iPos,char cSplit,char *szOutBuf, uint16 iBuffLen)
{
    uint16 i = 0;
    char   *c = szInput;
    char   *p = szOutBuf;
    bool bHasMore = 0;

    if (szInput == NULL)
        return 0;

    while (*c != '\0')
    {
        if (i == iPos && *c != cSplit)
        {
            *p++ = *c;

            // 防止溢出，后面还要往这个 szBuf[iBuffLen-1] 放入结尾符
            if (p >= (szOutBuf + iBuffLen - 1))
                break;
        }
        else if (i > iPos)
        {
            bHasMore = 1;
            break;
        }

        if (*c == cSplit)
        {
            i++;
        }

        c++;
    }

    *p = '\0';

    return bHasMore;
}


/************************************************************************
**函数名称:  myatof_ex
**入口参数:  str        ---   待转化的字符串
             data       ---   转化之后的数据结果
**出口参数:  无
**函数功能:  将字符串转化为浮点数
*************************************************************************/
void myatof_ex(char *str, double *data)
{
    if (data == NULL)
        return;

    if (str == NULL || str[0] == '\0')
    {
        *data = 0;
        return;
    }

    /* 用标准库 strtod 替换原手写解析: 更完整(支持科学计数法等),
     * 无需自行处理前导空格、符号、小数点与位数上限。保留原实现语义:
     * data==NULL 直接返回; str==NULL 或空串时 *data=0。*/
    *data = strtod(str, NULL);
}

// 参数：n=要转换的浮点数；a=小数位数；str=存放转换后的字符串；size=str 缓冲区总字节数
uint32 myftoa_ex(double n, uint8 a, char* str, uint32 size)
{
    if (str == NULL || size == 0)
        return 0;

    /* 用标准库 snprintf + 显式缓冲区长度, 彻底杜绝越界:
     * %.*f 以 a 为小数位数; snprintf 最多写 size-1 个字符并补 '\0',
     * 即使 n 为极大值/NaN/Inf 也只会安全截断, 绝不会溢出 str。*/
    snprintf(str, size, "%.*f", (int)a, n);

    return (uint32)strlen(str);
}



int my_atcmd_send_sync(char* cmd, char* pRsp, int rspLen, int timeout)
{
    AT_RESP_FROM_CP at_resp = {0};

    at_resp.buf_len  = rspLen;
    at_resp.resp_buf = pRsp;

    return aic_send_cmd_and_wait(cmd, &at_resp);
}


char *my_send_at_cmd(char *atcmd)
{
    static char pRsp[256] = { 0 };
    int ret = -1;

    my_log_printf(1, "Send AT cmd : %s", atcmd);

    memset(pRsp, 0, 256);

    ret = my_atcmd_send_sync(atcmd, pRsp, 256, 150);

    if (ret != 0)
    {
        my_log_printf(1, "%s returns ERROR! ret=%d", atcmd, ret);
        return NULL;
    }

    my_log_printf(1, "%s returns: [%s]", __func__, pRsp);
    return pRsp;
}


