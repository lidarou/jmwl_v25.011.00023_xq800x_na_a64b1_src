/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file sms_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_list.h>
#include <strings.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include "aic_at_cmd_handle.h"
#include "aic_at_interact_cp.h"

/*********************
 *      DEFINES
 *********************/
#define SRV_SMS_AT_CMD_MAX_LEN  (32)
#define SRV_SMS_PDU_STR_MAX_LEN (512)

/* 用户信息编码方式 */
#define GSM_7BIT        0
#define GSM_8BIT        4
#define GSM_UCS2        8

/**********************
 *      TYPEDEFS
 **********************/
/* 发送短信时传入的信息结构体 */
typedef struct
{
    /* sim卡的id，0-卡槽1，1-卡槽2，目前仅支持单卡（0） */
    uint8_t card_id;

    /* 短信要发送的目的号码 */
    char *p_dst_number;

    /* 发送的短信内容 */
    wchar_t *p_content;

    /* TODO: 后续可扩充成员 */
}demo_sms_send_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
extern size_t wcslen(const wchar_t *str);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// pSrc: 源字符串指针
// pDst: 目标数据指针
// nSrcLength: 源字符串长度
// 返回: 目标数据长度
static int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    for(int i=0; i<nSrcLength; i+=2)
    {
        // 输出高4位
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }

        pSrc++;

        // 输出低4位
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }
        pSrc++;
        pDst++;
    }

    // 返回目标数据长度
    return nSrcLength / 2;
}

// 字节数据转换为可打印字符串
// 如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
// pSrc: 源数据指针
// pDst: 目标字符串指针
// nSrcLength: 源数据长度
// 返回: 目标字符串长度
static int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";    // 0x0-0xf的字符查找表

    for(int i=0; i<nSrcLength; i++)
    {
        // 输出低4位
        *pDst++ = tab[*pSrc >> 4];

        // 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];

        pSrc++;
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标字符串长度
    return nSrcLength * 2;
}

// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
// 如："8613851872468" --> "683158812764F8"
// pSrc: 源字符串指针
// pDst: 目标字符串指针
// nSrcLength: 源字符串长度
// 返回: 目标字符串长度
static int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // 目标字符串长度
    char ch;          // 用于保存一个字符

    // 复制串长度
    nDstLength = nSrcLength;

    // 两两颠倒
    for(int i=0; i<nSrcLength;i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
        *pDst++ = *pSrc++;   // 复制后出现的字符
        *pDst++ = ch;        // 复制先出现的字符
    }

    // 源串长度是奇数吗？
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // 补'F'
        nDstLength++;        // 目标串长度加1
    }

    // 输出字符串加个结束符
    *pDst = '\0';

    // 返回目标字符串长度
    return nDstLength;
}

static int wchar_to_unicode(wchar_t *in, unsigned char *out)
{
    int index = 0;
    int i = 0;

    for (i = 0; i < wcslen(in); i++) {
        out[index] = (in[i] & 0xff00) >> 8;
        out[index + 1] = in[i] & 0xff;
        index += 2;
    }

    return index;
}

// UCS2编码
// pSrc: 源字符串指针
// pDst: 目标编码串指针
// nSrcLength: 源字符串长度
// 返回: 目标编码串长度
static int gsmEncodeUcs2(wchar_t* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nDstLength;        // UNICODE宽字符数目
    char char_out[256];      // UNICODE串缓冲区

    // 字符串-->UNICODE串
    nDstLength = wchar_to_unicode(pSrc, char_out);

    for(int i=0; i<nDstLength;i++)
    {
        *pDst++ = char_out[i];
    }

    // 返回目标编码串长度
    return nDstLength;
}

