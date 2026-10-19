// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.2.0
// PROJECT: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"
#include "../app_fota.h"
#include "../app_fota_flow.h"

void app_fota_on_back_clicked(lv_event_t * e)
{
    app_fota_back();
}

void app_fota_on_start_clicked(lv_event_t * e)
{
    app_fota_start_update();
}

void app_fota_on_stop_clicked(lv_event_t * e)
{
    app_fota_stop_update();
}
