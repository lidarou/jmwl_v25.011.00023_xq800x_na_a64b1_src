
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b23e3
#define LOG_MODULE    AMOD_VENDOR
#include "iv_usrex.h"
#include "iv_def.h"
/*-----------------data config start  -------------------*/ 

#define TOTAL_WR_PROPERTY_COUNT 7

static sDataPoint    sg_WR_DataTemplate[TOTAL_WR_PROPERTY_COUNT];

#define TOTAL_RO_PROPERTY_COUNT 1

static sDataPoint    sg_RO_DataTemplate[TOTAL_RO_PROPERTY_COUNT];

ivm_extend_param_t    g_ivm_objs;

static void _ivm_init_data_template(void)
{
    sg_RO_DataTemplate[0].data_property.data = &g_ivm_objs.ProReadonly.m_lock_motor_state;
    sg_RO_DataTemplate[0].data_property.key  = "lock_motor_state";
    sg_RO_DataTemplate[0].data_property.type = TYPE_TEMPLATE_BOOL;
    sg_RO_DataTemplate[0].state = eNOCHANGE;

    sg_WR_DataTemplate[0].data_property.data = &g_ivm_objs.ProWritable.m_battery_percentage;
    sg_WR_DataTemplate[0].data_property.key  = "battery_percentage";
    sg_WR_DataTemplate[0].data_property.type = TYPE_TEMPLATE_INT;
    sg_WR_DataTemplate[0].state = eNOCHANGE;

    sg_WR_DataTemplate[1].data_property.data = &g_ivm_objs.ProWritable.m_volume;
    sg_WR_DataTemplate[1].data_property.key  = "volume";
    sg_WR_DataTemplate[1].data_property.type = TYPE_TEMPLATE_ENUM;
    sg_WR_DataTemplate[1].state = eNOCHANGE;

    sg_WR_DataTemplate[2].data_property.data = &g_ivm_objs.ProWritable.m_arming_switch;
    sg_WR_DataTemplate[2].data_property.key  = "arming_switch";
    sg_WR_DataTemplate[2].data_property.type = TYPE_TEMPLATE_BOOL;
    sg_WR_DataTemplate[2].state = eNOCHANGE;

    sg_WR_DataTemplate[3].data_property.data = &g_ivm_objs.ProWritable.m_sensitivity;
    sg_WR_DataTemplate[3].data_property.key  = "sensitivity";
    sg_WR_DataTemplate[3].data_property.type = TYPE_TEMPLATE_ENUM;
    sg_WR_DataTemplate[3].state = eNOCHANGE;

    sg_WR_DataTemplate[4].data_property.data = &g_ivm_objs.ProWritable.m_lock_state;
    sg_WR_DataTemplate[4].data_property.key  = "lock_state";
    sg_WR_DataTemplate[4].data_property.type = TYPE_TEMPLATE_ENUM;
    sg_WR_DataTemplate[4].state = eNOCHANGE;

    sg_WR_DataTemplate[5].data_property.data = &g_ivm_objs.ProWritable.m_inner_lock_state;
    sg_WR_DataTemplate[5].data_property.key  = "inner_lock_state";
    sg_WR_DataTemplate[5].data_property.type = TYPE_TEMPLATE_BOOL;
    sg_WR_DataTemplate[5].state = eNOCHANGE;

    sg_WR_DataTemplate[6].data_property.data = &g_ivm_objs.ProWritable.m_child_lock_state;
    sg_WR_DataTemplate[6].data_property.key  = "child_lock_state";
    sg_WR_DataTemplate[6].data_property.type = TYPE_TEMPLATE_BOOL;
    sg_WR_DataTemplate[6].state = eNOCHANGE;

};

#define TOTAL_ACTION_COUNTS     (1)

static ivm_unlock_remote_t sg_unlock_remote;
static DeviceProperty g_actionInput_unlock_remote[] = {

};
static DeviceProperty g_actionOutput_unlock_remote[] = {
   {.key = "result", .data_buff_len = 0, .data = &sg_unlock_remote.action_out.m_result, .type = TYPE_TEMPLATE_BOOL},
};

static DeviceAction g_actions[]={
    {
     .pActionId = "unlock_remote",
     .timestamp = 0,
     .input_num = 0,
     .output_num = 1,
     .pInput = g_actionInput_unlock_remote,
     .pOutput = g_actionOutput_unlock_remote,
    },
};

#define EVENT_COUNTS     (1)

static DeviceProperty g_propertyEvent_unlock_remote_result[] = {
   {.key = "result", .data = &g_ivm_objs.Event.m_unlock_remote_result.m_result, .type = TYPE_TEMPLATE_BOOL},

};

static sEvent g_events[]={
    {
     .event_name = "unlock_remote_result",
     .type = "info",
     .timestamp = 0,
     .eventDataNum = 1,
     .pEventData = g_propertyEvent_unlock_remote_result,
    },
};

extern int ivm_init_Action(void *obj, int num, char *name, ivm_callback_Action cb);
extern int ivm_init_ProWritable(void *obj, int num, char *name, ivm_callback_ProWritable cb, char close_sync);
extern int ivm_init_ProReadonly(void *obj, int num, char *name);
extern int ivm_init_Event(void *obj, int num, char *name);

#define ivm_doi_init_ProReadonly(data) ivm_init_ProReadonly(sg_RO_DataTemplate, TOTAL_RO_PROPERTY_COUNT, #data)
#define ivm_doi_init_ProWritable(data, flag) ivm_init_ProWritable(sg_WR_DataTemplate, TOTAL_WR_PROPERTY_COUNT, #data, (ivm_callback_ProWritable)iv_usrcb_ProWritable_##data, flag)
#define ivm_doi_init_Action(data)  ivm_init_Action(g_actions, TOTAL_ACTION_COUNTS, #data, (ivm_callback_Action)iv_usrcb_Action_##data)
#define ivm_doi_init_Event(data) ivm_init_Event(g_events, EVENT_COUNTS, #data)


int ivm_env_init(void)
{
    memset((void *) &g_ivm_objs, 0, sizeof(ivm_extend_param_t));
    _ivm_init_data_template();

    ivm_doi_init_ProReadonly(lock_motor_state);
    ivm_doi_init_ProWritable(battery_percentage, 0);
    ivm_doi_init_ProWritable(volume, 0);
    ivm_doi_init_ProWritable(arming_switch, 0);
    ivm_doi_init_ProWritable(sensitivity, 0);
    ivm_doi_init_ProWritable(lock_state, 0);
    ivm_doi_init_ProWritable(inner_lock_state, 0);
    ivm_doi_init_ProWritable(child_lock_state, 0);

    ivm_doi_init_Action(unlock_remote);

    ivm_doi_init_Event(unlock_remote_result);

    return 0;
}
