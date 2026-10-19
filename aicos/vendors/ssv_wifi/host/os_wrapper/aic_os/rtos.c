/*
 * Copyright (c) 2021-2031, STONEOIM and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
/**---------------------------------------------------------------------------*
**                         Dependencies                                      *
**---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13cf12
#define LOG_MODULE    AMOD_VENDOR

#include <stdio.h>
#include <stdlib.h>

#include "rtos.h"

#include <log.h>

//#include "os_api.h"
//#include "mn_type.h"
#include "rtos_def.h"
#include <tx_api.h>
//#include "os_api.h"



//#include "threadx_os.h"

/**---------------------------------------------------------------------------*
**                         Compiler Flag                                      *
**---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

volatile ssv_type_u8 gOsFromISR = 0;

static ssv_type_u16 s_os_sem_count = 0;
//static OsTaskHandle s_thread_handle[10];
aos_timer_ptr s_timer_nov = NULL;
OsMsgQ s_test_MsgQ;
OsTaskHandle s_task_handle =NULL;
#if 0
static void OS_Thread_Callback(uint32 argc, void* argv)
{
    OsThread* pThread = NULL;

    if (argv){
        
        pThread = (OsThread*)argv;

        if (pThread->entry){
            pThread->entry(pThread->entry_param);
        }
    }

}
#endif
OS_APIs ssv_type_s32  PLATFORM_OS_Init( void )
{
    //gOsFromISR = 0;
    return OS_SUCCESS;
}

OS_APIs unsigned long OS_Random(void)
{
    //return random();
    int ret = 0;
    srand(aos_get_tickcount());
    ret = rand();
    //LOG_PRINTF("os_nov:OS_Random = %d!", ret);

    return ret;
}

OS_APIs void OS_Terminate( void )
{

}

/**
 * @Description: 
 *  释放之前分配的内存块。如果释放失败，系统将保存失败错误码，并且出现断言。可以调用函数 SCI_GetLastError 获得详细错误信息。释放后会将该指针置空。
 *  对于分配常驻系统的内存，不管大小，不论功能模块是 MMI/Ref/Layer1 还是 PS 都应该调用这个接口进行内存的分配
 * @Author: majinding
 * @param {void} *m 待释放的内存块指针。
 * @return {*}
 */


/**
 * @Description: 内存拷贝。从源内存块_SRC_PTR 向目的内存块 _DEST_PTR 拷贝_SIZE 个字节。
 * @Author: majinding
 * @param {void} *_DEST_PTR 指向目的内存块的指针
 * @param {const void} *_SRC_PTR 指向源内存块的指针
 * @param {uint32} _SIZE 拷贝的字节数
 * @return {*}
 */

/**
 * @Description: 设置内存块。用值_VALUE 填充_DEST_PTR 指向的内存块，填充长度为 _SIZE 个字节
 * @Author: majinding
 * @param {void} *_DEST_PTR 将被设置的内存块指针
 * @param {int} _VALUE 用来设置内存块的值
 * @param {uint32} _SIZE 将被设置的内存块的字节数
 * @return {*}
 */


/**
 * @Description: 
 * 禁止 IRQ 中断，并保存 IRQ 状态寄存器的值，当恢复 IRQ 中断时此值可用于恢复原来的状态。可实现嵌套调用。但最大嵌套次数不能超过 10 次。
 *  OS_EnterCritical 一定要与 OS_ExitCritical 配对使用，否则会造成死锁。禁止中断的时间必须要尽可能短（最多不能超过 1 毫秒）。
 * @Author: majinding
 * @param {*}
 * @return {*}
 */

static AOS_INTERRUPT_DECLARE 
ssv_type_u32 OS_EnterCritical()
{
 //   SCI_DisableIRQ();
    AOS_INTERRUPT_DISABLE;
    return 0;
}

/**
 * @Description: 
 * 函数功能:恢复 IRQ 中断，将禁止 IRQ 中断时保存的状态回写到 IRQ 状态寄存器中。 参数说明
 * 注意事项:SCI_DisableIRQ 一定要与 SCI_RestoreIRQ 配对使用，否则会造成死锁。
 * @Author: majinding
 * @param {*}
 * @return {*}
 */
void OS_ExitCritical(ssv_type_u32 val)
{
  //  SCI_RestoreIRQ();
  AOS_INTERRUPT_RESTORE
}

