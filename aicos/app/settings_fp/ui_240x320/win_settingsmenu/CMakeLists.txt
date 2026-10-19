SET(SOURCES screens/ui_FpSettings.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_1_png.c
    images/ui_img_2_png.c
    images/ui_img_3_png.c
    images/ui_img_4_png.c
    images/ui_img_5_png.c
    images/ui_img_6_png.c
    images/ui_img_7_png.c
    images/ui_img_8_png.c
    images/ui_img_9_png.c
    images/ui_img_10_png.c
    images/ui_img_11_png.c)

add_library(ui ${SOURCES})
