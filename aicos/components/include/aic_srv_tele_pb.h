/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_tele_pb.h
 *
 */

#ifndef __AIC_SRV_TELE_PB_H__
#define __AIC_SRV_TELE_PB_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aic_srv_bus.h"
#include <aic_type.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/
#define TS_PB_MAX_NAME_LEN (40)
#define TS_PB_MAX_NUMBER_LEN (32)
#define TS_PB_INVALID_INDEX (0xffff)

#define TS_EVENT_PB_STRUCT_HEAD ts_event_pb_e event_id;
/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    TS_EVENT_PB_INIT_IND = 0x8 << 16,

    TS_EVENT_PB_WRITE_RESP,

    TS_EVENT_PB_READ_RESP,

    TS_EVENT_PB_GET_STATUS_RESP,

    TS_EVENT_PB_MAX
} ts_event_pb_e;

typedef struct {
    ts_event_pb_e event_id;
} ts_event_pb_t;

typedef struct {
    uint16_t total;
    uint16_t used;
    uint16_t first_index;
} ts_pb_status_t;

typedef struct {
    /* contact invalid if name[0] is '\0' */
    char name[TS_PB_MAX_NAME_LEN];
    char number[TS_PB_MAX_NUMBER_LEN];
    uint16_t index;
} ts_pb_info_t;

typedef struct {
    TS_EVENT_PB_STRUCT_HEAD /* TS_EVENT_PB_INIT_IND */
    int32_t err;
    uint8_t card_id;
} ts_pb_init_ind_t;

typedef struct {
    TS_EVENT_PB_STRUCT_HEAD /* TS_EVENT_PB_WRITE_RESP */
    int32_t err;
    uint8_t card_id;
    uint16_t written_index;
} ts_pb_write_resp_t;

typedef struct {
    TS_EVENT_PB_STRUCT_HEAD /* TS_EVENT_PB_READ_RESP */
    int32_t err;
    uint8_t card_id;
    ts_pb_info_t *p_contact;
    uint16_t count;
    bool end;
} ts_pb_read_resp_t;

typedef struct {
    TS_EVENT_PB_STRUCT_HEAD /* TS_EVENT_PB_GET_STATUS_RESP */
    int32_t err;
    uint8_t card_id;
    ts_pb_status_t pb_status;
} ts_pb_get_status_resp_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * aic_srv_tele_pb_register - tele phonebook service register function.
 * @p_bus_info: [IN] bus info.
 * @_srv_callback: [IN] callback function client passed toservic.
 *                 the callback function param: p_param for event struct point, size for struct size
 *                 current events id：
 *                 1. TS_EVENT_PB_INIT_IND
 *                 2. TS_EVENT_PB_WRITE_RESP
 *                 3. TS_EVENT_PB_READ_RESP
 *                 4. TS_EVENT_PB_GET_STATUS_RESP
 * Return: > 0 means handle of phonebook service client，< 0 means error.
 */
int32_t aic_srv_tele_pb_register(aic_srv_bus_info_t *p_bus_info, srv_func_callback _srv_callback);

/**
 * aic_srv_tele_pb_unregister - deregister inferface for tele phonebook service.
 * @client_handle: [IN] handle of phonebook service client.
 * Return: 0 means success, other means error.
 */
int32_t aic_srv_tele_pb_unregister(int32_t client_handle);

/**
 * aic_srv_tele_pb_get_status - rise a phonebook status request.
 * @card_id: [IN] sim card id，0- card 1，1- card 2
 * @_response_callback: [IN] callback function to notify result of this request,
 *                   if set to NULL notify pass to global srv_callback.
 * Return: 0 means success, other means error.
 */
int32_t aic_srv_tele_pb_get_status(uint8_t card_id, srv_func_callback _response_callback);

/**
 * aic_srv_tele_pb_write - rise a phonebook write request.
 * [NOTE]: APIs with suffix _sync are time-consuming.
 * @card_id: [IN] sim card id，0- card 1，1- card 2
 * @p_contact: [IN] contact info.
 * @_response_callback: [IN] callback function to notify result of this request,
 *                   if set to NULL notify pass to global srv_callback.
 * Return: 0 means success, other means error.
 */
int32_t aic_srv_tele_pb_write(uint8_t card_id, ts_pb_info_t *p_contact, srv_func_callback _response_callback);

/**
 * aic_srv_tele_pb_read - rise a phonebook read request.
 * @card_id: [IN] sim card id，0- card 1，1- card 2
 * @count: [IN] read contacts count.
 * @first_index: [IN] first contacts index to read from.
 * @_response_callback: [IN] callback function to notify result of this request,
 *                   if set to NULL notify pass to global srv_callback.
 * Return: 0 means success, other means error.
 */
int32_t aic_srv_tele_pb_read(uint8_t card_id, uint16_t count, uint16_t first_index, srv_func_callback _response_callback);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_SRV_TELE_PB_H__ */
