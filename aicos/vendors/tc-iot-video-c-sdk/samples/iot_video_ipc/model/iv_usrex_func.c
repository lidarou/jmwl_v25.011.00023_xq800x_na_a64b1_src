/*
    用户实现本文件中定义的各个功能回调函数，即实现了具体的业务功能.
    回调函数实现注意事项:

    1.在回调函数中，不能做阻塞式操作，禁止在回调函数中做耗时的操作;
    2.在非回调函数中访问物模型对象资源，应该使用ivm_lock()/ivm_unlock()加锁和解锁;
    3.在回调函数处理中，不要进行加锁/解锁操作。（因为通讯线程已经加锁了）
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14cb04
#define LOG_MODULE    AMOD_VENDOR

#include <assert.h>
#include "iv_usrex.h"
#include "iv_cm.h"


int iv_usrcb_ProWritable_battery_percentage(const TYPE_DEF_TEMPLATE_INT *battery_percentage)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    printf(">>>>>>>>>>>>>>>>>>> %s: invoked! value %ld\n", __FUNCTION__, *battery_percentage);
    return 0;
}


int iv_usrcb_ProWritable_volume(const TYPE_DEF_TEMPLATE_ENUM *volume)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    printf(">>>>>>>>>>>>>>>>>>> %s: invoked! value %ld\n", __FUNCTION__, *volume);
    return 0;
}


int iv_usrcb_ProWritable_arming_switch(const TYPE_DEF_TEMPLATE_BOOL *arming_switch)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    return 0;
}


int iv_usrcb_ProWritable_sensitivity(const TYPE_DEF_TEMPLATE_ENUM *sensitivity)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    return 0;
}


int iv_usrcb_ProWritable_lock_state(const TYPE_DEF_TEMPLATE_ENUM *lock_state)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    return 0;
}


int iv_usrcb_ProWritable_inner_lock_state(const TYPE_DEF_TEMPLATE_BOOL *inner_lock_state)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    return 0;
}


int iv_usrcb_ProWritable_child_lock_state(const TYPE_DEF_TEMPLATE_BOOL *child_lock_state)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    return 0;
}


int iv_usrcb_Action_unlock_remote(ivm_unlock_remote_t *unlock_remote)
{
    //User implementation code
    //注意: 回调函数中,不能做阻塞式操作,不得做耗时的操作。会导致核心通讯线程阻塞!!!
    printf(">>>>>>>>>>>>>>>>>>> %s: invoked!\n", __FUNCTION__);
    return 0;
}