/**
* sms_encode_pdu - pdu编码接口.
* @p_send_info: [IN] 发送的短信结构体信息.
* @p_dst: [IN] 目标PDU串指针.
* Return: 目标PDU串长度.
*/
static int sms_encode_pdu(demo_sms_send_info_t *p_send_info, char* pDst)
{
    /* 内部用的串长度 */
    int nLength = 0;

    /* 目标PDU串长度 */
    int nDstLength = 0;

    /* 内部用的缓冲区 */
    unsigned char buf[256] = {0};
    AT_RESP_FROM_CP at_resp = {0};
    char dst_number[20] = {0};
    char smsc_number_old[20] = {0};
    char smsc_number[20] = {0};
    u8 *p_temp1_char = NULL;
    u8 *p_temp2_char = NULL;
    u8 smc_len = 0;
    u8 smsc_number_idx = 0;
    u8 temp_cnt = 0;

    /* 参数有效性检查 */
    if (NULL == p_send_info) {
        app_loge("[%s] p_send_info is NULL!\r\n", __func__);
        return -1;
    }

    /* 分配空间 */
    at_resp.resp_buf = aic_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_resp.resp_buf) {
        app_loge("[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_resp.buf_len = AT_MAX_AP_RESP_LENGTH;

    /* 第一步：编码短消息中心号码相关信息 */
    /* 获取短消息中心SMSC地址信息段 */
    aic_send_cmd_and_wait("at+csca?", &at_resp);

    /* 解析短消息中心号码 */
    p_temp1_char = strchr(at_resp.resp_buf, '\"');
    p_temp1_char++;
    p_temp2_char = strchr(p_temp1_char, '\"');
    smc_len = p_temp2_char - p_temp1_char;
    memcpy(smsc_number_old, p_temp1_char, smc_len);
    app_logi("[%s] smsc_number_old = %s, smc_len = %d.\r\n", __func__, smsc_number_old, smc_len);

    /* 过滤短消息中心号码中的+号 */
    for (temp_cnt = 0 ; temp_cnt < smc_len; temp_cnt++) {
        if ('+' == smsc_number_old[temp_cnt]) {
            continue;
        }
        smsc_number[smsc_number_idx] = smsc_number_old[temp_cnt];
        smsc_number_idx++;
    }
    app_logi("[%s] smsc_number = %s, smsc_number_idx = %d.\r\n", __func__, smsc_number, smsc_number_idx);

    nLength = strlen(smsc_number);
    /* SMSC地址信息长度 */
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;

    /* 固定: 用国际格式号码 */
    buf[1] = 0x91;

    /* 转换2个字节到目标PDU串 */
    nDstLength = gsmBytes2String(buf, pDst, 2);

    /* 转换SMSC到目标PDU串 */
    nDstLength += gsmInvertNumbers(smsc_number, &pDst[nDstLength], nLength);

    /* 第二步：编码TPDU段基本参数、目标地址等 */
    /* TP-DA地址字符串的长度 */
    sprintf(dst_number, "86%s", p_send_info->p_dst_number);
    nLength = strlen(dst_number);

    /* 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10) */
    buf[0] = 0x01;

    /* TP-MR=0 */
    buf[1] = 0;

    /* 目标地址数字个数(TP-DA地址字符串真实长度) */
    buf[2] = (char)nLength;

    /* 固定: 用国际格式号码 */
    buf[3] = 0x91;

    /* 转换4个字节到目标PDU串 */
    nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);

    /* 转换TP-DA到目标PDU串 */
    nDstLength += gsmInvertNumbers(dst_number, &pDst[nDstLength], nLength);

    /* 第三步：编码TPDU段协议标识、编码方式、用户信息等 */
    /* 用户信息字符串的长度 */
    nLength = 2 * wcslen(p_send_info->p_content);

    /* 协议标识(TP-PID) */
    buf[0] = 0;

    /* 用户信息编码方式(TP-DCS) */
    buf[1] = GSM_UCS2;

    /* UCS2编码方式：转换TP-DA到目标PDU串 */
    buf[2] = gsmEncodeUcs2(p_send_info->p_content, &buf[3], nLength);

    /* nLength等于该段数据长度 */
    nLength = buf[2] + 3;

    /* 转换该段数据到目标PDU串 */
    nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);

    app_logi("[%s] pDst = %s, nDstLength = %d.\r\n", __func__, pDst, nDstLength);

    aic_free(at_resp.resp_buf);

    /* 返回目标字符串长度 */
    return nDstLength;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
