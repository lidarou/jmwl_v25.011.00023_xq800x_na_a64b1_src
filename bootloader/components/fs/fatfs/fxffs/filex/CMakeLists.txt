if (NOT CONFIG_BL_SDK)
add_subdirectory(common)
endif()

target_include_directories(boot_driver PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/common/inc
)

target_include_directories(boot_driver PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/ports/cortex_mstar/gnu/inc
)
