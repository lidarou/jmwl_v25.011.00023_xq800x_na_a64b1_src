// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.2.1
// LVGL VERSION: 8.3.4
// PROJECT: photo

#include "ui.h"
#include "photo_flow.h"

void photo_delete_btn_event(lv_event_t * e)
{
	 flow_photo_list_remove_evt_cb(e);
}

void photo_preview_btn_event(lv_event_t * e)
{
	flow_photo_preview_enter_evt_cb(e);
}

void photo_return_btn_event(lv_event_t * e)
{
	/* end test */
	// flow_photo_back_key_evt_cb(e);
}

void photo_preview_delete_btn_event(lv_event_t * e)
{
	flow_photo_preview_remove_evt_cb(e);
}

void photo_play_btn_event(lv_event_t * e)
{
	flow_photo_preview_play_evt_cb(e);
}

void photo_preview_return_btn_event(lv_event_t * e)
{
	flow_photo_preview_exit_evt_cb(e);
}
