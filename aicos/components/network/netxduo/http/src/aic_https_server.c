/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Network Component
 *  Hypertext Transfer Protocol Secure (HTTPS using TLS) Server
 */

/* Force error checking to be disabled in this module */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17a29e
#define LOG_MODULE    AMOD_NET

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Enable HTTPS */
#ifndef NX_WEB_HTTPS_ENABLE
#define NX_WEB_HTTPS_ENABLE
#endif

#include <stdio.h>
#include <string.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nx_web_http_server.h>
#include <fs_api.h>
#include "aic_https_server.h"

u32 aic_https_server_create(NX_WEB_HTTP_SERVER *server_ptr,
                            AIC_HTTPS_SERVER_INFO *server_info,
                            u32 (*authentication_check)
                            (NX_WEB_HTTP_SERVER *server_ptr,
                            u32 request_type, char *resource, char **name,
                            char **password, char **realm),
                            u32 (*request_notify)
                            (NX_WEB_HTTP_SERVER *server_ptr,
                            u32 request_type, char *resource,
                            NX_PACKET *packet_ptr))
{
    NX_PACKET *packet_ptr;
    u32 status;
    u32 media_handle;

    if (server_ptr == NX_NULL || server_info == NX_NULL ||
        &server_info->ip_ptr == NX_NULL || server_info->stack_ptr == NX_NULL ||
        &server_info->pool_ptr == NX_NULL || server_info->work_path == NX_NULL){
        return NX_PTR_ERROR;
    }

    packet_ptr = server_info->pool_ptr->nx_packet_pool_available_list;

    if (((packet_ptr->nx_packet_data_end - packet_ptr->nx_packet_data_start) -
        NX_PHYSICAL_TRAILER) < AIC_HTTPS_SERVER_MIN_PACKET_SIZE)
        return NX_WEB_HTTP_POOL_ERROR;

    status = fs_media_handle_get(server_info->work_path, &media_handle);
    if (status) {
        aicx_error(0x0153ef, "", "get media ptr fail\n");
        return status;
    }

    server_info->media_ptr = (FX_MEDIA *)(&media_handle);

    status = nx_web_http_server_create(server_ptr,
                                       server_info->http_server_name,
                                       server_info->ip_ptr,
                                       server_info->server_port,
                                       server_info->media_ptr,
                                       server_info->stack_ptr,
                                       server_info->stack_size,
                                       server_info->pool_ptr,
                                       authentication_check, request_notify);

    return status;
}