/**
 * @description: 使用指定的参数创建静态线程。此接口创建的线程所使用的内存资源固定为常驻系统的。
 * 表3-26 SCI_CreateThread
 * 参数名称 含义
 * thread_name 线程名称。
 * queue_name 消息队列名称。
 * entry 线程的实体函数。
 * argc 线程实体函数的第一个参数。
 * argv 线程实体函数的第二个参数。
 * stack_size 堆栈大小（单位：字节）。
 * queue_num 消息队列大小（单位：消息个数）。
 * 
 * priority 线程优先级，取值范围：
 * SCI_PRIORITY_KERNEL (0) //线程优先级（0 为最高优先级）
 * SCI_PRIORITY_TIME_CRITICAL (5)
 * SCI_PRIORITY_HIGHEST (10)
 * SCI_PRIORITY_ABOVE_NORMAL (15)
 * SCI_PRIORITY_NORMAL (20)
 * SCI_PRIORITY_BELOW_NORMAL (25)
 * SCI_PRIORITY_LOWEST (28)
 * SCI_PRIORITY_IDLE (31)
 * preempt 该线程是否可以抢占，一般和 priority 相等。取值范围请参考:
 * SCI_PREEMPT (0x1)
 * SCI_NO_PREEMPT (0x0)
 * 
 * auto_start 创建后是否自动运行。SCI_AUTO_START/SCI_DONOT_START
 * 返回值
 * 如果创建成功，返回线程的 ID。
 * 否则返回 SCI_INVALID_BLOCK_ID。
 * 注意事项
 * 对有生命周期的线程不应该调用此接口进行线程的创建。
 * 调用函数 SCI_GetLastError 可获得详细错误信息。

 * @param {*thread_name 线程名称。queue_name 消息队列名称 entry线程的实体函数。}
 * @return { BLOCK_ID 如果创建成功，返回线程的 ID。}
 */

void test_timer_callback(int param){
   // OsMsgQEntry test;
   // test.MsgCmd = 100;
    
  //  OS_MsgQEnqueue( s_test_MsgQ, &test, false);

   // LOG_PRINTF("os_nov:test_timer_callback send event %p!", &test);
}
//aos_create_thread
ssv_type_s32  OS_TaskCreate( TASK_FUNC task, const char *name, ssv_type_u32 stackSize, void *param, ssv_type_u32 pri, OsTaskHandle *taskHandle )
{
    OsThread *pThread = 0;
    aos_thread_id thread_id = AOS_INVALID_THREAD_ID;
    char queue_name[OS_TASK_QUEUE_NUM];
    char task_name[OS_TASK_QUEUE_NUM];

    static int s_task_count = 0;
    LOG_PRINTF("OS_TaskCreate, name %s, stacksize=%d, task=%p, pri=%d", name, stackSize, task, pri);

    if (name == NULL){
        return OS_FAILED;
    }
    
#if 1  
    if ((name == NULL) )
    {
        LOG_PRINTF("OS_TaskCreate:Invalid params!");
        snprintf(task_name, OS_TASK_QUEUE_NUM, "nov_task_%d", s_task_count);
    }else {
        strncpy(task_name, name, OS_TASK_QUEUE_NUM);
    }
#endif

    pThread = OS_MemAlloc(sizeof(OsThread));
    if (0 == pThread)
    {
        LOG_PRINTF("OS_TaskCreate:pThread Is NULL\r\n");
        return OS_FAILED;
    }
    pThread->entry = task;
    pThread->entry_param = param;

    LOG_PRINTF("OS_TaskCreate:OS_TaskCreate, create begin");
    
    snprintf(queue_name, OS_TASK_QUEUE_NUM, "%s_queue", task_name);
    
    LOG_PRINTF("OS_TaskCreate:pThread %p\r\n", pThread);
   thread_id = aos_create_thread((char *)name,
                                task,
                                NULL,stackSize,
                                1, queue_name,
                             //   0,NULL,
                               pri, AOS_AUTO_START);
   // thread_id = SCI_CreateThread(task_name, queue_name, OS_Thread_Callback, 1, pThread, stackSize, 10, priority, SCI_PREEMPT, SCI_AUTO_START);
    if (AOS_INVALID_THREAD_ID == thread_id)
    {
        LOG_PRINTF("OS_TaskCreate:Fail to create thread '%s'", task_name);
        return OS_FAILED;
    }

    s_task_count++;
    pThread->thread_id = thread_id;
    LOG_PRINTF("OS_TaskCreate:Create thread '%s',thread id = %#x", task_name, thread_id);

#if 0
    if (s_timer_nov == NULL){
        OS_MsgQCreate(&s_test_MsgQ, 10);
        s_timer_nov =  SCI_CreateTimer("rtos_nov_timer_p", test_timer_callback, 0, 1000, 1);
    }
    s_task_handle = pThread;
#endif
    if (taskHandle){
        *taskHandle = pThread;
    }
    //SCI_ResumeThread(thread_id);
    
    return OS_SUCCESS;
}

