
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0434b2
#define LOG_MODULE    AMOD_VENDOR
#include "abup_typedef.h"
#include "abup_debug.h"
#include "abup_define.h"
#include "abup_mcu.h"
#if defined(__EXT_BOOTLOADER__)
#include "abup_bl_main.h"
#endif



abupMcuPatch abup_patch_mcu[ABUP_FOTA_SDK_MAX_MCU_NUM]=
{
	//{ABUP_FOTA_ADDRESS2,abup_patch_mcu1},
};

abup_int16 abup_patch_mcu1(void)
{
#if defined(__EXT_BOOTLOADER__)
   abup_int16 status = ABUP_FUDIFFNET_ERROR_NONE;
#endif
	return 0;
}

abup_int16 abup_patch_mcu2(void)
{
	return 0;
}



