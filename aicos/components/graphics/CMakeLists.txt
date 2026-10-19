
include_directories(lvgl)
add_subdirectory_ifdef(CONFIG_LV_CONF_MINIMAL lvgl)
add_subdirectory(hal)
add_subdirectory(alv)

aic_include_directories(
	./lvgl
	./hal
	./alv
)
