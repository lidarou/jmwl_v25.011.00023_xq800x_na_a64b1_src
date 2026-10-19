/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file alv_app_mgr.c
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09f256
#define LOG_MODULE    AMOD_LVGL

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "alv_app_mgr.h"
#include <alog_api.h>

/*********************
 *      DEFINES
 *********************/
#define MY_APP_CLASS &alv_app_class
#define MY_WIN_CLASS &alv_win_class

#ifndef min
#define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

#define NAME_MAX_LEN  (20)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum app_status {
    AM_APP_STATUS_NO = 0,   /* TODO: renamed this status */
    AM_APP_STATUS_ACTIVED,
    AM_APP_STATUS_FOCUSED,

    AM_APP_STATUS_MAX
} app_status_e;

typedef struct {
    char            *name;
    lv_obj_t        *app;
    app_status_e    status;
} alv_app_node_t;

typedef struct {
    lv_obj_t    obj;

    alv_app_node_t *app_node;   /* Node of this APP in "s_apps_ll" */

    char        *name;          /* Name of this APP */
    lv_obj_t    *first_win;     /* First window which wil be open when enter this APP */
    lv_ll_t     z_ll;           /* A linked list of Z axis */


    void        (*active)(lv_obj_t *app, void *p_param);  /* The "active" callback API, which will be called
                                                when status of this APP transfer from NO to ACTIVED */
    void        (*deactive)(lv_obj_t *app, void *p_param);   /*  */

    void        (*focus)(lv_obj_t *app, void *p_param);   /*  */
    void        (*defocus)(lv_obj_t *app, void *p_param);   /*  */
} alv_am_app_t;

/*----- Win -----*/
typedef struct {
    lv_obj_t    obj;

    char        *name;
    void        (*active)(lv_obj_t *win, void *p_param);
    void        (*deactive)(lv_obj_t *win, void *p_param);
    void        (*focus)(lv_obj_t *win, void *p_param);
    void        (*defocus)(lv_obj_t *win, void *p_param);
} alv_am_win_t;

typedef struct {
    lv_obj_t *win;
    lv_obj_t *focused;
} alv_am_z_node_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void app_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void app_deconstructor(const lv_obj_class_t *class_p, lv_obj_t *obj);

static void win_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void win_deconstructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t alv_app_class  = {
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .constructor_cb = app_constructor,
    .destructor_cb  = app_deconstructor,
    .width_def      = LV_PCT(100),
    .height_def     = LV_PCT(100),
    .instance_size  = sizeof(alv_am_app_t),
    .base_class     = &lv_obj_class
};

const lv_obj_class_t alv_win_class  = {
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .constructor_cb = win_constructor,
    .destructor_cb  = win_deconstructor,
    .width_def      = LV_PCT(100),
    .height_def     = LV_PCT(100),
    .instance_size  = sizeof(alv_am_win_t),
    .base_class     = &lv_obj_class
};

static lv_ll_t s_apps_ll;   /* a linked list of all apps */

/**********************
 *      MACROS
 **********************/


/**********************
 *   STATIC FUNCTIONS
 **********************/

static void app_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    //lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    alv_am_app_t *p_app = (alv_am_app_t *)obj;

    p_app->active   = NULL;
    p_app->deactive = NULL;
    p_app->focus    = NULL;
    p_app->defocus  = NULL;

    _lv_ll_init(&(p_app->z_ll), sizeof(alv_am_z_node_t));

    LV_TRACE_OBJ_CREATE("finished");
}

static void app_deconstructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    alv_am_app_t *p_app = (alv_am_app_t *)obj;
    lv_ll_t *p_z_ll = &(p_app->z_ll);

    /* clear the linked list of Z */
    _lv_ll_clear(p_z_ll);

    LV_TRACE_OBJ_CREATE("finished");
}

static void win_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    alv_am_win_t *p_win = (alv_am_win_t *)obj;

    p_win->active   = NULL;
    p_win->deactive = NULL;
    p_win->focus    = NULL;
    p_win->defocus  = NULL;

    LV_TRACE_OBJ_CREATE("finished");
}

static void win_deconstructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    LV_TRACE_OBJ_CREATE("finished");
}

/**
 * Get the pointer of app by name which has been created.
 * @param:
 *          app     pointer to an object, it will be the parent of the new app
 *          name    name of app whose obj you want
 * @return:     the pointer of window
 */
#ifdef CONFIG_FP
lv_obj_t *get_app_by_name(char *name)
#else
static lv_obj_t *get_app_by_name(char *name)
#endif
{
    if (!name) {
        //TODO: alog_info("Input error in [%s]\n", __func__);
        return NULL;
    }

    alv_app_node_t *p_current_app = (alv_app_node_t *)_lv_ll_get_head(&s_apps_ll);
    uint32_t num = _lv_ll_get_len(&s_apps_ll);
    for(uint32_t i = 0; i < num; i++) {

        if (NULL != p_current_app) {
            if (strcmp(name, p_current_app->name) == 0) {
                return p_current_app->app;
            }
            p_current_app = (alv_app_node_t *)_lv_ll_get_next(&s_apps_ll, p_current_app);
        }
    }
    return NULL;
}

static void print_len_val(uint32_t val)
{
#ifndef WIN32
    aicx_debug(0x0b06ee, "sd", " [%s],val = %d\n", __func__,val);
#endif
}

/**
 * Get the pointer of current app.
 * @param:      none
 * @return:     the pointer of current app
 */
static lv_obj_t *get_current_app(void)
{
    alv_app_node_t *p_current_app = (alv_app_node_t *)_lv_ll_get_tail(&s_apps_ll);

    if (NULL != p_current_app) {
        if (AM_APP_STATUS_FOCUSED == p_current_app->status) {
            return p_current_app->app;
        } else {
            // TODO :print
        }
    }
    return NULL;
}

/**
 Get the pointer of window by name in one APP
 * @param:
 *          app     pointer to an object, it will be the parent of the new app
 *          name    name of window whose obj you want
 * @return:     the pointer of window
 */
#ifdef CONFIG_FP
lv_obj_t *get_win_by_name(lv_obj_t *app, char *name)
#else
static lv_obj_t *get_win_by_name(lv_obj_t *app, char *name)
#endif
{
    if (!app) {
        //TODO: alog_info("Input error in [%s]\n", __func__);
        return NULL;
    }

    for(uint32_t i = 0; i < lv_obj_get_child_cnt(app); i++) {
        lv_obj_t *child = lv_obj_get_child(app, i);

        if (NULL != child) {
            alv_am_win_t *p_win = (alv_am_win_t *)child;

            if (strcmp(name, p_win->name) == 0) {
                return child;
            }
        }
    }
    return NULL;
}

/**
 * Get the win name of window by name in one APP
 * @param:
 *          win_name:output param
 * @return: -1,fail,0,success.
 */
