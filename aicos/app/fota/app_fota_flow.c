#include "alv_app_mgr.h"
#include "app_fota_flow.h"
#include "app_fota.h"
#include "app_fota/ui.h"
#include <ui_helpers.h>

struct app_fota {
  lv_obj_t *cur_page;
  bool quit_flag;
};

struct app_fota *s_fota_app;

void app_fota_back() {
  app_logd("app_fota: click to back...\n");
  alv_am_app_exit(NULL, FOTA_APP_NAME);
}

void fota_display_page(lv_obj_t *scr) {
  if (s_fota_app->cur_page != scr) {
    _ui_screen_change(scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0);
    s_fota_app->cur_page = scr;
  }
}

void update_widgets_display(enum fota_app_status stat) {
  switch (stat) {
  case app_fota_init:
    lv_label_set_text(ui_fota_main_start_label, "Update Now");
    break;
  case app_fota_version_checking:
    break;
  case app_fota_version_checked:
    lv_label_set_text(ui_fota_main_start_label, "Update Now");
    lv_label_set_text(ui_fota_main_ver_label, app_fota_get_cur_version());
    lv_label_set_text(ui_fota_main_new_ver_label,
                      app_fota_get_update_version());
    break;
  case app_fota_downloading:
    break;
  default: /* app_fota_need_reboot */
    lv_label_set_text(ui_fota_main_start_label, "Reboot & Update");
    break;
  }

  if (app_fota_is_started()) {
    fota_display_page(ui_fota_dl_scrn);
    lv_label_set_text(ui_fota_dl_stop_label, "Stop");
  } else {
    fota_display_page(ui_fota_main_scrn);
  }
}

void fota_destory(bool gracefully) {
  if (s_fota_app) {
    fota_app_malloc(s_fota_app);
    s_fota_app = NULL;
  }
  if (ui_fota_main_scrn) {
    lv_obj_del(ui_fota_main_scrn);
    ui_fota_main_scrn = NULL;
    app_logd("ui_fota_main_scrn memory free\n");
  }
  if (ui_fota_dl_scrn) {
    lv_obj_del(ui_fota_dl_scrn);
    ui_fota_dl_scrn = NULL;
    app_logd("ui_fota_dl_scrn memory free\n");
  }
}

void on_scr_unload_event() {
  app_logd("app_fota: on_scr_unload_event\n");
  if (s_fota_app->quit_flag) {
    fota_destory(true);
  }
}

void on_scr_event(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_SCREEN_UNLOADED) {
    on_scr_unload_event();
  }
}

void fota_notify(void *srv, int evt_id, void *param) {
  enum fota_app_status stat;

  stat = app_fota_get_status();
  update_widgets_display(stat);
}

lv_obj_t *fota_get_cur_page(void *param) { return s_fota_app->cur_page; }

void fota_init(void *param) {
  enum fota_app_status stat;

  app_logd("app_fota: init\n");
  s_fota_app->quit_flag = false;
  ui_fota_init();

  lv_obj_add_event_cb(ui_fota_main_scrn, on_scr_event, LV_EVENT_SCREEN_UNLOADED,
                      NULL);
  lv_obj_add_event_cb(ui_fota_dl_scrn, on_scr_event, LV_EVENT_SCREEN_UNLOADED,
                      NULL);
  fota_app_service_create(fota_notify);
  /* set cur_page */
  if (app_fota_is_started()) {
    s_fota_app->cur_page = ui_fota_dl_scrn;
  } else {
    s_fota_app->cur_page = ui_fota_main_scrn;
  }
  lv_disp_load_scr(s_fota_app->cur_page);

  stat = app_fota_get_status();
  update_widgets_display(stat);
}

void fota_quit(bool gracefully) {
  s_fota_app->quit_flag = true;
  fota_srv_destory(true);
}

/**
 * flow_fota_init - init fota app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_init(lv_obj_t *app) {
  LV_UNUSED(app);
  if (!s_fota_app) {

    s_fota_app = fota_app_malloc(sizeof(struct app_fota));
    if (!s_fota_app) {
      return;
    }
    memset(s_fota_app, 0, sizeof(struct app_fota));
  }

  fota_init(NULL);
}

/**
 * flow_fota_destory - destroy fota app flow status
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_destory(lv_obj_t *app) {
  LV_UNUSED(app);
  fota_quit(true);
}

/**
 * flow_fota_suspend - suspend the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_suspend(lv_obj_t *app) { LV_UNUSED(app); }

/**
 * flow_fota_resume - resume the app
 * @app: [IN] pointer to app obj
 *
 * Return: NA
 */
void flow_fota_resume(lv_obj_t *app) {
  LV_UNUSED(app);
  if (s_fota_app) {
    lv_disp_load_scr(s_fota_app->cur_page);
  }
}
