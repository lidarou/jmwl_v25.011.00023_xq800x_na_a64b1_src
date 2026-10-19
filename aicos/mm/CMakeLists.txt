cmake_minimum_required(VERSION 3.13)

#
# Create Library and add Curent Library to project library list
#
aic_library_named(mm)

add_subdirectory_ifdef(CONFIG_AUDIO_SUPPORT audio)
add_subdirectory_ifdef(CONFIG_AUDIO_SUPPORT codec)
add_subdirectory_ifdef(CONFIG_VOLTE_SIMU volte_simu)
add_subdirectory_ifdef(CONFIG_CAMERA_SUPPORT camera)