#ifdef CONFIG_FP
int32_t alv_am_win_get_cur_winname(char *win_name)
#else
static int32_t alv_am_win_get_cur_winname(char *win_name)
#endif
{
    alv_am_app_t *p_cur_app = NULL;
    alv_am_win_t *p_cur_win = NULL;
    if(NULL == win_name) {
        aicx_error(0x1a690b, "s", "[%s],win_name is null.\n", __func__);
        return -1;
    }
    lv_obj_t *p_app = get_current_app();
    if(NULL == p_app) {
        aicx_error(0x1c9afa, "s", "[%s],p_app is null.\n", __func__);
        return -1;
    }
    p_cur_app = (alv_am_app_t *)p_app;
    alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_cur_app->z_ll));
    if(NULL != p_node) {
        p_cur_win = (alv_am_win_t *)p_node->win;
    }
    if(NULL == p_cur_win) {
        aicx_error(0x021ba3, "s", "[%s],p_cur_win is null.\n", __func__);
        return -1;
    } else {
        lv_memcpy(win_name, p_cur_win->name, strlen(p_cur_win->name));
    }

    alog_info("[%s],win_name = %s.\n", __func__, win_name);
    return 0;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/*----- APIs of application-----*/
lv_obj_t *alv_am_app_create(lv_obj_t *parent, char *name)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_APP_CLASS, parent);
    lv_obj_class_init_obj(obj);

    alv_am_app_t *p_app = (alv_am_app_t *)obj;

    p_app->name = lv_mem_alloc(sizeof(char)*NAME_MAX_LEN);
    lv_memset_00(p_app->name,NAME_MAX_LEN);
    lv_memcpy(p_app->name,name,min(NAME_MAX_LEN,strlen(name)));

    /* Create list to saving apps */
    if ( _lv_ll_is_empty(&s_apps_ll) ) {
        _lv_ll_init(&s_apps_ll, sizeof(alv_app_node_t));
    }

    /* TODO: Judge whether the name has duplicate */

    /* Add node of this app in "s_apps_ll" */
    alv_app_node_t *p_app_node = _lv_ll_ins_head(&s_apps_ll); /* Add app node at head */
    if(NULL == p_app_node) {
        LV_LOG_ERROR("p_app_node is null");
        return NULL;
    }
    p_app_node->name           = name;              /* Save name of APP in node */
    p_app_node->app            = obj;               /* Save pointer of APP in node */
    p_app_node->status         = AM_APP_STATUS_NO;  /* Set NO status for this APP */

    /* Save app node in app class */
    p_app->app_node            = p_app_node;        /* Save node in obj of this APP */

    return obj;
}

void alv_am_app_set_active(lv_obj_t *app, void (*active)(lv_obj_t *app, void *p_param))
{
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    if (NULL != p_app) {
        p_app->active = active;
    } else {
        aicx_info(0x0f7708, "s", "p_app is null in [%s]\n", __func__);
    }

}
void alv_am_app_set_deactive(lv_obj_t *app, void (*deactive)(lv_obj_t *app, void *p_param))
{
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    if (NULL != p_app) {
        p_app->deactive = deactive;
    } else {
        aicx_info(0x0f7708, "s", "p_app is null in [%s]\n", __func__);
    }

}
void alv_am_app_set_focus(lv_obj_t *app, void (*focus)(lv_obj_t *app, void *p_param))
{
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    if (NULL != p_app) {
        p_app->focus = focus;
    } else {
        aicx_info(0x0f7708, "s", "p_app is null in [%s]\n", __func__);
    }
}

void alv_am_app_set_defocus(lv_obj_t *app, void (*defocus)(lv_obj_t *app, void *p_param))
{
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    if (NULL != p_app) {
        p_app->defocus = defocus;
    } else {
        aicx_info(0x0f7708, "s", "p_app is null in [%s]\n", __func__);
    }
}


