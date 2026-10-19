/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifndef __LV_PORT_AIC310_H__
#define __LV_PORT_AIC310_H__

#include "aic_type.h"
#include "display_cfg.h"


#define DISP_HOR_RES LCD_PANEL_WIDTH
#define DISP_VER_RES LCD_PANEL_HEIGHT

void lv_set_refresh(boolean enable);

/**
* lv_disp_init - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to initialize display.
*/
void lv_disp_init(void);

#endif /*__LV_PORT_AIC310_H__*/
