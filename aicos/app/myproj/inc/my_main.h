#ifndef __MY_MAIN_H__
#define __MY_MAIN_H__

#define  MY_INIT_TASK_STACK_SIZE            2*1024
#define  MY_GPSRECV_TASK_STACK_SIZE         2*1024

// MQTT 2Kջ�ռ�
// MQTTS ����4KBջ�ռ�
#define  MY_MQTT_TASK_STACK_SIZE            6*1024

//���ȼ�����λ�ǰٷֱȣ�0%~100%,
//100%Ϊ��ߵȼ����ɾ���ʵ��ת�����ײ�SDK�õ����ȼ�
#define  MY_INIT_TASK_PRIORITY            40
#define  MY_GPSRECV_TASK_PRIORITY         40
#define  MY_MQTT_TASK_PRIORITY            35


typedef void (*CUSTOM_NW_EVT_CB)(u32 eventId, void* pData, u32 len);
extern void aic_custom_nw_reg_callback(u8 reg_type, CUSTOM_NW_EVT_CB callback);

extern void aic_network_setup_enhance_low_power(u32 enable, u32 idle_time, u32 retry_time);


extern volatile uint8 g_pdpState;

#endif