/**
 * @Description: 删除已经创建的、ID 为 thread_id 的线程，并释放相关资源。删除后不可再对此线程进行操作。
 * @Author: majinding
 * @param {BLOCK_ID} thread_id
 * @return {*} 如果创建成功，返回 SCI_SUCCESS,否则返回 SCI_ERROR.
 */
void OS_TaskDelete(OsTaskHandle taskHandle)
{
    OsThread *pThread = (OsThread *)taskHandle;
    if (pThread == NULL)
    {
        return;
    }
    //SCI_ASSERT(SCI_DeleteThread(pThread->thread_id) != SCI_ERROR);     
    aos_delete_thread(pThread->thread_id);
    OS_MemFree(pThread);
}



OS_APIs void OS_StartScheduler( void ){

}

ssv_type_u32  OS_Task_GetSelf(void *arg)
{
    return aos_identify_thread();
}

OS_APIs ssv_type_bool OS_TaskShallStop(void){
    return false;
}


/**
 * @Description: 挂起当前线程，挂起时长为 time_in_ms。
 * @Author: majinding
 * @param {uint32} time_in_ms 挂起当前线程的时间常数，单位为 Tick。目前 1Tick 等于 1 毫秒
 * @return {*}
 */
void OS_Sleep(uint32 time_in_ms)
{
    aos_sleep(time_in_ms);
}

/**
 * @Description: 获得系统加电后到当前的 Tick 数。在本系统中，1 Tick 等于 1 毫秒。
 * @Author: majinding
 * @param {*}
 * @return {*}返回从系统加电后到当前的 Tick 数。
 */
ssv_type_u32 OS_GetSysTick(void)
{
    return aos_get_tickcount();
}

/**
 * @Description: 创建互斥信号量
 * @Author: majinding
 * @param {const char} *name_ptr 互斥信号量名称字符串指针
 * @param {uint32} priority_inherit 说明创建的互斥信号量是否支持优先级继承。SCI_INHERIT：支持优先级继承。 SCI_NO_INHERIT：不支持优先级继承。
 * @return {*} 如果创建成功，返回互斥信号量指针。否则，返回 SCI_NULL。
 */
ssv_type_s32 OS_MutexInit( OsMutex *mutex , const char* owenr)
{
    aos_mutex_ptr mutex_p = 0;
    mutex_p = aos_create_mutex((char *)mutex);
   // mutex_p = aos_create_mutex(owenr, SCI_INHERIT);
    if (mutex_p)
    {
        if (mutex)
        {
            *mutex = mutex_p;
        }
        return OS_SUCCESS;
    }
    return OS_FAILED;
}

/**
 * @Description: 获取互斥信号量。
 * @Author: majinding
 * @param {SCI_MUTEX_PTR} mutex_ptr 已经创建好的互斥信号量指针
 * @param {uint32} wait_option 已经创建好的互斥信号量指针
 * SCI_NO_WAIT：表示不等待，从本函数立即返回，不管是否能够获得指定的信号量
 * SCI_WAIT_FOREVER：表示一直等待，直到能够获得指定的信号量。
 * 常数（1～0xFFFFFFE）表示等待的时间，单位 ms。
 * 
 * @return {*} 如果获取成功，返回 SCI_SUCCESS。否则返回 SCI_ERROR。
 */
void OS_MutexLock( OsMutex mutex )
{
    if (mutex)
    {
        aos_get_mutex(mutex, AOS_WAIT_FOREVER);

       
    }
}
/**
 * @Description: 释放互斥信号量。必须和获取互斥信号量 SCI_GetMutex 成对使用，同一个线程可以多次调用该函数。
 * @Author: majinding
 * @param {SCI_MUTEX_PTR} mutex_ptr 已经创建好的互斥信号量指针
 * @return {*}如果释放成功，返回 SCI_SUCCESS。否则返回 SCI_ERROR。
 */
