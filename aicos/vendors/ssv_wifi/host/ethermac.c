/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x198597
#define LOG_MODULE    AMOD_VENDOR

#ifdef __SSV_UNIX_SIM__
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char	ssv_type_u8;

int get_eth0_as_mac(ssv_type_u8 *mac)
{
#if 0
    struct ifreq s;
    int ret = 1;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strcpy(s.ifr_name, "eth0");
    if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
        int i;
        for (i = 2; i < 6; ++i)
            mac[i]=(ssv_type_u8)s.ifr_addr.sa_data[i];

        ret = 0;
    }
    //lint -e746 -e718
    close(fd);
    return ret;
#else
    return 0;
#endif
}
#endif//__SSV_UNIX_SIM__

