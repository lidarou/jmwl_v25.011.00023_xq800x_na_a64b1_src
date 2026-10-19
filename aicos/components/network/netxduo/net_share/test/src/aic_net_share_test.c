/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file to test net share,such as the packets transfer
 *    and port manager.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16d325
#define LOG_MODULE    AMOD_NET

#include <string.h>
//#include <strings.h>
#include <nx_api.h>
#include <aic_net_api.h>
#include <aic_osa.h>
#include <aic_errno.h>
//#include "aic_at_parse_utils.h"
#include "aic_net_share_port_mgr.h"
#include "aic_usb_net_share_mgr.h"

/* Define internal functions. */
aos_thread_id thread_netshare_test;

/* Define external vars. */
extern struct aic_network_ip_instance alte_ip;
extern struct aic_network_ip_instance ppp_dial_lan_ip;
extern hashtable_t *ppp_dial_hashtable;

static void test_ppp_dial_up_direct(void)
{
    NET_UP_PKT_DIRECT direct_no_port;
    NET_UP_PKT_DIRECT direct_with_port;
    u16 i;

    aicx_info(0x019961, "s", "%s:test upstream\n", __func__);

    /* no ports */
    aic_ppp_dial_up_direct_no_port(&direct_no_port);
    if (direct_no_port.ip_ptr == (NX_IP *)alte_ip.ip_ptr) {
        aicx_info(0x0afc61, "s", "%s:IP instance is right\n", __func__);
    }
    aicx_info(0x1bd9fb, "sddd", "%s:intf_index=%d, port_converted=%d, to_nat=%d\n", __func__,
              direct_no_port.intf_index, direct_no_port.port_converted,
              direct_no_port.to_nat);

    /* with ports */
    for (i = 0; i <= 60000; (i += 3000)) {
        aic_ppp_dial_up_direct_with_port(i, &direct_with_port);
        if (direct_with_port.ip_ptr == (NX_IP *)alte_ip.ip_ptr) {
            aicx_info(0x0726c2, "s", "%s:IP instance direct right\n", __func__);
        }
        aicx_info(0x1723ff, "sdddd", "%s:intf_index=%d,original port=%d, port_converted=%d, "
                  "to_nat=%d\n", __func__, direct_with_port.intf_index, i,
                  direct_with_port.port_converted, direct_with_port.to_nat);
    }

    for (i = 0; i <= 20000; (i += 3000)) {
        aic_ppp_dial_up_direct_with_port(i, &direct_with_port);
        if (direct_with_port.ip_ptr == (NX_IP *)alte_ip.ip_ptr) {
            aicx_info(0x0726c2, "s", "%s:IP instance direct right\n", __func__);
        }
        aicx_info(0x1723ff, "sdddd", "%s:intf_index=%d,original port=%d, port_converted=%d, "
                  "to_nat=%d\n", __func__, direct_with_port.intf_index, i,
                  direct_with_port.port_converted, direct_with_port.to_nat);
    }
}