void OS_MutexUnLock( OsMutex mutex )
{
    if (mutex){
        aos_put_mutex(mutex);
    }
}

/**
 * @Description: 删除互斥信号量。
 * @Author: majinding
 * @param {SCI_MUTEX_PTR} mutex_ptr 待删除的互斥信号量指针。
 * @return {*}如果释放成功，返回 SCI_SUCCESS。否则返回 SCI_ERROR。
 */
void OS_MutexDelete( OsMutex mutex )
{
    if (mutex){
        aos_delete_mutex(mutex);
    }
}

ssv_type_u32 OS_SemCntQuery( OsSemaphore Sem){
    OsSemaphore_t* pSem = (OsSemaphore_t*)Sem;
    
    if (pSem){
        return pSem->sem_count;
    }

    return 0;
}

/**
 * @Description: 创建计数信号量。
 * @Author: majinding
 * @param {const char} *name_ptr 计数信号量名称字符串指针。
 * @param {uint32} initial_count 计数信号量的初始值
 * @return {*}如果创建成功，返回计数信号量指针。 否则，返回 SCI_NULL。
 */
ssv_type_s32 OS_SemInit( OsSemaphore* Sem , ssv_type_u16 maxcnt , ssv_type_u16 cnt)
{
    char sem_name[OS_MSG_Q_NAME_LEN];
    OsSemaphore_t* pSem = NULL;
    aos_semaphore_ptr sem = NULL;
    
    if (Sem == NULL)
    {
        return OS_FAILED;
    }

    pSem = (OsSemaphore_t*)OS_MemAlloc(sizeof(OsSemaphore_t));
    if (NULL == pSem)
    {
        return OS_FAILED;
    }

    snprintf(sem_name, OS_MSG_Q_NAME_LEN, "rtos_nov_sem_p%d", s_os_sem_count);
    sem = aos_create_semaphore(sem_name, cnt);
  
    if (sem == NULL){
        __OS_MemFree(pSem);
        return OS_FAILED;
    }

    pSem->sem_count = cnt;
    pSem->sem = sem;
    
    *Sem = pSem;
    return OS_SUCCESS;
}

/**
 * @Description: 获取计数信号量，该信号量的计数值会减少 1。
 * @Author: majinding
 * @param {SCI_SEMAPHORE_PTR} sem_ptr 已经创建好的计数信号量指针。
 * @param {uint32} wait_option
 * 等待时间（单位为：毫秒）：
 * SCI_NO_WAIT：表示不等待，从本函数立即返回，不管是否能够获得指定的信号量。
 * SCI_WAIT_FOREVER：表示一直等待，直到能够获得指定的信号量。
 * 常数（1～0xFFFFFFE）表示等待的时间，单位 ms。
 * @return {*}如果获取成功，返回 SCI_SUCCESS。否则返回 SCI_ERROR。
 */
ssv_type_s32 OS_SemWait( OsSemaphore Sem , ssv_type_u16 timeout)
{
    OsSemaphore_t* pSem = (OsSemaphore_t*)Sem;
    uint32 wait_option = 0;
    uint32 status = 0;
    
    if (pSem){
        if (timeout == 0) 
            wait_option =  AOS_WAIT_FOREVER;
        else 
            wait_option = timeout;
        status = aos_get_semaphore(pSem->sem, wait_option);
       
  
        if (status == AOS_SUCCESS)
        {
            pSem->sem_count++;
            return OS_SUCCESS;
        }
    }
    
    return OS_FAILED;
}

/**
 * @Description: 释放计数信号量，该信号量的计数值会增加 1。
 * @Author: majinding
 * @param {SCI_SEMAPHORE_PTR} sem_ptr 已经创建好的计数信号量指针
 * @return {*}如果释放成功，返回 SCI_SUCCESS。否则返回 SCI_ERROR。
 */
ssv_type_u8 OS_SemSignal( OsSemaphore Sem)
{
    OsSemaphore_t* pSem = (OsSemaphore_t*)Sem;
    //uint32 status = 0;
    if (!pSem){
      return OS_FAILED;
    }
    aos_put_semaphore(pSem->sem);
       
       
    return OS_SUCCESS;
}

/**
 * @Description: 删除计数信号量。
 * @Author: majinding
 * @param {SCI_SEMAPHORE_PTR} sem_ptr 待删除的计数信号量指针。
 * @return {*}如果删除成功，返回 SCI_SUCCESS。否则返回 SCI_ERROR。
 */
