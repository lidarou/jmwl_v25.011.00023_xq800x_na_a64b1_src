if (NOT CONFIG_BL_SDK)
add_subdirectory(common)
endif()

# Add the Common/inc directory to the project include list
target_include_directories(boot_driver PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/common/inc
)
