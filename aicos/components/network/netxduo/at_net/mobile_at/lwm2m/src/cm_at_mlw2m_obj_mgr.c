/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Object Manager used in cm lwm2m
 *
 */

#define AIC_THIS_FILE    0x166f72
#define LOG_MODULE    AMOD_NET

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_at_tcpip_common.h"
#include "cm_at_mlwm2m_obj_mgr.h"

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/
static void aic_lwm2m_cm_obj_ins_entry_free(LWM2M_CM_OBJ_INS_ENTRY *obj_ins_entry);

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static void aic_lwm2m_cm_obj_ins_entry_free(LWM2M_CM_OBJ_INS_ENTRY *obj_ins_entry)
{
    LWM2M_CM_RES_ENTRY *entry = NULL;
    LWM2M_CM_RES_ENTRY *next_entry = NULL;

    /* free res entry list */
    list_for_each_entry_safe(entry, next_entry, &obj_ins_entry->res_list.list, node, LWM2M_CM_RES_ENTRY)
    {
        list_del(&entry->node);
        if (entry->value)
            aos_free(entry->value);
        aos_free(entry);
    }
}

/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
void aic_lwm2m_cm_obj_entry_list_init(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, char *mutex_name)
{
    list_head_init(&obj_entry_list->list);
    obj_entry_list->mutex = aos_create_mutex(mutex_name);
    AIC_ASSERT(obj_entry_list->mutex);
}

void aic_lwm2m_cm_obj_entry_list_destory(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list)
{
    aic_lwm2m_cm_obj_entry_list_cleanup(obj_entry_list);
    aos_delete_mutex(obj_entry_list->mutex);
    obj_entry_list->mutex = NULL;
}