u32 aic_https_server_delete(NX_WEB_HTTP_SERVER *server_ptr)
{
    u32 status;

    if (server_ptr == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_delete(server_ptr);

    return status;
}

u32 aic_https_server_cache_info_callback_set(
    NX_WEB_HTTP_SERVER *server_ptr, u32 (*cache_info_get)(char *, u32 *,
    NX_WEB_HTTP_SERVER_DATE *))
{
    if (server_ptr == NX_NULL || cache_info_get == NX_NULL)
        return NX_PTR_ERROR;

    /* Send internal. */
    return nx_web_http_server_cache_info_callback_set(server_ptr,
                                                      cache_info_get);
}

u32 aic_https_server_callback_data_send(NX_WEB_HTTP_SERVER *server_ptr,
                                        void *data_ptr, ulong data_length)
{
    u32 status;

    if(server_ptr == NX_NULL || data_ptr == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_callback_data_send(server_ptr, data_ptr,
                                                   data_length);

    return status;
}

u32 aic_https_server_callback_generate_response_header(
    NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
    AIC_HTTPS_SERVER_RESP *resp)
{
    u32 status;

    if (server_ptr == NX_NULL || resp == NX_NULL || packet_pptr == NX_NULL ||
        resp->status_code == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_callback_generate_response_header(server_ptr,
             packet_pptr, resp->status_code, resp->content_length,
             resp->content_type, resp->additional_header);

    return status;
}

u32 aic_https_server_callback_packet_send(NX_WEB_HTTP_SERVER *server_ptr,
                                          NX_PACKET *packet_ptr)
{
    if (server_ptr == NX_NULL || packet_ptr == NX_NULL)
        return NX_PTR_ERROR;

    return nx_web_http_server_callback_packet_send(server_ptr, packet_ptr);
}

u32 aic_https_server_callback_response_send(
    NX_WEB_HTTP_SERVER *server_ptr, AIC_HTTPS_SERVER_RESP *resp)
{
    u32 status;

    if(server_ptr == NX_NULL || resp==NX_NULL || resp->status_code == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_callback_response_send(server_ptr,
             resp->status_code, resp->information, resp->additional_info);

    return status;
}

u32 aic_https_server_content_get(NX_WEB_HTTP_SERVER *server_ptr,
                                 NX_PACKET *packet_ptr, ulong byte_offset,
                                 char *destination_ptr, u32 destination_size,
                                 u32 *actual_size)
{
    u32 status;

    if (server_ptr == NX_NULL || packet_ptr == NX_NULL ||
        destination_ptr == NX_NULL || actual_size == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_content_get_extended(server_ptr, packet_ptr,
             byte_offset, destination_ptr, destination_size, actual_size);

    return status;
}

u32 aic_https_server_content_length_get(NX_PACKET *packet_ptr,
                                        ulong *content_length)
{
    u32 status;

    if (packet_ptr == NX_NULL || content_length == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_content_length_get(packet_ptr, content_length);

    return status;
}

u32 aic_https_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr,
                                        NX_PACKET **packet_pptr,
                                        ulong *available_offset,
                                        ulong *available_length)
{
#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
    if (server_ptr == NX_NULL || packet_pptr == NX_NULL ||
        available_offset == NX_NULL || available_length == NX_NULL)
        return NX_PTR_ERROR;

    /* Check whether boundary is found. */
    if(server_ptr->nx_web_http_server_multipart.nx_web_http_server_multipart_boundary[0] == 0)
        return NX_WEB_HTTP_ERROR;

    return nx_web_http_server_get_entity_content(server_ptr, packet_pptr,
                                                 available_offset,
                                                 available_length);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(packet_pptr);
    NX_PARAMETER_NOT_USED(available_offset);
    NX_PARAMETER_NOT_USED(available_length);

    return NX_WEB_HTTP_ERROR;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
}

u32 aic_https_server_get_entity_header(
    NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
    char *entity_header_buffer, ulong buffer_size)
{
#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
    if (server_ptr == NX_NULL || packet_pptr == NX_NULL ||
        entity_header_buffer == NX_NULL)
        return NX_PTR_ERROR;

    return nx_web_http_server_get_entity_header(server_ptr, packet_pptr,
                                                entity_header_buffer,
                                                buffer_size);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(packet_pptr);
    NX_PARAMETER_NOT_USED(entity_header_buffer);
    NX_PARAMETER_NOT_USED(buffer_size);

    return NX_WEB_HTTP_ERROR;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
}

u32 aic_https_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr,
                                     void (*gmt_get)(NX_WEB_HTTP_SERVER_DATE *))
{
    if (server_ptr == NX_NULL || gmt_get == NX_NULL)
        return NX_PTR_ERROR;

    return nx_web_http_server_gmt_callback_set(server_ptr, gmt_get);
}

u32 aic_https_server_invalid_userpassword_notify_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*invalid_username_password_callback)(char *resource,
    NXD_ADDRESS *client_nxd_address, u32 request_type))
{
    u32 status;

    if (server_ptr == NX_NULL || invalid_username_password_callback == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_invalid_userpassword_notify_set(server_ptr,
             invalid_username_password_callback);

    return status;
}

u32 aic_https_server_mime_maps_additional_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, u32 mime_maps_num)
{
    if (server_ptr == NX_NULL || mime_maps == NX_NULL)
        return NX_PTR_ERROR;

    return nx_web_http_server_mime_maps_additional_set(server_ptr, mime_maps,
                                                       mime_maps_num);
}

u32 aic_https_server_response_packet_allocate(
    NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr,
    u32 wait_option)
{
    u32 status;

    if(server_ptr == NX_NULL || packet_ptr == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_response_packet_allocate(server_ptr, packet_ptr,
                                                         wait_option);

    return status;
}

u32 aic_https_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr,
                                         NX_PACKET **packet_ptr,
                                         ulong *content_length)
{
    if(server_ptr == NX_NULL || packet_ptr == NX_NULL || *packet_ptr == NX_NULL
       || content_length == NX_NULL)
        return NX_PTR_ERROR;

    return(nx_web_http_server_packet_content_find(server_ptr, packet_ptr,
                                                  content_length));
}

u32 aic_https_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr,
                                NX_PACKET **packet_ptr)
{
    if(server_ptr == NX_NULL || packet_ptr == NX_NULL)
        return NX_PTR_ERROR;

    return nx_web_http_server_packet_get(server_ptr, packet_ptr);

}

u32 aic_https_server_param_get(NX_PACKET *packet_ptr, u32 param_number,
                               char *param_ptr, u32 *param_size,
                               u32 max_param_size)
{
    u32    status;

    if (packet_ptr == NX_NULL || param_ptr == NX_NULL || param_size == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_param_get(packet_ptr, param_number, param_ptr,
                                          param_size, max_param_size);

    return status;
}

u32 aic_https_server_query_get(NX_PACKET *packet_ptr, u32 query_number,
                               char *query_ptr, u32 *query_size,
                               u32 max_query_size)
{
    u32 status;

    if (packet_ptr == NX_NULL || query_ptr == NX_NULL || query_size == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_query_get(packet_ptr, query_number, query_ptr,
                                          query_size, max_query_size);

    return status;
}

u32 aic_https_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr,
                                          u32 chunk_size, NX_PACKET *packet_ptr)
{
    u32 status;

    if (server_ptr == NX_NULL || packet_ptr == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_response_chunked_set(server_ptr, chunk_size,
                                                     packet_ptr);

    return status;
}

u32 aic_https_server_secure_configure(NX_WEB_HTTP_SERVER *server_ptr,
                                      const NX_SECURE_TLS_CRYPTO *crypto_table,
                                      AIC_HTTPS_SERVER_SECURE_CRT *server_crt,
                                      AIC_HTTPS_SERVER_TRUSTED_CRT *trusted_crt,
                                      AIC_HTTPS_SERVER_REMOTE_CRT *remote_crt)
{
    u32 status;

    if (server_ptr == NX_NULL || server_crt == NX_NULL ||
        crypto_table == NX_NULL || server_crt->tls_packet_buffer == NX_NULL ||
        server_crt->metadata_buffer == NX_NULL ||
        &server_crt->identity_certificate == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_secure_configure(server_ptr, crypto_table,
             server_crt->metadata_buffer, server_crt->metadata_size,
             server_crt->tls_packet_buffer, server_crt->tls_packet_buffer_size,
             &(server_crt->identity_certificate),
             trusted_crt->trusted_certificates, trusted_crt->trusted_certs_num,
             remote_crt->remote_certificates, remote_crt->remote_certs_num,
             remote_crt->remote_cert_buffer, remote_crt->remote_cert_buffer_size);

    return status;
}

u32 aic_https_server_start(NX_WEB_HTTP_SERVER *server_ptr)
{
    u32 status;

    if (server_ptr == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_start(server_ptr);

    return status;
}

u32 aic_https_server_stop(NX_WEB_HTTP_SERVER *server_ptr)
{
    u32    status;

    if (server_ptr == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_stop(server_ptr);

    return status;
}

u32 aic_https_server_type_get(NX_WEB_HTTP_SERVER *server_ptr,
                              AIC_HTTPS_SERVER_TYPE_INFO *type)
{
    u32 status;

    if (server_ptr == NX_NULL || type ==NX_NULL || type->name == NX_NULL ||
        type->http_type_string == NX_NULL || type->string_size == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_type_get(server_ptr, type->name,
                                          type->http_type_string,
                                          type->string_size);

    return status;
}

u32 aic_https_server_digest_authenticate_notify_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr,
    char *name_ptr, char *realm_ptr, char *password_ptr, char *method,
    char *authorization_uri, char *authorization_nc,
    char *authorization_cnonce))
{
    u32 status;

    if (server_ptr == NX_NULL || digest_authenticate_callback == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_digest_authenticate_notify_set( server_ptr,
             digest_authenticate_callback);

    return status;
}

u32 aic_https_server_authentication_check_set(
    NX_WEB_HTTP_SERVER *server_ptr,
    u32 (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr,
    u32 request_type, char *resource, char **name, u32 *name_length,
    char **password, u32 *password_length, char **realm, u32 *realm_length))
{
    u32 status;

    if (server_ptr == NX_NULL || authentication_check_extended == NX_NULL)
        return NX_PTR_ERROR;

    status = nx_web_http_server_authentication_check_set(server_ptr,
             authentication_check_extended);

    return status;
}