* aic_srv_tele_sms_send - 发送短信接口.
* @p_send_info: [IN] 发送短信的相关信息结构体.
* Return: 成功返回0，失败返回其它值.
*/
int demo_sms_send(demo_sms_send_info_t *p_send_info)
{
    u8 at_str[SRV_SMS_AT_CMD_MAX_LEN + 1] = {0};
    AT_RESP_FROM_CP at_resp = {0};
    u8 *p_pdu_str = NULL;
    int pdu_len = 0;
    /* SMSC串长度 */
    unsigned char nSmscLength = 0;

    app_logi("[%s] enter.\r\n", __func__);

    /* 参数有效性检查 */
    if (NULL == p_send_info) {
        app_loge("[%s] p_send_info is NULL!\r\n", __func__);
        return -1;
    }

    if (NULL == p_send_info->p_dst_number) {
        app_loge("[%s] dest number is NULL!\r\n", __func__);
        return -1;
    }
    app_logi("[%s] card_id = %d, dst_number = %s.\r\n",
              __func__,
              p_send_info->card_id,
              p_send_info->p_dst_number);

    at_resp.resp_buf = aic_zalloc(AT_MAX_AP_RESP_LENGTH + 1);
    if (NULL == at_resp.resp_buf) {
        app_loge("[%s] malloc failed!\r\n", __func__);
        return -1;
    }
    at_resp.buf_len = AT_MAX_AP_RESP_LENGTH;

    /* 当前默认以PDU模式进行编码及发送 */
    p_pdu_str = aic_zalloc(SRV_SMS_PDU_STR_MAX_LEN + 1);
    if (NULL == p_pdu_str) {
        app_loge("[%s] malloc p_pdu_str failed!\r\n", __func__);
        aic_free(at_resp.resp_buf);
        return -1;
    }

    /* PDU模式：首先要对字符串进行PDU编码转换 */
    pdu_len = sms_encode_pdu(p_send_info, p_pdu_str);

    /* 取PDU串中的SMSC信息长度 */
    gsmString2Bytes(p_pdu_str, &nSmscLength, 2);
    /* 加上长度字节本身 */
    nSmscLength++;

    /* 清空at_resp */
    memset(at_resp.resp_buf, 0, AT_MAX_AP_RESP_LENGTH);
    at_resp.buf_len = AT_MAX_AP_RESP_LENGTH;

    /* 生成命令，将PDU长度通过at命令进行下发 */
    sprintf(at_str, "AT+CMGS=%d", pdu_len / 2 - nSmscLength);

    /* 发送命令 */
    aic_send_cmd_and_wait(at_str, &at_resp);
    app_logi("[%s] cmgs recv resp %s.\r\n", __func__, at_resp.resp_buf);
    if (aic_is_message_medium_result(at_resp.resp_buf, strlen(at_resp.resp_buf))) {
        app_logi("[%s] cmgs resp contain >.\r\n", __func__);
        at_resp.resp_buf[0] = '\0';

        /* 发送短信内容 */
        *(p_pdu_str + pdu_len) = 0x1A;
        aic_send_msg_content_and_wait(p_pdu_str, (pdu_len + 1), &at_resp);

        if (4 <= strlen(at_resp.resp_buf)) {
            if (('O' == at_resp.resp_buf[strlen(at_resp.resp_buf) - 1 - 3])
                && ('K' == at_resp.resp_buf[strlen(at_resp.resp_buf) - 1 - 2])) {
                aic_free(at_resp.resp_buf);
                aic_free(p_pdu_str);
                app_logi("[%s] sms send success.\r\n", __func__);
                return 0;
            } else {
                aic_free(at_resp.resp_buf);
                aic_free(p_pdu_str);
                app_logi("[%s] sms send fail, and return -1.\r\n", __func__);
                return -1;
            }
        } else {
            aic_free(at_resp.resp_buf);
            aic_free(p_pdu_str);
            app_logi("[%s] sms send fail(strlen(at_resp.resp_buf) is error), and return -1.\r\n", __func__);
            return -1;
        }
    } else {
        aic_free(at_resp.resp_buf);
        aic_free(p_pdu_str);

        app_logi("[%s] AT+CMGS response no >.\r\n", __func__);
        return -1;
    }
}

/* 如下函数是app层对demo_sms_send接口的调用demo示例，用于发送短信，可参考 */
void app_demo_code_sms_send(void)
{
    int sms_ret = -1;
    demo_sms_send_info_t send_info = {0};
    char dst_number[32] = "130xxxxxxxx";
    wchar_t content[] = L"中国abcd啊！";

    send_info.p_dst_number = dst_number;
    send_info.p_content = content;
    sms_ret = demo_sms_send(&send_info);
    if (0 == sms_ret) {
        app_logi("[%s] sms send success.\r\n", __func__);
    } else {
        app_logi("[%s] sms send fail(sms_ret = %d).\r\n", __func__, sms_ret);
    }
}