static void test_ppp_dial_down_direct(void)
{
    NET_DOWN_PKT_DIRECT direct_no_port;
    NET_DOWN_PKT_DIRECT direct_with_port;
    u16 i, converted_port;
    u32 ret;

    aicx_info(0x14a36f, "s", "%s:test downstream no port\n", __func__);
    aicx_info(0x0b3d43, "sx", "%s:ppp_dial_lan_ip=0x%x\n",
              __func__, (NX_IP *)ppp_dial_lan_ip.ip_ptr);

    aic_down_pkt_direct_no_port(1, &direct_no_port);
    aicx_info(0x124b3b, "sxdd", "%s:no port:IP instance=0x%x,intf_index=%d, to_nat=%d\n",
              __func__, direct_no_port.ip_ptr,
              direct_no_port.intf_index, direct_no_port.to_nat);

    aicx_info(0x1b6cb1, "s", "%s:test downstream with reserved port:\n", __func__);
    for (i = 0; i <= ANET_RESERVED_PORT_END; (i += 300)) {
        ret = aic_down_pkt_direct_with_port(i, 1, &direct_with_port);
        if (ret)
            aicx_error(0x114f52, "sd", "%s:reserved port:error, ret=%d\n", __func__, ret);

        aic_get_ppp_dial_dst_port(i, 1, &converted_port);
        aicx_info(0x084732, "sdd", "%s:reserved port:original port=%d,converted_port=%d\n",
                  __func__, i, converted_port);
        aicx_info(0x12ecea, "sxdd", "%s:reserved port:IP instance=0x%x,intf_index=%d, to_nat=%d\n",
                  __func__, direct_with_port.ip_ptr,
                  direct_with_port.intf_index, direct_with_port.to_nat);
    }

    aicx_info(0x129dc2, "s", "%s:test downstream with ppp port:\n", __func__);
    for (i = ANET_PPP_DIAL_PORT_START; i <= ANET_PPP_DIAL_PORT_START + 20; i++) {
        ret = aic_down_pkt_direct_with_port(i, 1, &direct_with_port);
        if (ret)
            aicx_error(0x0896be, "sd", "%s:ppp port:error, ret=%d\n", __func__, ret);

        aic_get_ppp_dial_dst_port(i, 1, &converted_port);
        aicx_info(0x0c7ad9, "sdd", "%s:ppp port:original port=%d,converted_port=%d\n",
                  __func__, i, converted_port);
        aicx_info(0x0c57ca, "sxdd", "%s:ppp port:IP instance=0x%x,intf_index=%d, to_nat=%d\n",
                  __func__, direct_with_port.ip_ptr,
                  direct_with_port.intf_index, direct_with_port.to_nat);
    }

    for (i = ANET_PPP_DIAL_PORT_START + 5000; i <= ANET_PPP_DIAL_PORT_START + 7000; (i += 150)) {
        ret = aic_down_pkt_direct_with_port(i, 1, &direct_with_port);
        if (ret)
            aicx_error(0x0896be, "sd", "%s:ppp port:error, ret=%d\n", __func__, ret);

        aic_get_ppp_dial_dst_port(i, 1, &converted_port);
        aicx_info(0x0c7ad9, "sdd", "%s:ppp port:original port=%d,converted_port=%d\n",
                  __func__, i, converted_port);
        aicx_info(0x0c57ca, "sxdd", "%s:ppp port:IP instance=0x%x,intf_index=%d, to_nat=%d\n",
                  __func__, direct_with_port.ip_ptr,
                  direct_with_port.intf_index, direct_with_port.to_nat);
    }

    aicx_info(0x0c8f27, "s", "%s:test downstream with internal port:\n", __func__);
    aicx_info(0x0c883d, "sx", "%s:alte_ip=0x%x\n", __func__, (NX_IP *)alte_ip.ip_ptr);
    for (i = ANET_INTERNAL_PORT_START; i <= 60000; (i += 1000)) {
        ret = aic_down_pkt_direct_with_port(i, 1, &direct_with_port);
        if (ret)
            aicx_error(0x18bb9c, "sd", "%s:internal port error:ret=%d\n", __func__, ret);
        aicx_info(0x1c1cbc, "sd", "%s:internal port:original port=%d\n", __func__, i);
        aicx_info(0x1de8ce, "sxdd", "%s:internal port:IP instance=0x%x,intf_index=%d, to_nat=%d\n",
                  __func__, direct_with_port.ip_ptr,
                  direct_with_port.intf_index, direct_with_port.to_nat);
    }

    aicx_info(0x14162c, "s", "%s:test downstream with usb port:\n", __func__);
    for (i = ANET_USBSHARE_PORT_START; i <= 12000; (i += 500)) {
        ret = aic_down_pkt_direct_with_port(i, 1, &direct_with_port);
        if (ret)
            aicx_info(0x1ae27c, "sdd", "%s:usb port %d,should return ret=%d\n",
                       __func__, i, ret);
    }
}

#if 0
static void test_ppp_dial_hash_delete()
{
    aicx_info(0x0fa0e5, "s", "%s:delete ppp dial hashtable\n", __func__);
    aic_delete_port_hashtable(ANET_NETSHARE_TYPE_PPP, ANET_SHARE_MODE_NDIS);
    if (!ppp_dial_hashtable)
        aicx_info(0x0dc22e, "s", "%s:delete OK\n", __func__);
}
#endif

void test_net_share_entry(void *thread_input)
{
    NET_UP_PKT_DIRECT direct_with_port;

    test_ppp_dial_up_direct();
    test_ppp_dial_down_direct();
    //test_ppp_dial_hash_delete();
    aos_sleep(ANET_PORT_TIMER_EXPIRE_TIME + 500);

    aic_ppp_dial_up_direct_with_port(5370, &direct_with_port);
    if (direct_with_port.ip_ptr == (NX_IP *)alte_ip.ip_ptr) {
        aicx_info(0x0726c2, "s", "%s:IP instance direct right\n", __func__);
    }
}

