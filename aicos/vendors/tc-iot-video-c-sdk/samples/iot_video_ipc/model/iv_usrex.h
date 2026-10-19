#ifndef __IVM_USREX_H_
#define __IVM_USREX_H_

#include "qcloud_iot_export.h"
#include "iv_dm.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
以下函数由开发者应用层去实现，实现具体的业务功能:
1. 云平台在接收到对设备的设置操作(更改ProWritable)或控制操作(Action)时,会主动通知设备;
2. 设备SDK通过iv_usrcb_ProWritable_xxx/iv_usrcb_Action_xxx系列函数通知应用层;
3. 函数的参数，即为对应物模型对象的指针;
*/
typedef struct {
    struct {
        TYPE_DEF_TEMPLATE_BOOL m_result;
    }action_out;
}ivm_unlock_remote_t;

typedef struct {
    TYPE_DEF_TEMPLATE_BOOL m_result;
} ivm_unlock_remote_result;

typedef struct {
    struct {
        TYPE_DEF_TEMPLATE_BOOL m_lock_motor_state;
    } ProReadonly;
    struct {
        TYPE_DEF_TEMPLATE_INT m_battery_percentage;
        TYPE_DEF_TEMPLATE_ENUM m_volume;
        TYPE_DEF_TEMPLATE_BOOL m_arming_switch;
        TYPE_DEF_TEMPLATE_ENUM m_sensitivity;
        TYPE_DEF_TEMPLATE_ENUM m_lock_state;
        TYPE_DEF_TEMPLATE_BOOL m_inner_lock_state;
        TYPE_DEF_TEMPLATE_BOOL m_child_lock_state;
    } ProWritable;
    struct {
        ivm_unlock_remote_result m_unlock_remote_result;
    } Event;
} ivm_extend_param_t;

extern   ivm_extend_param_t     g_ivm_objs;

/*!    \brief	物模型实例全局变量,应用层可直接访问/修改这个对象的成员变量。
        注意:
        1. 设备开发者可直接访问g_ivm_objs全局变量访问/修改物模型
        如：对一个int32类型的物模型对象yyyy的状态置值，应用层可直接操作：
        ```
        ivm_lock();		//互斥加锁
        g_ivm_objs.ProReadonly.yyyy = 1;		       //置状态值
        iv_dm_property_report(yyy, cb, param); //置状态
        ivm_unlock();	//互斥解锁
        ```
        2. 访问互斥
        * 在iv_usrcb_xxxxx系列回调函数中访问时g_ivm_objs时，无需加锁
        * 在其它任何地方访问g_ivm_objs，都应该调用ivm_lock()/ivm_unlock()作互斥处理
        3. 不得在iv_usrcb_xxxxx系列回调函数进行导致阻塞或耗时的操作，这样会阻塞核心通讯线程.
        4. 读写表示该属性即可从设备端上报到云端也可从云端发起控制，只读表示该属性只从设备向云端上报。
        */
/*! 
 \brief  更新物模型 --整形
 */ 
#define ivm_ProReadonly_setInt(obj, _val) \
   do {                                   \
       ivm_lock();                        \
       g_ivm_objs.ProReadonly.m_##obj = _val; \
       iv_dm_property_report(#obj, NULL, NULL);\
       ivm_unlock();                      \
   } while (0)

#define ivm_ProWritable_setInt(obj, _val) \
   do {                                   \
       ivm_lock();                        \
       g_ivm_objs.ProWritable.m_##obj = _val; \
       iv_dm_property_report(#obj, NULL, NULL);\
       ivm_unlock();                      \
   } while (0)
/*! 
 \brief  更新物模型 --字符串
 */ 
#define ivm_ProReadonly_setString(obj, _str)  \
   do {                                   \
       ivm_lock();                        \
       strncpy(g_ivm_objs.ProReadonly.m_##obj, _str, sizeof(g_ivm_objs.ProReadonly.m_##obj) - 1); \
       iv_dm_property_report(#obj, NULL, NULL);       \
       ivm_unlock();                      \
   } while (0)

#define ivm_ProWritable_setString(obj, _str)  \
   do {                                   \
       ivm_lock();                        \
       strncpy(g_ivm_objs.ProWritable.m_##obj, _str, sizeof(g_ivm_objs.ProWritable.m_##obj) - 1); \
       iv_dm_property_report(#obj, NULL, NULL);       \
       ivm_unlock();                      \
   } while (0)
/*! 
 \brief  更新结构体--字符串成员
 */ 
#define ivm_ProWritable_setStructString(obj, member, _str)  \
   do {                                   \
       strncpy(g_ivm_objs.ProWritable.m_##obj.m_##member, _str, sizeof(g_ivm_objs.ProWritable.m_##obj.m_##member) - 1); \
   } while (0)

int iv_usrcb_ProWritable_battery_percentage(const TYPE_DEF_TEMPLATE_INT *battery_percentage);
int iv_usrcb_ProWritable_volume(const TYPE_DEF_TEMPLATE_ENUM *volume);
int iv_usrcb_ProWritable_arming_switch(const TYPE_DEF_TEMPLATE_BOOL *arming_switch);
int iv_usrcb_ProWritable_sensitivity(const TYPE_DEF_TEMPLATE_ENUM *sensitivity);
int iv_usrcb_ProWritable_lock_state(const TYPE_DEF_TEMPLATE_ENUM *lock_state);
int iv_usrcb_ProWritable_inner_lock_state(const TYPE_DEF_TEMPLATE_BOOL *inner_lock_state);
int iv_usrcb_ProWritable_child_lock_state(const TYPE_DEF_TEMPLATE_BOOL *child_lock_state);

int iv_usrcb_Action_unlock_remote(ivm_unlock_remote_t *unlock_remote);

#ifdef __cplusplus
}
#endif


#endif
