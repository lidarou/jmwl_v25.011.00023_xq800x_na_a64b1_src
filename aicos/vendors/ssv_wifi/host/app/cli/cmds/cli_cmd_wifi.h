/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _CLI_CMD_WIFI_H_
#define _CLI_CMD_WIFI_H_

void cmd_iw(ssv_type_s32 argc, char *argv[]);
void cmd_ctl(ssv_type_s32 argc, char *argv[]);
void cmd_rf(ssv_type_s32 argc, char *argv[]);
void cmd_rc(ssv_type_s32 argc, char *argv[]);
void cmd_mib(ssv_type_s32 argc, char *argv[]);
void cmd_ampdu(ssv_type_s32 argc, char *argv[]);
void cmd_efuse(ssv_type_s32 argc, char *argv[]);
void cmd_ssv_adapt(ssv_type_s32 argc, char *argv[]);
void cmd_ssv_set_adapt(ssv_type_s32 argc, char *argv[]);


#if (ENABLE_SMART_CONFIG == 1)
void cmd_sconfig(ssv_type_s32 argc, char *argv[]);
#endif
#if(ENABLE_WPS ==1)
void cmd_wps(ssv_type_s32 argc, char *argv[]);
#endif
void ssv6xxx_wifi_cfg(void);


#define GET_SEC_INDEX(start,end,c)         \
	while(start <end ) {                   \
		if(c & BIT(start)){                \
			break;					       \
		}                                  \
		start++;                           \
	}                                      \


#endif /* _CLI_CMD_WIFI_H_ */