int32_t alv_am_app_set_first_win(lv_obj_t *app, lv_obj_t *win)
{
    if ((!app) || (!win)) {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    alv_am_app_t *p_app = (alv_am_app_t *)app;

    p_app->first_win = win;

    return 0;
}

int32_t alv_am_app_enter(lv_obj_t *app, char *name, void *p_param)
{
    alv_am_app_t *p_app = NULL;
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (app) {
        p_app = (alv_am_app_t *)app;
    }
    else if (name) {
        /* Get app by name */
        p_app = (alv_am_app_t *)get_app_by_name(name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if (NULL == p_app) {
#ifndef WIN32
        aicx_info(0x0fb195, "s", "p_app is NULL [%s]\n", __func__);
#endif
        return -1;
    }

    /* Have NOT Status Transfer */
    if ( AM_APP_STATUS_FOCUSED == p_app->app_node->status ) {
#ifndef WIN32
        aicx_info(0x13ff6a, "s", "Already in this current APP [%s]\n", __func__);
#endif
        return -1;
    }
    /* Modify focused(current) APP, and save its window info */
    uint32_t num = _lv_ll_get_len(&s_apps_ll);
    print_len_val(num);

    /* Change status of APP which at tail */
    alv_app_node_t *p_last_node_app = _lv_ll_get_tail(&s_apps_ll);

    /* Save the focused obj of last app*/
    if ( AM_APP_STATUS_FOCUSED == p_last_node_app->status ) {
        p_last_node_app->status  = AM_APP_STATUS_ACTIVED;

        alv_am_app_t *p_last_app         = (alv_am_app_t *)p_last_node_app->app;

        /* Defocus last app */
        if(NULL != p_last_app->defocus) {
            aicx_info(0x12ab8e, "s", "[%s],p_last_app->defocus", __func__);
            p_last_app->defocus((lv_obj_t *)p_last_app, p_param);
        }

        /*if first win is null,use app mgr,first win is not null,use win mgr */
        if (p_last_app->first_win) {
            alv_am_z_node_t *p_last_node_win = (alv_am_z_node_t *)_lv_ll_get_tail(&(p_last_app->z_ll));
            alv_am_win_t *p_last_win = (alv_am_win_t *)p_last_node_win->win;
            /* Defocus window of last APP*/
            if(p_last_win->defocus) {
                p_last_win->defocus((lv_obj_t *)p_last_win, p_param);
            }
            /* Hide current window */
            lv_obj_add_flag((lv_obj_t *)(p_last_node_win->win), LV_OBJ_FLAG_HIDDEN);
        }
    }
    else {
        LV_LOG_WARN("The focused APP is NOT at the taill, or there is NO any APP yet. \n");
    }

    /* Status Transfer: NO=>ACTIVED */
    /* When status of the APP is NO Call event callback "active" */
    if ( AM_APP_STATUS_NO == p_app->app_node->status ) {
        if (p_app->active) {
            p_app->active((lv_obj_t *)p_app,p_param);
        }
    }
    p_app->app_node->status = AM_APP_STATUS_ACTIVED;

    /* Status Transfer: ACTIVED=>FOCUSED */
    /* When status of the APP is ACTIVED Call event callback "focus" */
    if ( AM_APP_STATUS_ACTIVED == p_app->app_node->status ) {
        /* add focus callback */
        if (p_app->focus) {
            aicx_info(0x17fa93, "s", "[%s],p_app->focus", __func__);
            p_app->focus((lv_obj_t *)p_app, p_param);
        }
    }

    /* Move the node of this APP to tail */
    _lv_ll_remove(&s_apps_ll, (void *)p_app->app_node);
    lv_mem_free((void *)p_app->app_node);
    alv_app_node_t *p_node_app = _lv_ll_ins_tail(&s_apps_ll);
    p_node_app->name           = p_app->name;
    p_node_app->app            = (lv_obj_t *)p_app;
    p_node_app->status         = AM_APP_STATUS_FOCUSED;

    /* Save app node in app class */
    p_app->app_node            = p_node_app;

    /* Enter first window of this APP */
    if (p_app->first_win) {
        /* The app has fisrt win, means this app have many children (windows) managered by win mgr */
        p_win = (alv_am_win_t *)p_app->first_win;

        /* Make this screen active */
#ifndef CONFIG_FP
        lv_disp_load_scr((lv_obj_t *)p_win);
#endif
        //lv_obj_move_foreground((lv_obj_t *)p_win);//TODO: 为什么lv_disp_load_scr((lv_obj_t *)p_win);不行，rootcause是？

        /* Active first window of this app */
        if (p_win->active) {
            p_win->active((lv_obj_t *)p_win, p_param);
        }
        else {
#ifndef WIN32
            aicx_info(0x16b253, "s", "There is NOT active function of this win in [%s]\n", __func__);
#endif
        }

        /* Focus first window of this app */
        if (p_win->focus) {
            p_win->focus((lv_obj_t *)p_win, p_param);
        }

        /* Add node into Z axis */
        alv_am_z_node_t *p_node = _lv_ll_ins_tail(&(p_app->z_ll));
        p_node->win             = (lv_obj_t *)p_win;    /* Add win node at tail */
        /* print length of linked list */
        uint32_t len            = _lv_ll_get_len(&(p_app->z_ll));
        print_len_val(len);
    }
    else {
#ifndef WIN32
        aicx_info(0x06183d, "s", "There is NOT first win of this APP in [%s]\n", __func__);
#endif
    }

    return 0;
}

int32_t alv_am_app_exit(lv_obj_t *app, char *name, void *p_param)
{
    alv_am_app_t *p_app = NULL;
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (app) {
        p_app = (alv_am_app_t *)app;
    }
    else if (name) {
        /* Get app by name */
        p_app = (alv_am_app_t *)get_app_by_name(name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    /* Defocus this app */
    if(NULL != p_app->defocus) {
        aicx_info(0x102624, "s", "[%s],p_app->defocus", __func__);
        p_app->defocus((lv_obj_t *)p_app, p_param);
    }

    /* Get windows in Z and exit all */
    /*if first win is null,use app mgr,first win is not null,use win mgr */
    if(NULL != p_app->first_win) {
        alv_am_z_node_t *p_tail_node = _lv_ll_get_tail(&(p_app->z_ll));
        p_win = (alv_am_win_t *)p_tail_node->win;

        // TODO: one API for exit win only
        /* TODO: Defocus this window */

        /* Del all child obj of this win */
        uint32_t cnt = lv_obj_get_child_cnt((lv_obj_t *)p_win);
        for(uint32_t i = (cnt-1); i < cnt; i--) {  /* Must be "i--", because children is in an array whose id is start with "0" */
            lv_obj_t *child = lv_obj_get_child((lv_obj_t *)p_win, i);
            lv_obj_del(child);
        }
    }else {
#ifndef WIN32
        aicx_info(0x160e31, "s", "[%s],app use app mgr \n", __func__);
#endif
    }

    /* 将要退出的app移动到队列头部 */
    alv_app_node_t *p_head_node = _lv_ll_get_head(&s_apps_ll);
    _lv_ll_move_before(&s_apps_ll, p_app->app_node, p_head_node);
    p_app->app_node->status = AM_APP_STATUS_NO;

    /* Show prev APP */
    alv_app_node_t *p_node_app = _lv_ll_get_tail(&s_apps_ll);
    p_node_app->status         = AM_APP_STATUS_FOCUSED;

    /* Show prev window */
    alv_am_app_t *p_last_app      = (alv_am_app_t *)p_node_app->app;
    alv_am_z_node_t *p_prev_node  = _lv_ll_get_tail(&(p_last_app->z_ll));

    /* if first win is null,use app mgr,else use win mgr */
    if(NULL != p_last_app->first_win) {
        alv_am_win_t *p_prev_win = (alv_am_win_t *)p_prev_node->win;
        lv_obj_clear_flag((lv_obj_t *)p_prev_win, LV_OBJ_FLAG_HIDDEN);

        /* Focus prev window*/
        if (p_prev_win->focus) {
            p_prev_win->focus((lv_obj_t *)p_prev_win, p_param);
            aicx_info(0x004c25, "ss", "[%s] focus win %s", __func__, p_prev_win->name);
        }
        /*TODO: keep previous settings, del this later*/
        else {
            if (p_prev_win->deactive) {
                p_prev_win->deactive((lv_obj_t *)p_prev_win, p_param);
            }
            if (p_prev_win->active) {
                p_prev_win->active((lv_obj_t *)p_prev_win, p_param);
            }
            aicx_info(0x0f85b1, "ss", "[%s] active win %s", __func__, p_prev_win->name);
        }
        /*  Make this screen active */
        //lv_disp_load_scr((lv_obj_t *)p_prev_win);
        //lv_obj_move_foreground((lv_obj_t *)p_prev_win); //TODO: 为什么lv_disp_load_scr((lv_obj_t *)p_prev_win);不行，rootcause是？
    } else {
#ifndef WIN32
        aicx_info(0x160e31, "s", "[%s],app use app mgr \n", __func__);
#endif
    }

    /* last app focus*/
    if(p_last_app->focus){
        aicx_info(0x06f6fc, "s", "[%s],p_last_app->focus", __func__);
        p_last_app->focus((lv_obj_t *)p_last_app, p_param);
    }

    /* besure disp->act_scr is not null,when this app has only one screen, must after fresh last app screen,then delelted */
    /* Deactive current app */
    if( p_app->deactive ) {
        p_app->deactive((lv_obj_t *)p_app, p_param);
    }

    /* Deactive current app last window */
    if(NULL != p_app->first_win) {
        if(p_win->deactive) {
            p_win->deactive((lv_obj_t *)p_win, p_param);
        }
    }

    /* clear app z order */
    _lv_ll_clear(&(p_app->z_ll));

    return 0;
}

/* TODO: app的名称是在create时申请，需要在alv_am_app_destroy里释放 */

bool alv_am_app_is_exist(char *app_name)
{
    bool result = false;
    alv_am_app_t *p_app = NULL;

    if(NULL == app_name) {
#ifndef WIN32
        aicx_info(0x131bb7, "s", "app_name is null [%s]\n", __func__);
#endif
        return result;
    }

    lv_obj_t *p_current_app = get_current_app();
    if(NULL != p_current_app) {
        p_app = (alv_am_app_t *)p_current_app;

        if(0 == strcmp(app_name,p_app->name)){
            result = true;
            }
        }

    return result;
}

bool alv_am_app_is_active(char *app_name)
{
    bool result = false;
    alv_am_app_t *p_app = NULL;
    if(NULL == app_name) {
        aicx_error(0x15e3ea, "s", "[%s] app name is null!", __func__);
        return result;
    }
    p_app = (alv_am_app_t *)get_app_by_name(app_name);
    if(NULL == p_app) {
        aicx_error(0x0ab403, "s", "[%s] app is null!", __func__);
        return result;
    }
    if(NULL == p_app->app_node) {
        aicx_error(0x11a0ff, "s", "[%s] app node is null!", __func__);
        return result;
    }
    if(AM_APP_STATUS_ACTIVED == p_app->app_node->status) {
        result = true;
    }
    return result;
}
/*-------------------------------------------------------------------*/
/*----- APIs of windos-----*/
lv_obj_t * alv_am_win_create(lv_obj_t *parent, char *name)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_WIN_CLASS, parent);
    lv_obj_class_init_obj(obj);

    alv_am_win_t *win = (alv_am_win_t *)obj;

    win->name = lv_mem_alloc(sizeof(char)*NAME_MAX_LEN);
    lv_memset_00(win->name,NAME_MAX_LEN);
    lv_memcpy(win->name, name,min(NAME_MAX_LEN,strlen(name)));

    return obj;
}

void alv_am_win_set_active(lv_obj_t *win, void(*active)(lv_obj_t *win, void *p_param))
{
    alv_am_win_t *p_win = (alv_am_win_t *)win;

    if (NULL != p_win) {
        p_win->active = active;
    } else {
        aicx_info(0x0961a8, "s", "p_win is null in [%s]\n", __func__);
    }
}

void alv_am_win_set_deactive(lv_obj_t *win, void (*deactive)(lv_obj_t *win, void *p_param))
{
    alv_am_win_t *p_win = (alv_am_win_t *)win;

    if (NULL != p_win) {
        p_win->deactive = deactive;
    } else {
        aicx_info(0x0961a8, "s", "p_win is null in [%s]\n", __func__);
    }
}

void alv_am_win_set_focus(lv_obj_t *win, void (*focus)(lv_obj_t *win, void *p_param))
{
    alv_am_win_t *p_win = (alv_am_win_t *)win;

    if (NULL != p_win) {
        p_win->focus = focus;
    } else {
        aicx_info(0x0961a8, "s", "p_win is null in [%s]\n", __func__);
    }
}

void alv_am_win_set_defocus(lv_obj_t *win, void (*defocus)(lv_obj_t *win, void *p_param))
{
    alv_am_win_t *p_win = (alv_am_win_t *)win;

    if (NULL != p_win) {
        p_win->defocus = defocus;
    } else {
        aicx_info(0x0961a8, "s", "p_win is null in [%s]\n", __func__);
    }
}

int32_t alv_am_win_enter(lv_obj_t *win, char *name, void *p_param)
{
    alv_am_win_t *p_win = NULL;

    /* Get obj of win */
    if (win) {
        p_win = (alv_am_win_t *)win;
    }
    else if (name) {
        /* get win by name */
        lv_obj_t *p_cur_app = get_current_app();
        p_win = (alv_am_win_t *)get_win_by_name(p_cur_app,name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if(NULL == p_win) {
        aicx_error(0x17dbe2, "s", "[%s], p_win is null \n", __func__);
        return -1;
    }

    /* Get app by win */
    lv_obj_t *app = lv_obj_get_parent((lv_obj_t *)p_win);
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    /* Save focused */
    alv_am_z_node_t *p_cur_node = _lv_ll_get_tail(&(p_app->z_ll));/* Get current win node at tail */
    if(NULL != p_cur_node) {
        alv_am_win_t * p_cur_win    = (alv_am_win_t *)p_cur_node->win;
        /* Defocus current window*/
        if(p_cur_win->defocus) {
            p_cur_win->defocus((lv_obj_t *)p_cur_win, p_param);
        }
        /* Hide current window */
        lv_obj_add_flag(p_cur_node->win, LV_OBJ_FLAG_HIDDEN);
    } else {
        // TODO: assert();
    }

    // TODO: make sure is OK for any window
#ifndef CONFIG_FP
     lv_obj_move_foreground((lv_obj_t *)p_win);//TODO:为什么lv_disp_load_scr((lv_obj_t *)p_win); 不可以，rootcause是？
#endif
    //lv_disp_load_scr((lv_obj_t *)p_win); /*  Make this screen active */

    /* Active this window */
    if(p_win->active) {
        p_win->active((lv_obj_t *)p_win, p_param);
    }

    /* Focus this window */
    if(p_win->focus) {
        p_win->focus((lv_obj_t *)p_win, p_param);
    }

    /* Add node into Z axis */
    alv_am_z_node_t *p_node = _lv_ll_ins_tail(&(p_app->z_ll));
    p_node->win             = (lv_obj_t *)p_win;    /* Add win node at tail */
    /* print this */
    uint32_t len            = _lv_ll_get_len(&(p_app->z_ll));
    print_len_val(len);

    return 0;
}

int32_t alv_am_win_exit(lv_obj_t *win, char *name, void *p_param)
{
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (win) {
        p_win = (alv_am_win_t *)win;
    }
    else if (name) {
        /* get win by name */
        lv_obj_t *p_cur_app = get_current_app();
        p_win = (alv_am_win_t *)get_win_by_name(p_cur_app,name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if(NULL == p_win) {
        aicx_error(0x1e691b, "ss", "%s, p_win is null [%s]\n", __func__);
        return -1;
    }

    /* Judge whether p_win is in Z axis */

    /* Get app by win */
    lv_obj_t *app = lv_obj_get_parent((lv_obj_t *)p_win);
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    uint32_t len = _lv_ll_get_len(&(p_app->z_ll));
    print_len_val(len);
    if (1 == len) {
        aicx_info(0x1001fa, "s", "There is only ONE window in APP in [%s]\n", __func__);
        return -1;
    }

    alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_app->z_ll));
    //p_win                   = (alv_am_win_t *)p_node->win;

    /* Defocus this win*/
    if (p_win->defocus) {
        p_win->defocus((lv_obj_t *)p_win, p_param);
    }

    /* Remove node from Z axis */
    _lv_ll_remove(&(p_app->z_ll), p_node);
#ifdef CONFIG_FP
    lv_mem_free(p_node);
    p_node = NULL;
#endif
    /* free p_node */

#ifdef CONFIG_FP
#ifndef CONFIG_APP_ENTRY_IDLE_FP
    /* Del all child obj of this win */
    uint32_t cnt = lv_obj_get_child_cnt((lv_obj_t *)p_win);
    for(uint32_t i = (cnt-1); i < cnt; i--) {  /* Must be "i--", because children is in an array whose id is start with "0" */
        lv_obj_t *child = lv_obj_get_child((lv_obj_t *)p_win, i);
        lv_obj_del(child);
    }
#endif
#else
    /* Del all child obj of this win */
    uint32_t cnt = lv_obj_get_child_cnt((lv_obj_t *)p_win);
    for(uint32_t i = (cnt-1); i < cnt; i--) {  /* Must be "i--", because children is in an array whose id is start with "0" */
        lv_obj_t *child = lv_obj_get_child((lv_obj_t *)p_win, i);
        lv_obj_del(child);
    }
#endif

    /* Show prev window */
    alv_am_z_node_t *p_prev_node  = _lv_ll_get_tail(&(p_app->z_ll));
    alv_am_win_t *p_prev_win = (alv_am_win_t *)p_prev_node->win;
    lv_obj_clear_flag((lv_obj_t *)p_prev_win, LV_OBJ_FLAG_HIDDEN);

    /*  Make this screen active */
    //lv_disp_load_scr((lv_obj_t *)p_prev_win);
#ifdef CONFIG_FP
    lv_obj_move_foreground((lv_obj_t *)p_prev_win);//TODO:为什么lv_disp_load_scr((lv_obj_t *)p_prev_win);不可以，rootcause是？
#endif

    /* Focus previous win*/
    if(p_prev_win->focus) {
        p_prev_win->focus((lv_obj_t *)p_prev_win, p_param);
        aicx_info(0x004c25, "ss", "[%s] focus win %s", __func__, p_prev_win->name);
    }
    /*TODO: keep previous settings now, del this later*/
    else if(p_prev_win->active) { /* TODO:exist memory leak,need add show in win lifecycle */
        p_prev_win->active((lv_obj_t *)p_prev_win, p_param);
        aicx_info(0x0f85b1, "ss", "[%s] active win %s", __func__, p_prev_win->name);
    }

    /* Deactive this window*/
    /* Besure disp=>act_scr is not null,first fresh new scr, then delete old scr */
    if (p_win->deactive) {
        p_win->deactive((lv_obj_t *)p_win, p_param);
    }
    return 0;
}

/* TODO: win的名称是在create时申请，需要在alv_am_win_destroy里释放 */

#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
extern int32_t alv_am_win_exit_action(lv_obj_t *win, char *name, void *p_param);
#endif
#endif

int32_t alv_am_win_exit_current(void *p_param)
{
    int32_t ret = 0;
    char win_name[NAME_MAX_LEN] = {0};

    /* Get current win and exit */
    alv_am_win_get_cur_winname(win_name);

#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
    ret = alv_am_win_exit_action(NULL, win_name, p_param);
#endif
#else
    ret = alv_am_win_exit(NULL, win_name, p_param);
#endif
    aicx_info(0x10e6bf, "sd", "[%s] ret is %d", __func__, ret);
    return ret;
}

#ifdef CONFIG_FP
#ifdef CONFIG_APP_ENTRY_IDLE_FP
int32_t alv_am_app_enter_action(lv_obj_t *app, char *name, void *p_param)
{
    alv_am_app_t *p_app = NULL;
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (app) {
        p_app = (alv_am_app_t *)app;
    }
    else if (name) {
        /* Get app by name */
        p_app = (alv_am_app_t *)get_app_by_name(name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if (NULL == p_app) {
#ifndef WIN32
        aicx_info(0x0fb195, "s", "p_app is NULL [%s]\n", __func__);
#endif
        return -1;
    }

    /* Have NOT Status Transfer */
    if ( AM_APP_STATUS_FOCUSED == p_app->app_node->status ) {
#ifndef WIN32
        aicx_info(0x13ff6a, "s", "Already in this current APP [%s]\n", __func__);
#endif
        return -1;
    }

    /* Status Transfer: NO=>ACTIVED */
    /* When status of the APP is NO Call event callback "active" */
    if ( AM_APP_STATUS_NO == p_app->app_node->status ) {
        if (p_app->active) {
            p_app->active((lv_obj_t *)p_app,p_param);
        }
        p_app->app_node->status = AM_APP_STATUS_ACTIVED;
    }

    /* Status Transfer: ACTIVED=>FOCUSED */
    /* When status of the APP is ACTIVED Call event callback "focus" */
    if ( AM_APP_STATUS_ACTIVED == p_app->app_node->status ) {
        /* add focus callback */
        if (p_app->focus) {
            p_app->focus((lv_obj_t *)p_app, p_param);
        }
        p_app->app_node->status	= AM_APP_STATUS_FOCUSED;
    }

    /* Modify focused(current) APP, and save its window info */
    uint32_t num = _lv_ll_get_len(&s_apps_ll);
    print_len_val(num);

    /* Change status of APP which at tail */
    alv_app_node_t *p_last_node_app = _lv_ll_get_tail(&s_apps_ll);

    /* Save the focused obj of last app*/
    if(((alv_am_app_t *)p_last_node_app->app!=p_app)&&( AM_APP_STATUS_FOCUSED == p_last_node_app->status )) {
        p_last_node_app->status  = AM_APP_STATUS_ACTIVED;

        alv_am_app_t *p_last_app         = (alv_am_app_t *)p_last_node_app->app;
        /* TODO: Defocus last app */

        /*if first win is null,use app mgr,first win is not null,use win mgr */
        if (p_last_app->first_win) {
            alv_am_z_node_t *p_last_node_win = (alv_am_z_node_t *)_lv_ll_get_tail(&(p_last_app->z_ll));
            alv_am_win_t *p_last_win = (alv_am_win_t *)p_last_node_win->win;
            /* Defocus window of last APP*/
            if(p_last_win->defocus) {
                p_last_win->defocus((lv_obj_t *)p_last_win, p_param);
            }
            /* Hide current window */
        }
        if(p_last_app->defocus){
            p_last_app->defocus((lv_obj_t *)p_app, p_param);
        }

    }
    else {
        LV_LOG_WARN("The focused APP is NOT at the taill, or there is NO any APP yet. \n");
    }

    /* Move the node of this APP to tail */
    _lv_ll_remove(&s_apps_ll, (void *)p_app->app_node);
    lv_mem_free((void *)p_app->app_node);
    alv_app_node_t *p_node_app = _lv_ll_ins_tail(&s_apps_ll);
    p_node_app->name           = p_app->name;
    p_node_app->app            = (lv_obj_t *)p_app;
    p_node_app->status         = AM_APP_STATUS_FOCUSED;

    /* Save app node in app class */
    p_app->app_node            = p_node_app;

    /* Enter first window of this APP */
    if (p_app->first_win) {
        /* The app has fisrt win, means this app have many children (windows) managered by win mgr */
        if (0 == _lv_ll_get_len(&(p_app->z_ll))) {
            p_win = (alv_am_win_t *)p_app->first_win;
        }
        else
        {
            alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_app->z_ll));
            p_win = (alv_am_win_t *)p_node->win;
        }
        /* Make this screen active */
        //lv_disp_load_scr((lv_obj_t *)p_win);
        //lv_obj_move_foreground((lv_obj_t *)p_win);//TODO: 为什么lv_disp_load_scr((lv_obj_t *)p_win);不行，rootcause是？

        /* Active first window of this app */
        if (p_win->active) {
            p_win->active((lv_obj_t *)p_win, p_param);
        }
        else {
#ifndef WIN32
            aicx_info(0x16b253, "s", "There is NOT active function of this win in [%s]\n", __func__);
#endif
        }

        /* Focus first window of this app */
        if (p_win->focus) {
            p_win->focus((lv_obj_t *)p_win, p_param);
        }

        /* Add node into Z axis */
        alv_am_z_node_t *p_node = _lv_ll_ins_tail(&(p_app->z_ll));
        p_node->win             = (lv_obj_t *)p_win;    /* Add win node at tail */
        /* print length of linked list */
        uint32_t len            = _lv_ll_get_len(&(p_app->z_ll));
        print_len_val(len);
    }
    else {
#ifndef WIN32
        aicx_info(0x06183d, "s", "There is NOT first win of this APP in [%s]\n", __func__);
#endif

    }

    return 0;
}

int32_t alv_am_app_exit_action(lv_obj_t *app, char *name, void *p_param)
{
    alv_am_app_t *p_app = NULL;

    /* Get obj of app */
    if (app) {
        p_app = (alv_am_app_t *)app;
    }
    else if (name) {
        /* Get app by name */
        p_app = (alv_am_app_t *)get_app_by_name(name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }
    alv_app_node_t *p_app_node = p_app->app_node;

    if(p_app_node->status==AM_APP_STATUS_FOCUSED){
        p_app_node->status  = AM_APP_STATUS_ACTIVED;

        alv_am_app_t *p_exit_app         = (alv_am_app_t *)p_app_node->app;

        /*if first win is null,use app mgr,first win is not null,use win mgr */
        if (p_exit_app->first_win) {
            alv_am_z_node_t *p_exit_node = (alv_am_z_node_t *)_lv_ll_get_tail(&(p_exit_app->z_ll));
            alv_am_win_t *p_exit_win = (alv_am_win_t *)p_exit_node->win;
            /* Defocus window of last APP*/
            if(p_exit_win->defocus) {
                p_exit_win->defocus((lv_obj_t *)p_exit_win, p_param);
            }

            //alog_info("p_exit_win->name1:%s",p_exit_win->name);
            /* Hide current window */
        }
        if(p_exit_app->defocus){
            p_exit_app->defocus((lv_obj_t *)p_exit_app, p_param);
        }

    }

    alv_app_node_t *p_last_node_app = _lv_ll_get_tail(&s_apps_ll);
    if(p_last_node_app==p_app_node)
    {
        /* 将要退出的app移动到队列头部 */
        alv_app_node_t *p_head_node = _lv_ll_get_head(&s_apps_ll);
        _lv_ll_move_before(&s_apps_ll, p_app->app_node, p_head_node);
        //p_app->app_node->status = AM_APP_STATUS_NO;

        /* Show prev APP */
        alv_app_node_t *p_node_app = _lv_ll_get_tail(&s_apps_ll);
        if(p_node_app->status==AM_APP_STATUS_ACTIVED)
        {
            p_node_app->status              = AM_APP_STATUS_FOCUSED;

            /* Show prev window */
            alv_am_app_t *p_last_app        = (alv_am_app_t *)p_node_app->app;

            /* last app focus*/
            if(p_last_app->focus){
                aicx_info(0x06f6fc, "s", "[%s],p_last_app->focus", __func__);
                p_last_app->focus((lv_obj_t *)p_last_app, p_param);
            }
            if(NULL != p_last_app->first_win)
            {
                alv_am_z_node_t *p_prev_node	= _lv_ll_get_tail(&(p_last_app->z_ll));
                alv_am_win_t *p_prev_win = (alv_am_win_t *)p_prev_node->win;

                if(p_prev_win->focus)
                    p_prev_win->focus((lv_obj_t *)p_prev_win,p_param);
            }

        }

    }
    else
    {
        /* 将要退出的app移动到队列头部 */
        alv_app_node_t *p_head_node = _lv_ll_get_head(&s_apps_ll);
        _lv_ll_move_before(&s_apps_ll, p_app->app_node, p_head_node);
    }

    //alog_info("p_app_node->name:%s,status:%d",p_app_node->name,p_app_node->status);
    if(p_app_node->status  == AM_APP_STATUS_ACTIVED){
        p_app_node->status  = AM_APP_STATUS_NO;

        alv_am_app_t *p_exit_app         = (alv_am_app_t *)p_app_node->app;
        /* Deactive current app last window */
        if (p_exit_app->first_win) {
           alv_am_z_node_t *p_exit_node = _lv_ll_get_tail(&(p_exit_app->z_ll));
           alv_am_win_t *p_exit_win;

           //alog_info("p_exit_node:0x%x",p_exit_node);
           while(p_exit_node){
                p_exit_win = (alv_am_win_t *)p_exit_node->win;
                //alog_info("p_exit_win->name:%s",p_exit_win->name);
                if (p_exit_win->deactive) {
                   p_exit_win->deactive((lv_obj_t *)p_exit_win, p_param);
                    }
                p_exit_node = _lv_ll_get_prev(&(p_exit_app->z_ll),p_exit_node);
           }

        }
        if(p_exit_app->deactive ) {
          p_exit_app->deactive((lv_obj_t *)p_exit_app, p_param);
        }
        /* clear app z order */
        _lv_ll_clear(&(p_exit_app->z_ll));
    }


    return 0;
}


int32_t alv_am_app_jump_action(lv_obj_t *app, char *name, void *p_param)
{
    alv_am_app_t *p_app = NULL;
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (app) {
        p_app = (alv_am_app_t *)app;
    }
    else if (name) {
        /* Get app by name */
        p_app = (alv_am_app_t *)get_app_by_name(name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if (NULL == p_app) {
#ifndef WIN32
        aicx_info(0x0fb195, "s", "p_app is NULL [%s]\n", __func__);
#endif
        return -1;
    }

    /* Have NOT Status Transfer */
    if ( AM_APP_STATUS_FOCUSED == p_app->app_node->status ) {
#ifndef WIN32
        aicx_info(0x13ff6a, "s", "Already in this current APP [%s]\n", __func__);
#endif
        return -1;
    }

    /* Status Transfer: NO=>ACTIVED */
    /* When status of the APP is NO Call event callback "active" */
    if ( AM_APP_STATUS_NO == p_app->app_node->status ) {
        if (p_app->active) {
            p_app->active((lv_obj_t *)p_app,p_param);
        }
    }
    p_app->app_node->status = AM_APP_STATUS_ACTIVED;

    /* Status Transfer: ACTIVED=>FOCUSED */
    /* When status of the APP is ACTIVED Call event callback "focus" */
    if ( AM_APP_STATUS_ACTIVED == p_app->app_node->status ) {
        /* add focus callback */
        if (p_app->focus) {
            p_app->focus((lv_obj_t *)p_app, p_param);
        }
    }

    /* Modify focused(current) APP, and save its window info */
    uint32_t num = _lv_ll_get_len(&s_apps_ll);
    print_len_val(num);

    /* Change status of APP which at tail */
    alv_app_node_t *p_last_node_app = _lv_ll_get_tail(&s_apps_ll);

    /* Save the focused obj of last app*/
    if(((alv_am_app_t *)p_last_node_app->app!=p_app)&&( AM_APP_STATUS_FOCUSED == p_last_node_app->status )) {
        p_last_node_app->status  = AM_APP_STATUS_ACTIVED;

        alv_am_app_t *p_last_app         = (alv_am_app_t *)p_last_node_app->app;
        /* TODO: Defocus last app */

        /*if first win is null,use app mgr,first win is not null,use win mgr */
        if (p_last_app->first_win) {
            alv_am_z_node_t *p_last_node_win = (alv_am_z_node_t *)_lv_ll_get_tail(&(p_last_app->z_ll));
            alv_am_win_t *p_last_win = (alv_am_win_t *)p_last_node_win->win;
            /* Defocus window of last APP*/
            if(p_last_win->defocus) {
                p_last_win->defocus((lv_obj_t *)p_last_win, p_param);
            }
            /* Hide current window */
        }
        if(p_last_app->defocus){
            p_last_app->defocus((lv_obj_t *)p_app, p_param);
        }

    }
    else {
        LV_LOG_WARN("The focused APP is NOT at the taill, or there is NO any APP yet. \n");
    }

    /* Move the node of this APP to tail */
    _lv_ll_remove(&s_apps_ll, (void *)p_app->app_node);
    lv_mem_free((void *)p_app->app_node);
    alv_app_node_t *p_node_app = _lv_ll_ins_tail(&s_apps_ll);
    p_node_app->name           = p_app->name;
    p_node_app->app            = (lv_obj_t *)p_app;
    p_node_app->status         = AM_APP_STATUS_FOCUSED;

    /* Save app node in app class */
    p_app->app_node            = p_node_app;

    /* Enter first window of this APP */
    if (p_app->first_win) {
        /* The app has fisrt win, means this app have many children (windows) managered by win mgr */
        p_win = (alv_am_win_t *)p_app->first_win;

        /* Make this screen active */
        //lv_disp_load_scr((lv_obj_t *)p_win);
        //lv_obj_move_foreground((lv_obj_t *)p_win);//TODO: 为什么lv_disp_load_scr((lv_obj_t *)p_win);不行，rootcause是？

        /* Active first window of this app */
        if (p_win->active) {
            p_win->active((lv_obj_t *)p_win, p_param);
        }
        else {
#ifndef WIN32
            aicx_info(0x16b253, "s", "There is NOT active function of this win in [%s]\n", __func__);
#endif
        }

        /* Focus first window of this app */
        if (p_win->focus) {
            p_win->focus((lv_obj_t *)p_win, p_param);
        }

        /* Add node into Z axis */
        alv_am_z_node_t *p_node = _lv_ll_ins_tail(&(p_app->z_ll));
        p_node->win             = (lv_obj_t *)p_win;    /* Add win node at tail */
        /* print length of linked list */
        uint32_t len            = _lv_ll_get_len(&(p_app->z_ll));
        print_len_val(len);
    }
    else {
#ifndef WIN32
        aicx_info(0x06183d, "s", "There is NOT first win of this APP in [%s]\n", __func__);
#endif

    }

    if(((alv_am_app_t *)p_last_node_app->app!=p_app)&&(AM_APP_STATUS_ACTIVED == p_last_node_app->status )) {
        p_last_node_app->status  = AM_APP_STATUS_NO;

        alv_am_app_t *p_last_app        = (alv_am_app_t *)p_last_node_app->app;
        /* Deactive current app last window */
        alv_am_z_node_t *p_last_node = _lv_ll_get_tail(&(p_last_app->z_ll));
        alv_am_win_t *p_last_win;
        while(p_last_node){
            p_last_win = (alv_am_win_t *)p_last_node->win;
            if (p_last_win->deactive) {
                p_last_win->deactive((lv_obj_t *)p_last_win, p_param);
            }
            p_last_node = _lv_ll_get_prev(&(p_last_app->z_ll),p_last_node);
        }

        if(p_last_app->deactive ) {
            p_last_app->deactive((lv_obj_t *)p_last_app, p_param);
        }
         /* clear app z order */
        _lv_ll_clear(&(p_last_app->z_ll));
    }



    return 0;
}


int32_t alv_am_app_exit_action_all()
{
    alv_app_node_t *p_last_node = _lv_ll_get_tail(&s_apps_ll);
    alv_app_node_t *p_cur_node,*p_tmp_node;

    p_cur_node = _lv_ll_get_prev(&s_apps_ll,p_last_node);

    while(p_cur_node!=NULL)
    {
        if(p_cur_node->status  == AM_APP_STATUS_NO)
        {
            break;
        }
        p_tmp_node = _lv_ll_get_prev(&s_apps_ll,p_cur_node);

        alv_am_app_exit_action(p_cur_node->app,NULL,NULL);

        p_cur_node = p_tmp_node;
    }

    alv_am_app_exit_action(p_last_node->app,NULL,NULL);
    alv_am_app_enter_action(NULL, "idle_fp", NULL);

    return 0;
}


int32_t alv_am_app_enter_defocus_action(lv_obj_t *app, char *name, void *p_param)
{
    alv_am_app_t *p_app = NULL;
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (app) {
        p_app = (alv_am_app_t *)app;
    }
    else if (name) {
        /* Get app by name */
        p_app = (alv_am_app_t *)get_app_by_name(name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if (NULL == p_app) {
#ifndef WIN32
        aicx_info(0x0fb195, "s", "p_app is NULL [%s]\n", __func__);
#endif
        return -1;
    }

    /* Have NOT Status Transfer */
    if ( AM_APP_STATUS_NO != p_app->app_node->status ) {
#ifndef WIN32
        aicx_info(0x13ff6a, "s", "Already in this current APP [%s]\n", __func__);
#endif
        return -1;
    }


    /* Status Transfer: NO=>ACTIVED */
    /* When status of the APP is NO Call event callback "active" */
    if (p_app->active) {
        p_app->active((lv_obj_t *)p_app,p_param);
    }
    p_app->app_node->status = AM_APP_STATUS_ACTIVED;

    /* Enter first window of this APP */
    if (p_app->first_win) {
        /* The app has fisrt win, means this app have many children (windows) managered by win mgr */
        p_win = (alv_am_win_t *)p_app->first_win;

        /* Active first window of this app */
        if (p_win->active) {
            p_win->active((lv_obj_t *)p_win, p_param);
        }
        else {
#ifndef WIN32
            aicx_info(0x16b253, "s", "There is NOT active function of this win in [%s]\n", __func__);
#endif
        }

        /* Add node into Z axis */
        alv_am_z_node_t *p_node = _lv_ll_ins_tail(&(p_app->z_ll));
        p_node->win             = (lv_obj_t *)p_win;    /* Add win node at tail */
        /* print length of linked list */
        uint32_t len            = _lv_ll_get_len(&(p_app->z_ll));
        print_len_val(len);
    }
    else {
#ifndef WIN32
        aicx_info(0x06183d, "s", "There is NOT first win of this APP in [%s]\n", __func__);
#endif

    }

    alv_app_node_t *p_last_node_app = _lv_ll_get_tail(&s_apps_ll);


    _lv_ll_move_before(&s_apps_ll,p_app->app_node,p_last_node_app);

    return 0;
}


int32_t alv_am_win_jump_action(lv_obj_t *win, char *name, void *p_param)
{
    alv_am_win_t *p_win = NULL;
    alv_am_win_t * p_cur_win = NULL;

    /* Get obj of win */
    if (win) {
        p_win = (alv_am_win_t *)win;
    }
    else if (name) {
        /* get win by name */
        lv_obj_t *p_cur_app = get_current_app();
        p_win = (alv_am_win_t *)get_win_by_name(p_cur_app,name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if(NULL == p_win) {
        aicx_error(0x17dbe2, "s", "[%s], p_win is null \n", __func__);
        return -1;
    }

    /* Get app by win */
    lv_obj_t *app = lv_obj_get_parent((lv_obj_t *)p_win);
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    /* Save focused */
    alv_am_z_node_t *p_cur_node = _lv_ll_get_tail(&(p_app->z_ll));/* Get current win node at tail */
    if(NULL != p_cur_node) {
        /* Remove node from Z axis */
        p_cur_win	 = (alv_am_win_t *)p_cur_node->win;
        _lv_ll_remove(&(p_app->z_ll), p_cur_node);
        lv_mem_free(p_cur_node);
        p_cur_node = NULL;
        /* Defocus current window*/
        if(p_cur_win->defocus) {
            p_cur_win->defocus((lv_obj_t *)p_cur_win, p_param);
        }


        /* Hide current window */
        //lv_obj_add_flag(p_cur_node->win, LV_OBJ_FLAG_HIDDEN);

    } else {
        // TODO: assert();
    }

    // TODO: make sure is OK for any window
    // lv_obj_move_foreground((lv_obj_t *)p_win);//TODO:为什么lv_disp_load_scr((lv_obj_t *)p_win); 不可以，rootcause是？
    //lv_disp_load_scr((lv_obj_t *)p_win); /*  Make this screen active */

    /* Active this window */
    if(p_win->active) {
        p_win->active((lv_obj_t *)p_win, p_param);
    }

    /* Focus this window */
    if(p_win->focus) {
        p_win->focus((lv_obj_t *)p_win, p_param);
    }

    /* Add node into Z axis */
    alv_am_z_node_t *p_node = _lv_ll_ins_tail(&(p_app->z_ll));
    p_node->win             = (lv_obj_t *)p_win;    /* Add win node at tail */
    /* print this */
    uint32_t len            = _lv_ll_get_len(&(p_app->z_ll));
    print_len_val(len);

    if(p_cur_win->deactive) {
        p_cur_win->deactive((lv_obj_t *)p_cur_win, p_param);
    }


    return 0;
}


int32_t alv_am_win_exit_action(lv_obj_t *win, char *name, void *p_param)
{
    alv_am_win_t *p_win = NULL;

    /* Get obj of app */
    if (win) {
        p_win = (alv_am_win_t *)win;
    }
    else if (name) {
        /* get win by name */
        lv_obj_t *p_cur_app = get_current_app();
        p_win = (alv_am_win_t *)get_win_by_name(p_cur_app,name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if(NULL == p_win) {
        aicx_error(0x1e691b, "ss", "%s, p_win is null [%s]\n", __func__);
        return -1;
    }

    /* Judge whether p_win is in Z axis */

    /* Get app by win */
    lv_obj_t *app = lv_obj_get_parent((lv_obj_t *)p_win);
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    uint32_t len = _lv_ll_get_len(&(p_app->z_ll));
    print_len_val(len);
    if (1 == len) {
        aicx_info(0x1001fa, "s", "There is only ONE window in APP in [%s]\n", __func__);
        alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_app->z_ll));
        if(p_win == (alv_am_win_t *)p_node->win){
            return alv_am_app_exit_action((lv_obj_t *)p_app,NULL,p_param);
        }
        else{
            return -1;
        }

    }

    alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_app->z_ll));
    if(p_win != (alv_am_win_t *)p_node->win){
        p_node = _lv_ll_get_prev(&(p_app->z_ll),p_node);
        while(p_node){
            if(p_win == (alv_am_win_t *)p_node->win){
                _lv_ll_remove(&(p_app->z_ll), p_node);
                lv_mem_free(p_node);
                p_node = NULL;
                if (p_win->deactive) {
                    p_win->deactive((lv_obj_t *)p_win, p_param);
                }
                return 0;
            }
            p_node = _lv_ll_get_prev(&(p_app->z_ll),p_node);
        }
       	return -1;

    }
    /* Defocus this win*/
    if (p_win->defocus) {
        p_win->defocus((lv_obj_t *)p_win, p_param);
    }

    /* Remove node from Z axis */
    _lv_ll_remove(&(p_app->z_ll), p_node);
    lv_mem_free(p_node);
    p_node = NULL;
    /* free p_node */
#ifndef CONFIG_APP_ENTRY_IDLE_FP
    /* Del all child obj of this win */
    uint32_t cnt = lv_obj_get_child_cnt((lv_obj_t *)p_win);
    for(uint32_t i = (cnt-1); i < cnt; i--) {  /* Must be "i--", because children is in an array whose id is start with "0" */
        lv_obj_t *child = lv_obj_get_child((lv_obj_t *)p_win, i);
        lv_obj_del(child);
    }
#endif
    /* Show prev window */
    alv_am_z_node_t *p_prev_node  = _lv_ll_get_tail(&(p_app->z_ll));
    alv_am_win_t *p_prev_win = (alv_am_win_t *)p_prev_node->win;
    lv_obj_clear_flag((lv_obj_t *)p_prev_win, LV_OBJ_FLAG_HIDDEN);

    /*  Make this screen active */
    //lv_disp_load_scr((lv_obj_t *)p_prev_win);
    //lv_obj_move_foreground((lv_obj_t *)p_prev_win);//TODO:为什么lv_disp_load_scr((lv_obj_t *)p_prev_win);不可以，rootcause是？

    /* Focus previous win*/
    if(p_prev_win->focus) {
        p_prev_win->focus((lv_obj_t *)p_prev_win, p_param);
        aicx_info(0x004c25, "ss", "[%s] focus win %s", __func__, p_prev_win->name);
    }
    /*TODO: keep previous settings now, del this later*/
    else if(p_prev_win->active) { /* TODO:exist memory leak,need add show in win lifecycle */
        //p_prev_win->active((lv_obj_t *)p_prev_win, p_param);
        aicx_info(0x0f85b1, "ss", "[%s] active win %s", __func__, p_prev_win->name);
    }

    /* Deactive this window*/
    /* Besure disp=>act_scr is not null,first fresh new scr, then delete old scr */
    if (p_win->deactive) {
        p_win->deactive((lv_obj_t *)p_win, p_param);
    }
    return 0;
}


int32_t alv_am_win_exit_action_by_app(char *app_name, char *win_name, void *p_param)
{
    alv_am_win_t *p_win = NULL;

   if (win_name) {
        /* get win by name */
        lv_obj_t *p_cur_app = get_current_app();
        lv_obj_t *p_tmp_app = get_app_by_name(app_name);
        if(p_cur_app==p_tmp_app)
        {
            return alv_am_win_exit_action(NULL,win_name,p_param);
        }
        p_win = (alv_am_win_t *)get_win_by_name(p_tmp_app,win_name);
    }
    else {
#ifndef WIN32
        aicx_info(0x10c62a, "s", "Input error in [%s]\n", __func__);
#endif
        return -1;
    }

    if(NULL == p_win) {
        aicx_error(0x1e691b, "ss", "%s, p_win is null [%s]\n", __func__);
        return -1;
    }

    /* Judge whether p_win is in Z axis */

    /* Get app by win */
    lv_obj_t *app = lv_obj_get_parent((lv_obj_t *)p_win);
    alv_am_app_t *p_app = (alv_am_app_t *)app;

    if(p_app->app_node->status!=AM_APP_STATUS_ACTIVED)
    {
        return -1;
    }
    uint32_t len = _lv_ll_get_len(&(p_app->z_ll));
    print_len_val(len);
    if (1 == len) {
        aicx_info(0x1001fa, "s", "There is only ONE window in APP in [%s]\n", __func__);

        alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_app->z_ll));
        if(p_win == (alv_am_win_t *)p_node->win){
            return alv_am_app_exit_action((lv_obj_t *)p_app,NULL,p_param);
        }
        else{
            return -1;
        }


    }

    alv_am_z_node_t *p_node = _lv_ll_get_tail(&(p_app->z_ll));

    while(p_node){
        if(p_win == (alv_am_win_t *)p_node->win){
            _lv_ll_remove(&(p_app->z_ll), p_node);
            lv_mem_free(p_node);
            p_node = NULL;
            if (p_win->deactive) {
                p_win->deactive((lv_obj_t *)p_win, p_param);
            }
            return 0;
        }
        p_node = _lv_ll_get_prev(&(p_app->z_ll),p_node);
    }

    return 0;
}
#endif
#endif