void OS_SemDelete(OsSemaphore Sem)
{
    OsSemaphore_t* pSem = (OsSemaphore_t*)Sem;
//    uint32 status = 0;
    if (pSem){
          aos_delete_semaphore(pSem->sem);
       
        __OS_MemFree(pSem);
    }
    

}

ssv_type_u8 OS_SemSignal_FromISR( OsSemaphore Sem){
    return OS_SemSignal(Sem);
}


void OS_MsDelay(ssv_type_u32 ms){
    aos_sleep(ms);
}

void SSV_OS_TickDelay(ssv_type_u32 ticks){
    OS_MsDelay(ticks*TICK_RATE_MS);
}


/* Timer: */
ssv_type_s32 OS_TimerCreate( OsTimer *timer, ssv_type_u32 ms, ssv_type_u8 autoReload, void *args, OsTimerHandler timHandler ){
    return OS_SUCCESS;
}

ssv_type_s32 OS_TimerSet( OsTimer timer, ssv_type_u32 ms, ssv_type_u8 autoReload, void *args ){
    return OS_SUCCESS;
}

ssv_type_s32 OS_TimerStart( OsTimer timer ){
    return OS_SUCCESS;
}

ssv_type_s32 OS_TimerStop( OsTimer timer ){
    return OS_SUCCESS;
}

void *OS_TimerGetData( OsTimer timer ){
    return NULL;
}

#if 1 //def CONFIG_OS_RTOS_THREADX

typedef struct {
    TX_QUEUE msg_queue;
    u8 *msg_buf;
}os_msg_threadx;

/* msg_len default 64 byte 16_ULONG */
void * threadx_create_msg(char *name, int msg_len, int queue_len)
{
    os_msg_threadx *threadx_msg = NULL;
    u32 ret = 0;

    threadx_msg = OS_MemAlloc(sizeof(os_msg_threadx));
    if (!threadx_msg)
    {
        LOG_PRINTF("threadx_os_create_msgqueue fail 1.\r\n");
        return NULL;
    }
    
    threadx_msg->msg_buf = OS_MemAlloc(msg_len*queue_len);
    if (!threadx_msg->msg_buf)
    {
        OS_MemFree(threadx_msg);
        LOG_PRINTF("threadx_os_create_msgqueue fail 2.\r\n");
        return NULL;
    }
    LOG_PRINTF("tx_queue_create name = %s, msg_len = %d, queue_len = %d\r\n", name, msg_len, queue_len);
	ret = tx_queue_create(&threadx_msg->msg_queue, name, msg_len/4, threadx_msg->msg_buf, msg_len*queue_len);
    if (ret != TX_SUCCESS)
    {
        OS_MemFree(threadx_msg->msg_buf);
        OS_MemFree(threadx_msg);
        LOG_PRINTF("tx_queue_create fail. ret = %d\r\n", ret);
        return NULL;
    }
	
	return threadx_msg;
}

/* send is forever   or  no wait  default, other not support*/
u32 threadx_sendmsg(void* mq, const char *msg, int wait_option)
{
	ULONG wait_flag = 0;
    os_msg_threadx *threadx_msg = mq;
    u32 ret = 0;
    if (!threadx_msg)
    {
        return OS_FAILED;
    }
	if (wait_option == 0xffffffff)
		wait_flag = TX_WAIT_FOREVER;
	else if (wait_option == 0)
		wait_flag = TX_NO_WAIT;
	else 
		wait_flag = wait_option;
	ret = tx_queue_send(&threadx_msg->msg_queue, (void *)msg, wait_flag);
    if (ret != TX_SUCCESS)
    {
        if (ret == TX_QUEUE_FULL)
        {
            
        }
        else 
        {
              LOG_PRINTF("tx_queue_send fail. ret = %d\r\n", ret);
        }
        
        return OS_FAILED;
    }
    
    return OS_SUCCESS;
}

