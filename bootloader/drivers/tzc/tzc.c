/*****************************************************************************/
/*                                                                           */
/*      Copyright 2022 by GUIXIN Corporation. All rights reserved.              */
/*                                                                           */
/*****************************************************************************/

#include <aic_io.h>

/* Allow eip130, usb, sdio, dap access all iram range */
void tzc_init(void)
{
    /* set grp0,1,2 to */
    AIC_WRITE32(0x49144200, 0x0);
    return;
}

