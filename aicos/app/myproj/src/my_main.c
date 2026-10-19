#include "my_common.h"

aos_thread_id my_init_task_id   = 0;
aos_thread_id my_gps_task_id    = 0;
aos_thread_id my_mqtt_task_id   = 0;

volatile uint8 g_pdpState = ANET_PDP_DEACTIVATED;


/*
 * NOTE:
 * 1. wakelock相关的接口必须在线程内使用, 否者系统会奔溃
 * 2. 必须在同一个线程中调用aic_get_wakelock和aic_put_wakelock
 */
void my_disable_sleep_internal()
{
    aic_get_wakelock();
}


void my_enable_sleep_internal()
{
    aic_put_wakelock();
}


// LTE快速休眠请求 释放RRC
void my_enable_lte_fast_dormancy_req(uint32 seconds)
{
    /*
     * <enable>: 是否打开增强型睡眠，1为打开，0为关闭。
     * <idle time>: 在最后一次数据收发之后，且数据成功发送，触发local release的时间，单位是秒，
     * 如果配置的是2，那么最后一次数据收发之后，且数据成功发送，过2s就触发local release，使modem进入idle态。
     * <retry time>: 在最后一次数据收发之后，如果数据没有发送成功，触发local release的时间，单位是秒，
     * 如果配置的是10，那么最后一次数据收发之后，且数据发送失败，过10s就触发local release，使modem进入idle态。
     */
	aic_network_setup_enhance_low_power(1, seconds, seconds);
}


void my_custom_nw_evt_cb(uint32 eventId, void* pData, uint32 len)
{
    switch (eventId)
    {
        /*
         * SDK的基站时间同步功能被干掉了
         * 现在通过SNTP获取UTC时间
         * NOTE:如果出海外固件需要修改aic_sntp_client中的服务器节点
        **/
        case NW_EVENT_NETTIME_SYNC_IND:
        {
            char time_str[32] = {0};
            //get_local_time(time_str, (uint8)sizeof(time_str));
            my_log_printf(1, "NW_EVENT_NETTIME_SYNC_IND: [%s]\n", time_str);
        }
            break;

        case NW_EVENT_SIM_PRESENT:
            my_log_printf(1, "Sim Event: SIM PRESENT");
            break;

        case NW_EVENT_SIM_ABSENT:
            my_log_printf(1, "Sim Event: SIM ABSENT");
            break;

        default:
            break;
    }
}

void my_device_cereg_update(uint8 cereg_state)
{
    static const char *states[] = {
        "not registered", "registered (home)", "searching", "denied",
        "unknown", "registered (roaming)"
    };
    my_log_printf(1, "[NET] registration=%u (%s)", cereg_state,
                  cereg_state < sizeof(states) / sizeof(states[0]) ?
                  states[cereg_state] : "unrecognized");
    my_log_printf(1, "%s: NET REGISTER STATUS: %d", __func__, cereg_state);

    // NW_REG_STATE_HOME_NETWORK=1: registered, home network
    // NW_REG_STATE_ROAMING=5: registered, roaming
    if (cereg_state == NW_REG_STATE_HOME_NETWORK ||
        cereg_state == NW_REG_STATE_ROAMING)
    {
        my_log_printf(1, "%s: 网络已注册", __func__);
    }
    // 0: not registered, MT is not currently searching an operator to register to
    else
    {
        my_log_printf(1, "%s: 网络未注册!", __func__);
    }
}


void my_pdp_state_change_cb(uint8 pdp_cid, uint8 pdp_state)
{
    WAN_NET *net_ptr = NULL;

    my_log_printf(1, "%s: PDP cid=%d, state=%d", __func__,
                    pdp_cid, pdp_state);

    if (pdp_cid != ANET_DEFAULT_PDP_CID)
    {
        my_log_printf(1, "[NET] ignoring non-default PDP cid=%u", pdp_cid);
        return;
    }

    if (pdp_state != ANET_PDP_ACTIVATED)
    {
        my_log_printf(1, "%s: PDP is not activated", __func__);
        g_pdpState = ANET_PDP_DEACTIVATED;
        my_log_printf(1, "[NET] default PDP lost; MQTT must wait for network");
        return;
    }

    // here pdp_cid == ANET_DEFAULT_PDP_CID
    net_ptr = aic_get_wan_net_per_pdp(pdp_cid);

    if (net_ptr != NULL && net_ptr->pdp_activated == 1)
    {
        g_pdpState = ANET_PDP_ACTIVATED;
        my_log_printf(1, "%s: PDP_ACTIVED CID=%d", __func__, pdp_cid);
        my_log_printf(1, "[NET] default PDP ready; MQTT connection can start");

        // LTE下在上下行无数据x秒后，发起RRC local release
        my_enable_lte_fast_dormancy_req(3);

        // 此处进入连接平台业务流程
        // TODO
    }
    else
    {
        g_pdpState = ANET_PDP_DEACTIVATED;
        my_log_printf(1, "[NET] default PDP unavailable, net_ptr=%p", net_ptr);
        my_log_printf(1, "%s: PDP_DEACTIVED CID=%d", __func__, pdp_cid);
    }
}


/*************************************************************
**函数名称:  my_init_task
**出口参数:  无
**函数功能:  初始化任务处理函数
*************************************************************/
void my_init_task(void *argv)
{
    my_log_printf(1, "[NET] registering SIM, registration and PDP callbacks");
    // SIM卡状态，网络时间同步
    aic_custom_nw_reg_callback(1, my_custom_nw_evt_cb);
    // 网络注册回调
    aic_register_cereg_state_cb(my_device_cereg_update);
    // PDP状态回调
    aic_register_pdp_state_change_cb(my_pdp_state_change_cb);

    while (1)
    {
        JM_Sleep(10 * 1000);
    }
}

/*********************************************************************
**函数名称:  my_gps_main()
**入口参数:  无
**出口参数:  无
**函数功能:  系统接口层相关数据初始化
*********************************************************************/
void my_gps_main(void)
{
    my_gpio_init();

    my_log_init();

    my_init_task_id = aos_create_thread("MY_INIT",
                                        my_init_task, NULL,
                                        MY_INIT_TASK_STACK_SIZE,
                                        0, AIC_NULL,
                                        MY_INIT_TASK_PRIORITY,
                                        AOS_AUTO_START);
    //my_log_printf(2, "#############my_init_task create ok,id=0x%x#############",my_init_task_id);

    my_gps_task_id = aos_create_thread("MY_GPSR",
                                        my_gpsrecv_task, NULL,
                                        MY_GPSRECV_TASK_STACK_SIZE,
                                        0, AIC_NULL,
                                        MY_GPSRECV_TASK_PRIORITY,
                                        AOS_AUTO_START);
    //my_log_printf(2, "#############my_gps_task create ok,id=0x%x#############",my_gps_task_id);

    my_mqtt_task_id = aos_create_thread("MY_MQTT",
                                        my_mqtt_task, NULL,
                                        MY_MQTT_TASK_STACK_SIZE,
                                        0, AIC_NULL,
                                        MY_MQTT_TASK_PRIORITY,
                                        AOS_AUTO_START);
    //my_log_printf(2, "#############my_mqtt_task create ok,id=0x%x#############",my_mqtt_task_id);
}
