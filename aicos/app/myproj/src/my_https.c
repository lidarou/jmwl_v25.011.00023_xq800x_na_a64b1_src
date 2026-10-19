/****************************************************************************
 * 文件：https_httpbin_example.c
 * 说明：
 *   - 仅使用 aic_https_client.h + aic_http_comm.h 提供的 API
 *   - 免费 HTTPS 测试服务：https://httpbin.org
 *   - 演示：
 *        1) HTTPS GET  https://httpbin.org/get
 *        2) HTTPS PUT  https://httpbin.org/put
 *
 * 适用于 ArtInChip Luban-Lite / NetX Duo HTTPS 客户端封装
 ****************************************************************************/
#include "my_common.h"

#include "aic_https_client.h"   /* HTTPS 客户端 API */
#include "aic_http_comm.h"      /* 共用 HTTP/HTTPS 实现 API */

/*------------------------------ 配置 ----------------------------------*/

/* 免费 HTTPS 测试地址 */
#define HTTPS_GET_URL   "https://httpbin.org/get"
#define HTTPS_PUT_URL   "https://httpbin.org/put"

#define HTTPS_SERVER_PORT   443

/* CA 证书路径（Luban-Lite 文件系统里放一份 httpbin / Let's Encrypt 根证书） */
#define HTTPS_CA_CERT_PATH  "/cert/ca/isrg_x1.pem"

/* 单次接收缓冲区大小 */
#define HTTPS_RECV_BUF_LEN  1024

/* 等待时间：直接用 SDK 定义的默认值 */
#define HTTPS_WAIT          HTTPS_DEFAULT_TIMEOUT

/*----------------------------------------------------------------------*/
/* 1. 创建并初始化 HTTPS 客户端                                          */
/*----------------------------------------------------------------------*/
static u32 https_example_client_create(void)
{
    u32 handle;
    u32 ret;

    /*
     * 创建一个 HTTPS 客户端实例
     * 返回非 0 = handle，0 = 失败
     */
    handle = aic_https_client_create((u8 *)"httpbin_https_client");
    if (handle == 0) {
        my_log_printf(1, "HTTPS: HTTPS client create failed");
        return 0;
    }
    my_log_printf(1, "HTTPS: HTTPS client created, handle=%u", handle);

// TODO: #ifdef CONFIG_FS
#if 0
    /*
     * 加载 CA 证书，用于校验服务器证书
     * crt_id：
     *   AIC_HTTPS_CA_CRT = 1
     */
    ret = aic_https_ca_crt_file_add(handle,
                                    (u8 *)HTTPS_CA_CERT_PATH,
                                    AIC_HTTPS_CA_CRT);
    if (ret != 0) {
        my_log_printf(1, "HTTPS: load CA cert failed, ret=%u", ret);
        aic_https_client_delete(handle);
        return 0;
    }
    my_log_printf(1, "HTTPS: CA cert loaded");
#else
    my_log_printf(1, "HTTPS: CONFIG_FS disabled, skip CA file");
#endif

    return handle;
}

/*----------------------------------------------------------------------*/
/* 2. HTTPS GET 示例                                                     */
/*----------------------------------------------------------------------*/
static void https_example_get(u32 handle)
{
    u32 ret;
    long rd;
    u8 buf[HTTPS_RECV_BUF_LEN];

    my_log_printf(1, "HTTPS: === HTTPS GET START ===");

    /*
     * 建立 TLS 安全连接
     * 内部完成：
     *   DNS -> TCP connect -> TLS handshake -> 证书校验
     */
    ret = aic_https_client_secure_connect(handle, HTTPS_WAIT);
    if (ret != 0) {
        my_log_printf(1, "HTTPS: secure connect failed, ret=%u", ret);
        return;
    }
    my_log_printf(1, "HTTPS: TLS connected");

    /*
     * 发送 GET 请求
     * username / password 传 NULL 表示不需要 Basic Auth
     */
    ret = aic_https_client_get_start(handle,
                                     (u8 *)HTTPS_GET_URL,
                                     NULL,
                                     NULL,
                                     HTTPS_SERVER_PORT,
                                     HTTPS_WAIT);
    if (ret != 0) {
        my_log_printf(1, "HTTPS: GET start failed, ret=%u", ret);
        return;
    }
    my_log_printf(1, "HTTPS: GET request sent");

    /*
     * 读取响应 body
     *
     * aic_https_client_response_data_get：
     *   返回值 > 0 : 本次读取字节数
     *   返回值 = 0 : 响应结束
     *   返回值 < 0 : 错误或 chunked 结束
     */
    while (1) {
        memset(buf, 0, sizeof(buf));

        rd = aic_https_client_response_data_get(handle,
                                                buf,
                                                sizeof(buf) - 1,
                                                HTTPS_WAIT);
        if (rd > 0) {
            my_log_printf(1, "HTTPS: GET recv %ld bytes", rd);
            printf("%s\r\n", buf);
        } else {
            my_log_printf(1, "HTTPS: GET recv done/err, rd=%ld", rd);
            break;
        }
    }

    my_log_printf(1, "HTTPS: === HTTPS GET END ===");
}

