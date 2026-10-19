
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0181c7
#define LOG_MODULE    AMOD_DRV
#include "xparameters.h"
#include "xuartps_hw.h"

#ifdef __cplusplus
extern "C" {
#endif
void outbyte(char c); 

#ifdef __cplusplus
}
#endif 

void outbyte(char c) {
	 XUartPs_SendByte(STDOUT_BASEADDRESS, c);
}
