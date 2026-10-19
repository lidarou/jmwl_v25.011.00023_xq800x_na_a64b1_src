/******************************************************************************/
/*                                                                            */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_HTTPS_SERVER_H__
#define __AIC_HTTPS_SERVER_H__

#include <aic_net_api.h>
#include <nx_web_http_server.h>

#ifndef AIC_HTTPS_SERVER_MIN_PACKET_SIZE
#define AIC_HTTPS_SERVER_MIN_PACKET_SIZE 600
#endif

#ifndef AIC_HTTPS_SERVER_SESSION_MAX
#define AIC_HTTPS_SERVER_SESSION_MAX 2
#endif

#define AIC_HTTPS_SERVER_ENABLE 1
#define AIC_HTTPS_SERVER_DISABLE 0

/* Define the HTTP Server data structure.  */
typedef struct aic_https_server_info
{
    u32 server_port;
    ulong stack_size;
    char *http_server_name;
    NX_IP *ip_ptr;
    FX_MEDIA *media_ptr;
    char *work_path;
    void *stack_ptr;
    NX_PACKET_POOL *pool_ptr;
} AIC_HTTPS_SERVER_INFO;

typedef struct aic_https_server_resp
{
    char *status_code;
    u32 content_length;
    char *content_type;
    char *additional_header;
    char *information;
    char *additional_info;
} AIC_HTTPS_SERVER_RESP;

typedef struct aic_https_server_secure_crt
{
    u32 tls_packet_buffer_size;
    ulong metadata_size;
    void *metadata_buffer;
    UCHAR* tls_packet_buffer;
    NX_SECURE_X509_CERT identity_certificate;
} AIC_HTTPS_SERVER_SECURE_CRT;

typedef struct aic_https_server_trusted_crt
{
    u32 trusted_certs_num;
    NX_SECURE_X509_CERT *trusted_certificates[];
} AIC_HTTPS_SERVER_TRUSTED_CRT;

typedef struct aic_https_server_remote_crt
{
    u32 remote_certs_num;
    u8 *remote_cert_buffer;
    u32 remote_cert_buffer_size;
    NX_SECURE_X509_CERT *remote_certificates[];
} AIC_HTTPS_SERVER_REMOTE_CRT;

typedef struct aic_https_server_type_info
{
    CHAR *name;
    CHAR *http_type_string;
    UINT *string_size;
} AIC_HTTPS_SERVER_TYPE_INFO;

u32 aic_https_server_create(NX_WEB_HTTP_SERVER *server_ptr,
                            AIC_HTTPS_SERVER_INFO *server_info,
                            u32 (*authentication_check)
                            (NX_WEB_HTTP_SERVER *server_ptr,
                            u32 request_type, char *resource, char **name,
                            char **password, char**realm),
                            u32 (*request_notify)
                            (NX_WEB_HTTP_SERVER *server_ptr,
                            u32 request_type, char *resource,
                            NX_PACKET *packet_ptr));

u32 aic_https_server_delete(NX_WEB_HTTP_SERVER *server_ptr);

u32 aic_https_server_cache_info_callback_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*cache_info_get)(char *, u32 *, NX_WEB_HTTP_SERVER_DATE *));

u32 aic_https_server_callback_data_send(
    NX_WEB_HTTP_SERVER *server_ptr, void *data_ptr, ulong data_length);

u32 aic_https_server_callback_packet_send(
    NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);

u32 aic_https_server_callback_generate_response_header(
    NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
    AIC_HTTPS_SERVER_RESP *resp);

u32 aic_https_server_content_get(NX_WEB_HTTP_SERVER *server_ptr,
                                 NX_PACKET *packet_ptr, ulong byte_offset,
                                 char *destination_ptr, u32 destination_size,
                                 u32 *actual_size);

u32 aic_https_server_content_length_get(NX_PACKET *packet_ptr,
                                        ulong *content_length);

u32 aic_https_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr,
                                        NX_PACKET **packet_pptr,
                                        ulong *available_offset,
                                        ulong *available_length);

u32 aic_https_server_get_entity_header(NX_WEB_HTTP_SERVER *server_ptr,
                                       NX_PACKET **packet_pptr,
                                       char *entity_header_buffer,
                                       ulong buffer_size);

u32 aic_https_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr,
                                    void (*gmt_get)(NX_WEB_HTTP_SERVER_DATE *));

u32 aic_https_server_invalid_userpassword_notify_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*invalid_username_password_callback)(char *resource,
    NXD_ADDRESS *client_nxd_address, u32 request_type));

u32 aic_https_server_mime_maps_additional_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, u32 mime_maps_num);

u32 aic_https_server_response_packet_allocate(
    NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr,
    u32 wait_option);

u32 aic_https_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr,
                                         NX_PACKET **packet_ptr,
                                         ulong *content_length);

u32 aic_https_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr,
                                NX_PACKET **packet_ptr);

u32 aic_https_server_param_get(NX_PACKET *packet_ptr, u32 param_number,
                               char *param_ptr, u32 *param_size,
                               u32 max_param_size);

u32 aic_https_server_query_get(NX_PACKET *packet_ptr, u32 query_number,
                               char *query_ptr, u32 *query_size,
                               u32 max_query_size);

u32 aic_https_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr,
                                         u32 chunk_size, NX_PACKET *packet_ptr);


/*u32 aic_https_server_secure_configure(NX_WEB_HTTP_SERVER *server_ptr,
                                          const NX_SECURE_TLS_CRYPTO *crypto_table,
                                          AIC_HTTPS_SECURE_CONFIGURE *config,
                                          AIC_REMOTE_CERTS *remote_certs);

u32 aic_https_server_secure_configure(NX_WEB_HTTP_SERVER *server_ptr,
                                      const NX_SECURE_TLS_CRYPTO *crypto_table,
                                      AIC_HTTPS_SECURE_CONFIGURE *config,
                                      NX_SECURE_X509_CERT *trusted_certificates[],
                                      u32 trusted_certs_num,
                                      NX_SECURE_X509_CERT *remote_certificates[],
                                      u32 remote_certs_num,
                                      u8 *remote_certificate_buffer,
                                      u32 remote_cert_buffer_size);*/
u32 aic_https_server_secure_configure(NX_WEB_HTTP_SERVER *server_ptr,
                                      const NX_SECURE_TLS_CRYPTO *crypto_table,
                                      AIC_HTTPS_SERVER_SECURE_CRT *server_crt,
                                      AIC_HTTPS_SERVER_TRUSTED_CRT *trusted_crt,
                                      AIC_HTTPS_SERVER_REMOTE_CRT *remote_crt);




u32 aic_https_server_start(NX_WEB_HTTP_SERVER *server_ptr);
u32 aic_https_server_stop(NX_WEB_HTTP_SERVER *server_ptr);

u32 aic_https_server_type_get(NX_WEB_HTTP_SERVER *server_ptr,
                              AIC_HTTPS_SERVER_TYPE_INFO *type);

u32 aic_https_server_digest_authenticate_notify_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr,
    char *name_ptr, char *realm_ptr, char *password_ptr, char *method,
    char *authorization_uri, char *authorization_nc,
    char *authorization_cnonce));

u32 aic_https_server_authentication_check_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr,
    u32 request_type, char *resource, char **name, u32 *name_length,
    char **password, u32 *password_length, char **realm, u32 *realm_length));

#endif

