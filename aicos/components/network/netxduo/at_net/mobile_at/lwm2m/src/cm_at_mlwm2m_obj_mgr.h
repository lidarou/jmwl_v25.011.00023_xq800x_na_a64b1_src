/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Object Manager used in cm lwm2m
 *
 */

#ifndef __CM_AT_MLWM2M_OBJ_MGR_H__
#define __CM_AT_MLWM2M_OBJ_MGR_H__

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus
/* Yes, C++ compiler is present.  Use standard C.  */
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_list.h"
#include "aic_lwm2m_client.h"
#include "aic_osa_ex.h"

/*******************************************
 *              DEFINES
 ******************************************/
#define LWM2M_AT_NNMI_CACHE_MAX_ENTRY_CNT 40
#define LWM2M_AT_NOTIFY_DATALEN_MAX 1024

/*******************************************
 *              TYPEDEFS
 ******************************************/
enum LWM2M_CM_RES_TYPE {
    LWM2M_CM_RES_TYPE_STRING = 1,
    LWM2M_CM_RES_TYPE_OPAQUE = 2,
    LWM2M_CM_RES_TYPE_INTEGER = 3,
    LWM2M_CM_RES_TYPE_FLOAT = 4,
    LWM2M_CM_RES_TYPE_BOOL = 5,
    LWM2M_CM_RES_TYPE_HEX_STR = 6,

    LWM2M_CM_RES_TYPE_TOTAL
};

typedef struct {
    u16 msg_id;
    u16 obj_id;
    s32 obj_ins_id;
    s32 res_id;
} LWM2M_CM_READ_REQ;

typedef struct {
    struct list_head node;
    LWM2M_CM_READ_REQ req;
} LWM2M_CM_READ_ENTRY;

typedef struct {
    struct list_head list;
} LWM2M_CM_READ_ENTRY_LIST;

typedef struct {
    struct list_head node;
    u16 res_id;
} LWM2M_CM_NOTIFY_ENTRY;

typedef struct {
    struct list_head list;
    u16 obj_id;
    u16 obj_ins_id;
    s32 count;
} LWM2M_CM_NOTIFY_ENTRY_LIST;

typedef struct {
    struct list_head node;
    u16 res_id;
    u8 *value;
    u32 value_len;
} LWM2M_CM_RES_ENTRY;

typedef struct {
    struct list_head list;
} LWM2M_CM_RES_ENTRY_LIST;

typedef struct {
    NX_LWM2M_CLIENT_OBJECT_INSTANCE object_instance;
    boolean is_available;
    LWM2M_CM_RES_ENTRY_LIST res_list;
} LWM2M_CM_OBJ_INS_ENTRY;

typedef struct {
    s32 count;
    LWM2M_CM_OBJ_INS_ENTRY *entries;
} LWM2M_CM_OBJ_INS_LIST;

typedef struct {
    struct list_head node;
    u16 objid;
    u32 atts;
    u32 acts;
    u16 *resid;
    u8 *res_type; /* one by one mapped to resid array, AIC_LWM2M_CLIENT_RESOURCE_xxx*/
    LWM2M_CM_OBJ_INS_LIST obj_ins_list;
} LWM2M_CM_OBJ_ENTRY;

typedef struct {
    struct list_head list;
    aos_mutex_ptr mutex;
} LWM2M_CM_OBJ_ENTRY_LIST;

/*******************************************
 *              EXTERNAL PROTOTYPES
 ******************************************/
/*----------------------------------------
    object entry list functions
----------------------------------------*/
void aic_lwm2m_cm_obj_entry_list_init(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, char *mutex_name);
void aic_lwm2m_cm_obj_entry_list_destory(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list);
u32 aic_lwm2m_cm_obj_entry_list_add(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 inscount,
                                    const char *bitmap, u32 acts, u32 atts);
void aic_lwm2m_cm_obj_entry_list_remove(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid);
void aic_lwm2m_cm_obj_entry_list_cleanup(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list);
u32 aic_lwm2m_cm_obj_entry_list_get_obj_atts(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u32 *atts);
u32 aic_lwm2m_cm_obj_entry_list_attach_resid(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 *resid, u32 atts);
/* DO NOT RELEASE instance list */
u32 aic_lwm2m_cm_obj_entry_list_get_instance_list(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid,
                                                  LWM2M_CM_OBJ_INS_LIST **instance_list);
/* DO NOT RELEASE instance entry */
u32 aic_lwm2m_cm_obj_entry_list_get_instance(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 insid,
                                             LWM2M_CM_OBJ_INS_ENTRY **ins_entry);
/* DO NOT RELEASE resid */
u32 aic_lwm2m_cm_obj_entry_list_get_res_id_list(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 **resid,
                                                u32 *atts, u32 *acts);
u32 aic_lwm2m_cm_obj_entry_list_set_res_value(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 obj_id, s32 obj_ins_id,
                                              s32 res_id, u8 type, u8 *value);
/* DO NOT RELEASE value */
u32 aic_lwm2m_cm_obj_entry_list_get_res_entry(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 obj_id, u16 obj_ins_id,
                                              u16 res_id, u8 **value, u32 *value_len, u8 *res_type);
u32 aic_lwm2m_cm_obj_entry_list_get_res_type(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 obj_id, u16 res_id, u8 *type);
/* even if the following api is an ugly interface, we still have to do that */
u32 aic_lwm2m_cm_add_all_instances_to_service(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list);
/* [Candidate API] create a new obj instance dedicated for WRITE-UPDATE operation */
u32 aic_lwm2m_cm_obj_entry_list_create_instance(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 insid,
                                                NX_LWM2M_CLIENT_RESOURCE *resource, u32 *resource_count);
/*----------------------------------------
    read entry list functions
----------------------------------------*/
void aic_lwm2m_cm_read_entry_list_init(LWM2M_CM_READ_ENTRY_LIST *read_entry_list);
u32 aic_lwm2m_cm_read_entry_list_add_req(LWM2M_CM_READ_ENTRY_LIST *read_entry_list, u16 mid, u16 obj_id, s32 obj_ins_id,
                                         s32 res_id);
u32 aic_lwm2m_cm_read_entry_list_get_req(LWM2M_CM_READ_ENTRY_LIST *read_entry_list, u16 mid, u16 obj_id,
                                         LWM2M_CM_READ_ENTRY **read_entry);
// static u32 aic_lwm2m_cm_read_entry_list_add_resp_entry(u16 obj_id, s32 obj_ins_id, s32 res_id, u8 type, u8 *value);
void aic_lwm2m_cm_read_entry_list_remove(LWM2M_CM_READ_ENTRY_LIST *read_entry_list, u16 mid, u16 obj_id);
void aic_lwm2m_cm_read_entry_list_cleanup(LWM2M_CM_READ_ENTRY_LIST *read_entry_list);
/*----------------------------------------
    notify entry list functions
----------------------------------------*/
void aic_lwm2m_cm_notify_entry_list_init(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list);
u32 aic_lwm2m_cm_notify_entry_list_add(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list, u16 obj_id, u16 obj_ins_id,
                                       u16 res_id);
boolean aic_lwm2m_cm_notify_entry_list_is_exist(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list, u16 obj_id,
                                                u16 obj_ins_id, u16 res_id);
void aic_lwm2m_cm_notify_entry_list_cleanup(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif
#endif /* __CM_AT_MLWM2M_OBJ_MGR_H__ */