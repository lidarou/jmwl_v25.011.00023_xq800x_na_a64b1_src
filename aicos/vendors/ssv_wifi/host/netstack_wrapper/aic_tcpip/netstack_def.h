/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#ifndef _NETSTACK_DEF_H_
#define _NETSTACK_DEF_H_

#define NETDEV_IF_DOWN      0
#define NETDEV_IF_UP        1
#define NETDEV_IF_LINK_UP   2
#define NETDEV_IF_BROADCAST   3

typedef enum _wifi_apmode_sec_s
{
    WIFI_APMODE_SEC_OPEN,
	WIFI_APMODE_SEC_WEP, 	
    WIFI_APMODE_SEC_WPA,//AES
    WIFI_APMODE_SEC_WPA2//AES
} wifi_apmode_sectype_t;

#endif //#ifndef _NETSTACK_DEF_H_