/*----------------------------------------------------------------------*/
/* 3. HTTPS PUT 示例                                                     */
/*----------------------------------------------------------------------*/
static void https_example_put(u32 handle)
{
    u32 ret;
    u8 *body = (u8 *)"{\"board\":\"ArtInChip\",\"value\":123}";
    ulong total = strlen((char *)body);

    my_log_printf(1, "HTTPS: === HTTPS PUT START ===");

    /* 1) TLS 连接 */
    ret = aic_https_client_secure_connect(handle, HTTPS_WAIT);
    if (ret != 0) {
        my_log_printf(1, "HTTPS: secure connect failed, ret=%u", ret);
        return;
    }

    /*
     * 2) 发起 PUT 请求头
     *    total_bytes 必须和后面 put_data 发送的总长度一致
     */
    ret = aic_https_client_put_start(handle,
                                     (u8 *)HTTPS_PUT_URL,
                                     NULL,
                                     NULL,
                                     HTTPS_SERVER_PORT,
                                     total,
                                     HTTPS_WAIT);
    if (ret != 0) {
        my_log_printf(1, "HTTPS: PUT start failed, ret=%u", ret);
        return;
    }
    my_log_printf(1, "HTTPS: PUT header sent, total_bytes=%lu", total);

    /*
     * 3) 发送 PUT body
     * 如果 body 很大，可以分多次调用 aic_https_client_put_data()
     */
    ret = aic_https_client_put_data(handle,
                                    body,
                                    total,
                                    HTTPS_WAIT);
    if (ret != 0) {
        my_log_printf(1, "HTTPS: PUT data failed, ret=%u", ret);
        return;
    }
    my_log_printf(1, "HTTPS: PUT body sent (%lu bytes)", total);

    /*
     * 4) 读取服务器返回的响应
     * httpbin 会把你发的 data 原样回显
     */
    while (1) {
        u8 buf[HTTPS_RECV_BUF_LEN];
        long rd;

        memset(buf, 0, sizeof(buf));
        rd = aic_https_client_response_data_get(handle,
                                                buf,
                                                sizeof(buf) - 1,
                                                HTTPS_WAIT);
        if (rd > 0) {
            my_log_printf(1, "HTTPS: PUT response %ld bytes", rd);
            printf("%s\r\n", buf);
        } else {
            my_log_printf(1, "HTTPS: PUT response done/err, rd=%ld", rd);
            break;
        }
    }

    my_log_printf(1, "HTTPS: === HTTPS PUT END ===");
}

/*----------------------------------------------------------------------*/
/* 4. 总入口                                                            */
/*----------------------------------------------------------------------*/
void https_httpbin_example(void)
{
    u32 handle;

    my_log_printf(1, "HTTPS: HTTPS example start");

    /* 创建客户端 */
    handle = https_example_client_create();
    if (handle == 0) {
        return;
    }

    /* GET 测试 */
    https_example_get(handle);

    /*
     * 如果你们的 HTTPS 栈不支持“一个 handle 连续发两个请求”，
     * 最稳妥做法是：删掉再建。
     */
    aic_https_client_delete(handle);

    handle = https_example_client_create();
    if (handle == 0) {
        return;
    }

    /* PUT 测试 */
    https_example_put(handle);

    /* 释放资源 */
    aic_https_client_delete(handle);

    my_log_printf(1, "HTTPS: HTTPS example done");
}