u32 threadx_receivemsg(void* mq, char *buf, int wait_option)
{
	ULONG wait_flag = 0;
    os_msg_threadx *threadx_msg = mq;
    u32 ret = 0;
    if (!threadx_msg)
    {
        LOG_PRINTF("theadx_os_receivemsg threadx_msg is null.\r\n");
        return OS_FAILED;
    }
	if (wait_option == 0xffffffff)
		wait_flag = TX_WAIT_FOREVER;
	else if (wait_option == 0)
		wait_flag = TX_NO_WAIT;
	else {
		wait_flag = wait_option;
	}

	ret = tx_queue_receive(&threadx_msg->msg_queue, buf, wait_flag);
	if (ret != TX_SUCCESS)
    {
        if (ret == TX_QUEUE_EMPTY)
        {
            return OS_FAILED;
        }
        
        LOG_PRINTF("theadx_os_receivemsg ret is %d.\r\n", ret);
        return OS_FAILED;
	}
        
	return OS_SUCCESS;
}

u32 threadx_deletemsg(void *mq)
{	
    u32 ret = 0;
    os_msg_threadx *threadx_msg = mq;
    if (!threadx_msg)
    {
        return OS_FAILED;
    }

	ret =  tx_queue_delete(&threadx_msg->msg_queue);
    OS_MemFree(threadx_msg->msg_buf);
    OS_MemFree(threadx_msg);
    
	return (ret != TX_SUCCESS)? OS_FAILED : OS_SUCCESS;
}

u32 threadx_getmsg_size(void *mq)
{	
    u32 ret = 0;
    ULONG enqueued = 0;
    os_msg_threadx *threadx_msg = mq;
    if (!threadx_msg)
    {
        return 0;
    }

	ret =  tx_queue_info_get(&threadx_msg->msg_queue, TX_NULL, &enqueued, TX_NULL, TX_NULL, TX_NULL, TX_NULL);
    if (ret != TX_SUCCESS)
    {
        return 0;
        
    }
	return enqueued;
}


#endif
 char g_name[OS_MSG_Q_NAME_LEN];
/* Message Queue: */
ssv_type_s32 OS_MsgQCreate(OsMsgQ *MsgQ, ssv_type_u32 QLen )
{
    //char name[OS_MSG_Q_NAME_LEN];
    static int s_msgQ_count = 0;
    void* msg_Q = NULL;
    
    if (MsgQ == NULL) {
        return OS_FAILED;
    }

    s_msgQ_count++;
    snprintf(g_name, OS_MSG_Q_NAME_LEN, "msgq_%d", s_msgQ_count);

    msg_Q = threadx_create_msg(g_name, sizeof(OsMsgQEntry), QLen);
    if (msg_Q == NULL)
    {
        return OS_FAILED;
    }

    *MsgQ = msg_Q;

    return OS_SUCCESS;
}

ssv_type_s32 OS_MsgQDelete( OsMsgQ MsgQ)
{
    if (MsgQ)
    {
        threadx_deletemsg(MsgQ);
    }
    
    return OS_FAILED;
}

ssv_type_s32 OS_MsgQEnqueue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_bool fromISR )
{
    ssv_type_u32 ret = 0;
    int wait_option = 0xffffffff;
    if (MsgQ && MsgItem)
    {
         if (fromISR) wait_option = 0;
         ret = threadx_sendmsg(MsgQ, (const char *)MsgItem, wait_option);
         if (ret == OS_SUCCESS)
         {
               return OS_SUCCESS;
         }
   }

    return OS_FAILED;
}

ssv_type_s32 OS_MsgQDequeue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_u32 timeOut, ssv_type_bool fromISR )
{
    ssv_type_u32 ret = 0;
    int wait_option = 0xffffffff;

    if (MsgQ && MsgItem)
    {
        if (fromISR) 
        {
            wait_option = 0;
        }
        else if (timeOut == 0) 
        {
            wait_option = 0xffffffff;
        }
        else
        {
            wait_option = timeOut;
        }
        ret = threadx_receivemsg(MsgQ, (char *)MsgItem, wait_option);
        if (ret == OS_SUCCESS)
        {
            return OS_SUCCESS;
        }
    }

    return OS_FAILED;
}

ssv_type_s32 OS_MsgQEnqueueTry( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, ssv_type_bool fromISR )
{
    u32 ret = 0;
    if (MsgQ && MsgItem)
    {
          ret = threadx_sendmsg(MsgQ, (const char *)MsgItem, 0);
          if (ret == OS_SUCCESS)
          {
               return OS_SUCCESS;
          }
    }

    return OS_FAILED;
}

ssv_type_s32 OS_MsgQWaitingSize( OsMsgQ MsgQ )
{
    return threadx_getmsg_size(MsgQ);
}

/**---------------------------------------------------------------------------*
**                         Compiler Flag                                      *
**---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
