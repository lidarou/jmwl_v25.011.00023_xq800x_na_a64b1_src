#include "app_fota.h"
#include "app_fota_flow.h"

struct fota_app_service {
    void (*notify)(void *srv, int evt_id, void *param);
    enum fota_app_status state;
    bool started;
};

static bool s_test_started = false;

struct fota_app_service *s_fota_app_service;

void fota_srv_init(void *param)
{
    s_fota_app_service->started = s_test_started;
}

void fota_srv_destory(bool gracefully)
{
    if (s_fota_app_service) {
        fota_app_free(s_fota_app_service);
        s_fota_app_service = NULL;
    }
}

void *fota_app_service_create(void (*notify)(void *srv, int evt_id,
                                             void *param))
{
    s_fota_app_service = fota_app_malloc(sizeof(struct fota_app_service));
    if (!s_fota_app_service) {
        return NULL;
    }

    memset(s_fota_app_service, 0, sizeof(struct fota_app_service));

    s_fota_app_service->notify = notify;

    return s_fota_app_service;
}

enum fota_app_status app_fota_get_status()
{
    int ret;

    if (!s_fota_app_service) {
        return 0;
    }

    ret = s_fota_app_service->state;
    ret++;
    if (ret > app_fota_need_reboot) {
        ret = 0;
    }

    return ret;
}

bool app_fota_is_started() { return s_fota_app_service->started; }

int app_fota_start_update()
{
    if (!s_fota_app_service) {
        return -1;
    }

    s_fota_app_service->state = app_fota_version_checking;
    s_fota_app_service->started = true;
    s_test_started = true;
    s_fota_app_service->notify(s_fota_app_service, fae_state_change, NULL);
    return 0;
}

int app_fota_stop_update()
{
    if (!s_fota_app_service) {
        return -1;
    }

    s_fota_app_service->state = app_fota_version_checked;
    s_fota_app_service->started = false;
    s_test_started = false;
    s_fota_app_service->notify(s_fota_app_service, fae_state_change, NULL);
    return 0;
}

int app_fota_reboot_to_update()
{
    s_fota_app_service->state = app_fota_init;
    return 0;
}

const char *app_fota_get_cur_version() { return "W23.07.1"; }

const char *app_fota_get_update_version()
{
    if (s_fota_app_service->state >= app_fota_version_checked) {
        return "W23.17.1";
    } else {
        return "Unkown";
    }
}
