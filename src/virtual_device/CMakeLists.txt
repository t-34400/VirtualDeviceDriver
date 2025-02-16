cmake_minimum_required(VERSION 3.10)

project(virtual_device)

file(GLOB_RECURSE SOURCE_FILES "${SRC_DIR}/virtual_device/*.cpp")

add_library(virtual_device STATIC ${SOURCE_FILES})

target_include_directories(virtual_device PRIVATE
    ${OPENVR_INCLUDE_DIR}
    ${DRIVERLOG_DIR}
    ${SRC_DIR}/osc_receiver
)

target_link_libraries(virtual_device PRIVATE
    ${OPENVR_LIB_DIR}/win64/openvr_api.lib
    util_driverlog
    osc_receiver
)

set_target_properties(virtual_device PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/win64
)