u32 aic_lwm2m_cm_obj_entry_list_add(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 inscount,
                                    const char *bitmap, u32 acts, u32 atts)
{
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;
    LWM2M_CM_OBJ_ENTRY *new_obj_entry = NULL;
    LWM2M_CM_OBJ_INS_ENTRY *new_obj_ins_entries = NULL;

    /* compose new entry first */
    new_obj_entry = aos_zalloc(sizeof(LWM2M_CM_OBJ_ENTRY));
    if (!new_obj_entry) {
        aicx_error(0x10d203, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_add no memory");
        ret = AIC_LWM2M_CLIENT_NO_MEMORY;
        goto ERROR_EXIT;
    }
    if (inscount > 0) {
        size_t i = 0;

        if (strlen(bitmap) != inscount) {
            aicx_error(0x1ca931, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_add invalid bitmap");
            ret = AIC_LWM2M_CLIENT_NOT_ACCEPTABLE;
            goto ERROR_EXIT;
        }
        new_obj_ins_entries = aos_zalloc(sizeof(LWM2M_CM_OBJ_INS_ENTRY) * inscount);
        if (!new_obj_ins_entries) {
            aicx_error(0x0195b6, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_add no memory #2");
            ret = AIC_LWM2M_CLIENT_NO_MEMORY;
            goto ERROR_EXIT;
        }
        /* scan bitmap */
        for (i = 0; i < inscount; i++) {
            list_head_init(&new_obj_ins_entries[i].res_list.list);
            if (bitmap[i] == '1') {
                new_obj_ins_entries[i].is_available = AIC_TRUE;
            } else if (bitmap[i] == '0') {
                new_obj_ins_entries[i].is_available = AIC_FALSE;
            } else {
                aicx_error(0x126f77, "s", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_add invalid bitmap content %s", bitmap);
                ret = AIC_LWM2M_CLIENT_NOT_ACCEPTABLE;
                goto ERROR_EXIT;
            }
        }
        new_obj_entry->obj_ins_list.count = inscount;
        new_obj_entry->obj_ins_list.entries = new_obj_ins_entries;
    }
    /* As confirmed by CM, there is no meaning for acts so far. */
    new_obj_entry->acts = acts;
    new_obj_entry->atts = atts;
    new_obj_entry->objid = objid;
    /* in case objid already exist in the list, remove the old one first */
    aic_lwm2m_cm_obj_entry_list_remove(obj_entry_list, objid);
    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    /* add into list */
    list_add_tail(&new_obj_entry->node, &obj_entry_list->list);
    aos_put_mutex(obj_entry_list->mutex);

    return AIC_LWM2M_CLIENT_SUCCESS;

ERROR_EXIT:
    if (new_obj_entry) {
        aos_free(new_obj_entry);
    }
    if (new_obj_ins_entries) {
        aos_free(new_obj_ins_entries);
    }

    return ret;
}

void aic_lwm2m_cm_obj_entry_list_remove(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == objid) {
            /* removed from list */
            list_del(&entry->node);
            /* free memory */
            if (entry->resid) {
                aos_free(entry->resid);
            }
            if (entry->res_type) {
                aos_free(entry->res_type);
            }
            if (entry->obj_ins_list.entries) {
                s32 i = 0;

                for (i = 0; i < entry->obj_ins_list.count; i++) {
                    aic_lwm2m_cm_obj_ins_entry_free(&entry->obj_ins_list.entries[i]);
                }
                aos_free(entry->obj_ins_list.entries);
            }
            aos_free(entry);
            break;
        }
    }
    aos_put_mutex(obj_entry_list->mutex);
}

u32 aic_lwm2m_cm_obj_entry_list_get_obj_atts(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u32 *atts)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == objid) {
            *atts = entry->atts;
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

u32 aic_lwm2m_cm_obj_entry_list_attach_resid(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 *resid, u32 atts)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == objid) {
            /* free old one in case it was created */
            entry->atts = atts;
            if (entry->res_type)
                aos_free(entry->res_type);
            entry->res_type = aos_zalloc(sizeof(u8) * entry->atts);
            if (!entry->res_type) {
                aicx_error(0x13ad64, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_attach_resid no memory.");
                break;
            }
            if (entry->resid)
                aos_free(entry->resid);
            entry->resid = resid;
            /* assign res type */
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

/* DO NOT RELEASE instance list */
u32 aic_lwm2m_cm_obj_entry_list_get_instance_list(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid,
                                                  LWM2M_CM_OBJ_INS_LIST **instance_list)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == objid) {
            *instance_list = &entry->obj_ins_list;
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

/* DO NOT RELEASE instance entry */
u32 aic_lwm2m_cm_obj_entry_list_get_instance(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 insid,
                                             LWM2M_CM_OBJ_INS_ENTRY **ins_entry)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    LWM2M_CM_OBJ_INS_LIST *instance_list = NULL;
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;
    s32 i = 0;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == objid) {
            instance_list = &entry->obj_ins_list;
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }
    if (!instance_list) {
        goto EXIT;
    }
    for (i = 0; i < instance_list->count; i++) {
        if (instance_list->entries[i].is_available &&
            insid == instance_list->entries[i].object_instance.object_instance_id) {
            *ins_entry = &instance_list->entries[i];
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }

EXIT:
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

/* DO NOT RELEASE resid */
u32 aic_lwm2m_cm_obj_entry_list_get_res_id_list(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 **resid,
                                                u32 *atts, u32 *acts)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == objid) {
            if (!entry->resid && entry->atts > 0) {
                aicx_error(0x15840a, "d", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_get_res_id_list invalid node of obj entry(%d)",
                           objid);
                ret = AIC_LWM2M_CLIENT_ERROR;
            } else {
                *resid = entry->resid;
                *atts = entry->atts;
                *acts = entry->acts;
                ret = AIC_LWM2M_CLIENT_SUCCESS;
            }
            break;
        }
    }
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

u32 aic_lwm2m_cm_obj_entry_list_set_res_value(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 obj_id, s32 obj_ins_id,
                                              s32 res_id, u8 type, u8 *value)
{
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    LWM2M_CM_RES_ENTRY_LIST *res_entry_list = NULL;
    char *new_value = NULL;
    size_t len = 0;
    u8 *res_type = NULL;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    /* search res entry list first */
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == obj_id) {
            s32 i = 0;
            boolean is_valid_res = AIC_FALSE;

            /* check if the res id is valid one */
            if (entry->resid) {
                for (i = 0; i < entry->atts; i++) {
                    if (entry->resid[i] == (u16)res_id && entry->res_type) {
                        is_valid_res = AIC_TRUE;
                        res_type = &entry->res_type[i];
                        break;
                    }
                }
            }
            if (!is_valid_res) {
                /* add this resource id and resource type into obj mgr */
                u32 new_atts = entry->atts + 1;
                u16 *new_resid = NULL;
                u8 *new_restype = NULL;

                new_resid = aos_zalloc(new_atts * sizeof(u16));
                new_restype = aos_zalloc(sizeof(u8) * new_atts);
                aicx_info(0x045056, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value add new res");
                if (!new_resid || !new_restype) {
                    aicx_error(0x113597, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory.");
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    goto ERROR_BREAK;
                }
                /* realloc a new one, and old info needs to backup */
                if (entry->resid) {
                    aos_memcpy(new_resid, entry->resid, entry->atts * sizeof(u16));
                    aos_free(entry->resid);
                }
                new_resid[entry->atts] = res_id;
                entry->resid = new_resid;
                if (entry->res_type) {
                    aos_memcpy(new_restype, entry->res_type, entry->atts * sizeof(u8));
                    aos_free(entry->res_type);
                }
                entry->res_type = new_restype;
                res_type = &entry->res_type[entry->atts];
                entry->atts = new_atts;
                ret = AIC_LWM2M_CLIENT_SUCCESS;

            ERROR_BREAK:
                if (ret != AIC_LWM2M_CLIENT_SUCCESS) {
                    if (new_resid)
                        aos_free(new_resid);
                    if (new_restype)
                        aos_free(new_restype);
                    break;
                }
            }
            /* search into instance list */
            for (i = 0; i < entry->obj_ins_list.count; i++) {
                LWM2M_CM_OBJ_INS_ENTRY *ins_entry = &entry->obj_ins_list.entries[i];

                aicx_info(0x0d9918, "dddd", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value search ins -- ins_entry[%d] id is %d, "
                          "is_available is %d "
                          "obj_ins_id is %d",
                          i, ins_entry->object_instance.object_instance_id, ins_entry->is_available, obj_ins_id);
                if (ins_entry->is_available && ins_entry->object_instance.object_instance_id == obj_ins_id) {
                    res_entry_list = &ins_entry->res_list;
                    break;
                }
            }
            break;
        }
    }
    if (res_entry_list) {
        LWM2M_CM_RES_ENTRY *res_entry = NULL;

        ret = AIC_LWM2M_CLIENT_SUCCESS;
        /* search if there is old value existing */
        list_for_each_entry(res_entry, &res_entry_list->list, node, LWM2M_CM_RES_ENTRY)
        {
            if (res_entry->res_id == res_id) {
                /* remove it */
                list_del(&res_entry->node);
                if (res_entry->value) {
                    aos_free(res_entry->value);
                }
                aos_free(res_entry);
                break;
            }
        }
        /* create a new one */
        res_entry = aos_zalloc(sizeof(LWM2M_CM_RES_ENTRY));
        if (!res_entry) {
            ret = AIC_LWM2M_CLIENT_NO_MEMORY;
            aicx_error(0x1310d1, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #0.");
            goto EXIT;
        }
        res_entry->res_id = res_id;
        /* convert type and value to aic LWM2M known ones */
        aicx_info(0x09bc2c, "ds", "[LWM2M CM] add res -- type is %d. value is %s", type, value);
        switch (type) {
        case LWM2M_CM_RES_TYPE_STRING:
            {
                len = strlen(value);
                new_value = aos_zalloc(len + 1);
                if (!new_value) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    aicx_error(0x074728, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #1.");
                    break;
                }
                strcpy(new_value, value);
                res_entry->value = new_value;
                res_entry->value_len = len;
                *res_type = AIC_LWM2M_CLIENT_RESOURCE_STRING;
            }
            break;
        case LWM2M_CM_RES_TYPE_OPAQUE:
            {
                len = strlen(value);
                if (len & 0x01) {
                    ret = AIC_LWM2M_CLIENT_ERROR;
                    aicx_error(0x092f38, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value invalid value");
                    break;
                }
                new_value = aos_zalloc(len / 2 + 1);
                if (!new_value) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    aicx_error(0x07fb62, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #2.");
                    break;
                }
                aic_hexstring_to_char(new_value, value, len);
                res_entry->value = new_value;
                res_entry->value_len = len / 2;
                *res_type = AIC_LWM2M_CLIENT_RESOURCE_OPAQUE;
            }
            break;
        case LWM2M_CM_RES_TYPE_INTEGER:
            {
                new_value = aos_zalloc(sizeof(NX_LWM2M_INT32));
                if (!new_value) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    aicx_error(0x084731, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #3.");
                    break;
                }
                *(NX_LWM2M_INT32 *)new_value = atoi(value);
                res_entry->value = new_value;
                res_entry->value_len = sizeof(NX_LWM2M_INT32);
                *res_type = AIC_LWM2M_CLIENT_RESOURCE_INTEGER32;
                aicx_info(0x0d6b9f, "sd", "[LWM2M CM] value is %s, new_value is %d", value, *(NX_LWM2M_INT32 *)new_value);
            }
            break;
        case LWM2M_CM_RES_TYPE_FLOAT:
            {
                new_value = aos_zalloc(sizeof(NX_LWM2M_FLOAT64));
                if (!new_value) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    aicx_error(0x0081c2, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #4.");
                    break;
                }
                *(NX_LWM2M_FLOAT64 *)new_value = atof(value);
                res_entry->value = new_value;
                res_entry->value_len = sizeof(NX_LWM2M_FLOAT64);
                *res_type = AIC_LWM2M_CLIENT_RESOURCE_FLOAT64;
                aicx_info(0x174c41, "dLf", "[LWM2M CM] add res -- type is %d. new value is %Lf", type, *(NX_LWM2M_FLOAT64 *)value);
            }
            break;
        case LWM2M_CM_RES_TYPE_BOOL:
            {
                new_value = aos_zalloc(sizeof(NX_LWM2M_BOOL));
                if (!new_value) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    aicx_error(0x10031d, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #5.");
                    break;
                }
                *(NX_LWM2M_BOOL *)new_value = (NX_LWM2M_BOOL)atoi(value);
                res_entry->value = new_value;
                res_entry->value_len = sizeof(NX_LWM2M_BOOL);
                *res_type = AIC_LWM2M_CLIENT_RESOURCE_BOOLEAN;
            }
            break;
        case LWM2M_CM_RES_TYPE_HEX_STR:
            {
                len = strlen(value);
                if (len & 0x01) {
                    ret = AIC_LWM2M_CLIENT_ERROR;
                    aicx_error(0x092f38, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value invalid value");
                    break;
                }
                new_value = aos_zalloc(len / 2 + 1);
                if (!new_value) {
                    ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                    aicx_error(0x07fb62, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no memory #2.");
                    break;
                }
                aic_hexstring_to_char(new_value, value, len);
                res_entry->value = new_value;
                res_entry->value_len = len / 2;
                *res_type = AIC_LWM2M_CLIENT_RESOURCE_STRING;
            }
            break;
        default:
            ret = AIC_LWM2M_CLIENT_BAD_OPTION;
            break;
        }
        if (ret == AIC_LWM2M_CLIENT_SUCCESS) {
            list_add_tail(&res_entry->node, &res_entry_list->list);
        } else {
            if (res_entry) {
                aos_free(res_entry);
            }
            if (new_value) {
                aos_free(new_value);
            }
        }
    } else {
        aicx_error(0x18cfeb, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_set_res_value no available res list.");
    }

EXIT:
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

/* DO NOT RELEASE res_entry */
u32 aic_lwm2m_cm_obj_entry_list_get_res_entry(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 obj_id, u16 obj_ins_id,
                                              u16 res_id, u8 **value, u32 *value_len, u8 *res_type)
{
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;
    LWM2M_CM_OBJ_ENTRY *entry = NULL;

    /* search res entry list first */
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == obj_id) {
            s32 i = 0;
            boolean is_valid_res = AIC_FALSE;
            u8 type = AIC_LWM2M_CLIENT_RESOURCE_NONE;

            /* check if the res id is valid one */
            for (i = 0; i < entry->atts; i++) {
                if (entry->resid[i] == (u16)res_id && entry->res_type) {
                    is_valid_res = AIC_TRUE;
                    type = entry->res_type[i];
                    break;
                }
            }
            if (!is_valid_res)
                break;
            /* search into obj instance list */
            for (i = 0; i < entry->obj_ins_list.count; i++) {
                LWM2M_CM_OBJ_INS_ENTRY *ins_entry = &entry->obj_ins_list.entries[i];

                if (ins_entry->is_available && ins_entry->object_instance.object_instance_id == obj_ins_id) {
                    LWM2M_CM_RES_ENTRY *iter_res = NULL;

                    /* scan resource instance list */
                    list_for_each_entry(iter_res, &ins_entry->res_list.list, node, LWM2M_CM_RES_ENTRY)
                    {
                        if (iter_res->res_id == res_id && iter_res->value) {
                            ret = AIC_LWM2M_CLIENT_SUCCESS;
                            *value = iter_res->value;
                            *value_len = iter_res->value_len;
                            *res_type = type;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    return ret;
}

void aic_lwm2m_cm_obj_entry_list_cleanup(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list)
{
    LWM2M_CM_OBJ_ENTRY *entry = NULL;
    LWM2M_CM_OBJ_ENTRY *next_entry = NULL;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry_safe(entry, next_entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        list_del(&entry->node);
        if (entry->resid) {
            aos_free(entry->resid);
        }
        if (entry->res_type) {
            aos_free(entry->res_type);
        }
        if (entry->obj_ins_list.entries) {
            s32 i = 0;

            for (i = 0; i < entry->obj_ins_list.count; i++) {
                aic_lwm2m_cm_obj_ins_entry_free(&entry->obj_ins_list.entries[i]);
            }
            aos_free(entry->obj_ins_list.entries);
        }
        aos_free(entry);
    }
    aos_put_mutex(obj_entry_list->mutex);
}

u32 aic_lwm2m_cm_obj_entry_list_get_res_type(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 obj_id, u16 res_id, u8 *type)
{
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;
    LWM2M_CM_OBJ_ENTRY *entry = NULL;

    *type = AIC_LWM2M_CLIENT_RESOURCE_NONE;
    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    /* search res entry list first */
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        if (entry->objid == obj_id) {
            s32 i = 0;

            /* check if the res id is valid one */
            for (i = 0; i < entry->atts; i++) {
                if (entry->resid[i] == (u16)res_id && entry->res_type) {
                    *type = entry->res_type[i];
                    ret = AIC_LWM2M_CLIENT_SUCCESS;
                    break;
                }
            }
            break;
        }
    }
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

u32 aic_lwm2m_cm_add_all_instances_to_service(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list)
{
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;
    LWM2M_CM_OBJ_ENTRY *entry = NULL;

    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    list_for_each_entry(entry, &obj_entry_list->list, node, LWM2M_CM_OBJ_ENTRY)
    {
        s32 i = 0;
        u16 objid = 0;
        LWM2M_CM_OBJ_INS_LIST *obj_ins_list = NULL;

        obj_ins_list = &entry->obj_ins_list;
        objid = entry->objid;
        /* create obj instances */
        for (i = 0; i < obj_ins_list->count; i++) {
            u16 instance_id = (u16)i;
            if (obj_ins_list->entries[i].is_available) {
                ret = aic_lwm2m_client_obj_instance_add(
                    objid, (NX_LWM2M_CLIENT_OBJECT_INSTANCE *)&obj_ins_list->entries[i], &instance_id);
                if (ret != AIC_LWM2M_CLIENT_SUCCESS && ret != AIC_LWM2M_CLIENT_ALREADY_EXIST) {
                    aicx_error(0x1ab6dd, "dd", "[LWM2M CM] failed to add instance#%d, ret = %d", i, ret);
                }
            }
        }
    }
    aos_put_mutex(obj_entry_list->mutex);

    return ret;
}

u32 aic_lwm2m_cm_obj_entry_list_create_instance(LWM2M_CM_OBJ_ENTRY_LIST *obj_entry_list, u16 objid, u16 insid,
                                                NX_LWM2M_CLIENT_RESOURCE *resource, u32 *resource_count)
{
    u32 ret = AIC_LWM2M_CLIENT_SUCCESS;
    LWM2M_CM_OBJ_ENTRY *new_obj_entry = NULL;
    LWM2M_CM_OBJ_INS_ENTRY *new_obj_ins_entries = NULL;
    u16 *resid = NULL;
    u8 *res_type = NULL;
    u32 atts = *resource_count;
    u32 i = 0;

    /* compose new obj entry first */
    new_obj_entry = aos_zalloc(sizeof(LWM2M_CM_OBJ_ENTRY));
    if (!new_obj_entry) {
        aicx_error(0x0680f1, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_create_instance no memory");
        ret = AIC_LWM2M_CLIENT_NO_MEMORY;
        goto ERROR_EXIT;
    }
    new_obj_ins_entries = aos_zalloc(sizeof(LWM2M_CM_OBJ_INS_ENTRY));
    if (!new_obj_ins_entries) {
        aicx_error(0x000a47, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_create_instance no memory #2");
        ret = AIC_LWM2M_CLIENT_NO_MEMORY;
        goto ERROR_EXIT;
    }
    /* init candidate obj instance */
    list_head_init(&new_obj_ins_entries->res_list.list);
    new_obj_ins_entries->is_available = AIC_TRUE;
    new_obj_entry->obj_ins_list.count = 1;
    new_obj_entry->obj_ins_list.entries = new_obj_ins_entries;
    new_obj_entry->acts = 0;
    new_obj_entry->atts = atts;
    new_obj_entry->objid = objid;
    /* create resource ids and types */
    resid = aos_zalloc(sizeof(u16) * atts);
    res_type = aos_zalloc(sizeof(u8) * atts);
    if (!resid || !res_type) {
        aicx_error(0x13b2fd, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_create_instance no memory #3");
        ret = AIC_LWM2M_CLIENT_NO_MEMORY;
        goto ERROR_EXIT;
    }
    new_obj_entry->resid = resid;
    new_obj_entry->res_type = res_type;
    /* attach resource entry into obj instance */
    for (i = 0; i < atts; i++) {
        LWM2M_CM_RES_ENTRY *res_entry = NULL;
        u16 new_resid = 0;
        const u8 *opaque_value = NULL;
        UINT value_length = 0;

        aic_lwm2m_client_resource_info_get(&resource[i], &new_resid, NULL);
        resid[i] = new_resid;
        /* set type as none since we have no idea of exact type here */
        res_type[i] = AIC_LWM2M_CLIENT_RESOURCE_NONE;
        res_entry = aos_zalloc(sizeof(LWM2M_CM_RES_ENTRY));
        if (!res_entry) {
            ret = AIC_LWM2M_CLIENT_NO_MEMORY;
            aicx_error(0x0d0237, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_create_instance no memory #4");
            goto ERROR_EXIT;
        }
        list_add_tail(&res_entry->node, &new_obj_ins_entries->res_list.list);
        res_entry->res_id = new_resid;
        nx_lwm2m_client_resource_opaque_get(resource, (const VOID **)&opaque_value, &value_length);
        if (opaque_value) {
            res_entry->value = aos_zalloc(value_length);
            if (!res_entry->value) {
                ret = AIC_LWM2M_CLIENT_NO_MEMORY;
                aicx_error(0x0acd87, "", "[LWM2M CM] aic_lwm2m_cm_obj_entry_list_create_instance no memory #5");
                goto ERROR_EXIT;
            }
            res_entry->value_len = value_length;
        }
    }
    /* in case objid already exist in the list, remove the old one first */
    aic_lwm2m_cm_obj_entry_list_remove(obj_entry_list, objid);
    aos_get_mutex(obj_entry_list->mutex, AOS_WAIT_FOREVER);
    /* add into obj entry list */
    list_add_tail(&new_obj_entry->node, &obj_entry_list->list);
    aos_put_mutex(obj_entry_list->mutex);

    return AIC_LWM2M_CLIENT_SUCCESS;

ERROR_EXIT:
    if (res_type) {
        aos_free(res_type);
    }
    if (resid) {
        aos_free(resid);
    }
    if (new_obj_entry) {
        aos_free(new_obj_entry);
    }
    if (new_obj_ins_entries) {
        aic_lwm2m_cm_obj_ins_entry_free(new_obj_ins_entries);
        aos_free(new_obj_ins_entries);
    }
    return ret;
}

void aic_lwm2m_cm_read_entry_list_init(LWM2M_CM_READ_ENTRY_LIST *read_entry_list)
{
    list_head_init(&read_entry_list->list);
}

u32 aic_lwm2m_cm_read_entry_list_add_req(LWM2M_CM_READ_ENTRY_LIST *read_entry_list, u16 mid, u16 obj_id, s32 obj_ins_id,
                                         s32 res_id)
{
    LWM2M_CM_READ_ENTRY *new_entry = NULL;

    new_entry = aos_zalloc(sizeof(LWM2M_CM_READ_ENTRY));
    if (!new_entry)
        return AIC_LWM2M_CLIENT_NO_MEMORY;
    new_entry->req.msg_id = mid;
    new_entry->req.obj_id = obj_id;
    new_entry->req.obj_ins_id = obj_ins_id;
    new_entry->req.res_id = res_id;
    /* in case objid already exist in the list, remove the old one first */
    aic_lwm2m_cm_read_entry_list_remove(read_entry_list, mid, obj_id);
    /* add into list */
    list_add_tail(&new_entry->node, &read_entry_list->list);

    return AIC_LWM2M_CLIENT_SUCCESS;
}

u32 aic_lwm2m_cm_read_entry_list_get_req(LWM2M_CM_READ_ENTRY_LIST *read_entry_list, u16 mid, u16 obj_id,
                                         LWM2M_CM_READ_ENTRY **read_entry)
{
    u32 ret = AIC_LWM2M_CLIENT_NOT_FOUND;
    LWM2M_CM_READ_ENTRY *entry = NULL;

    list_for_each_entry(entry, &read_entry_list->list, node, LWM2M_CM_READ_ENTRY)
    {
        if (entry->req.obj_id == obj_id && entry->req.msg_id == mid) {
            *read_entry = entry;
            ret = AIC_LWM2M_CLIENT_SUCCESS;
            break;
        }
    }
    return ret;
}

void aic_lwm2m_cm_read_entry_list_remove(LWM2M_CM_READ_ENTRY_LIST *read_entry_list, u16 mid, u16 obj_id)
{
    LWM2M_CM_READ_ENTRY *entry = NULL;

    list_for_each_entry(entry, &read_entry_list->list, node, LWM2M_CM_READ_ENTRY)
    {
        if (entry->req.obj_id == obj_id && entry->req.msg_id == mid) {
            /* removed from list */
            list_del(&entry->node);
            aos_free(entry);
            break;
        }
    }
}

void aic_lwm2m_cm_read_entry_list_cleanup(LWM2M_CM_READ_ENTRY_LIST *read_entry_list)
{
    LWM2M_CM_READ_ENTRY *entry = NULL;
    LWM2M_CM_READ_ENTRY *next_entry = NULL;

    list_for_each_entry_safe(entry, next_entry, &read_entry_list->list, node, LWM2M_CM_READ_ENTRY)
    {
        list_del(&entry->node);
        aos_free(entry);
    }
}

void aic_lwm2m_cm_notify_entry_list_init(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list)
{
    list_head_init(&notify_entry_list->list);
}

u32 aic_lwm2m_cm_notify_entry_list_add(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list, u16 obj_id, u16 obj_ins_id,
                                       u16 res_id)
{
    LWM2M_CM_NOTIFY_ENTRY *new_entry = NULL;

    if (notify_entry_list->count > 0 &&
        (notify_entry_list->obj_id != obj_id || notify_entry_list->obj_ins_id != obj_ins_id)) {
        aicx_error(0x08b844, "dddd", "[LWM2M CM] aic_lwm2m_cm_notify_entry_list_add Invalid ids obj id(%d vs %d), ins id(%d vs %d)",
                   notify_entry_list->obj_id, obj_id, notify_entry_list->obj_ins_id, obj_ins_id);
        return AIC_LWM2M_CLIENT_ERROR;
    }
    /* check if exists */
    if (!aic_lwm2m_cm_notify_entry_list_is_exist(notify_entry_list, obj_id, obj_ins_id, res_id)) {
        new_entry = aos_zalloc(sizeof(LWM2M_CM_NOTIFY_ENTRY));
        if (!new_entry)
            return AIC_LWM2M_CLIENT_NO_MEMORY;
        new_entry->res_id = res_id;
        /* add into list */
        list_add_tail(&new_entry->node, &notify_entry_list->list);
        if (!notify_entry_list->count) {
            notify_entry_list->obj_id = obj_id;
            notify_entry_list->obj_ins_id = obj_ins_id;
        }
        notify_entry_list->count++;
    }

    return AIC_LWM2M_CLIENT_SUCCESS;
}

boolean aic_lwm2m_cm_notify_entry_list_is_exist(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list, u16 obj_id,
                                                u16 obj_ins_id, u16 res_id)
{
    boolean is_exist = AIC_FALSE;

    if (notify_entry_list->count && notify_entry_list->obj_id == obj_id &&
        notify_entry_list->obj_ins_id == obj_ins_id) {
        LWM2M_CM_NOTIFY_ENTRY *entry = NULL;
        list_for_each_entry(entry, &notify_entry_list->list, node, LWM2M_CM_NOTIFY_ENTRY)
        {
            if (entry->res_id == res_id) {
                is_exist = AIC_TRUE;
                break;
            }
        }
    }
    return is_exist;
}

void aic_lwm2m_cm_notify_entry_list_cleanup(LWM2M_CM_NOTIFY_ENTRY_LIST *notify_entry_list)
{
    LWM2M_CM_NOTIFY_ENTRY *entry = NULL;
    LWM2M_CM_NOTIFY_ENTRY *next_entry = NULL;

    list_for_each_entry_safe(entry, next_entry, &notify_entry_list->list, node, LWM2M_CM_NOTIFY_ENTRY)
    {
        list_del(&entry->node);
        aos_free(entry);
    }
    notify_entry_list->count = 0;
}
