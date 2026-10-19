

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////
///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH              != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

#if LV_COLOR_16_SWAP            !=0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////
///////////////////// FUNCTIONS ////////////////////
///////////////////// SCREENS ////////////////////
