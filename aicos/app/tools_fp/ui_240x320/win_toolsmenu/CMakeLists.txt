SET(SOURCES screens/ui_FpTools.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_calculatekey_png.c
    images/ui_img_divide_png.c
    images/ui_img_minus_png.c
    images/ui_img_multiply_png.c
    images/ui_img_plus_png.c
    images/ui_img_equal_png.c
	
add_library(ui ${SOURCES})
