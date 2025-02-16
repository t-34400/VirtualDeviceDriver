cmake_minimum_required(VERSION 3.10)

project(virtual_device_driver)

set(OPENVR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/openvr/headers)
set(OPENVR_LIB_DIR ${CMAKE_SOURCE_DIR}/openvr/lib)

add_subdirectory(openvr/samples/drivers/utils/driverlog)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

include_directories(${OPENVR_INCLUDE_DIR})

add_library(virtual_device_driver SHARED ${SOURCE_FILES})

target_link_libraries(virtual_device_driver ${OPENVR_LIB_DIR}/win64/openvr_api.lib)
target_link_libraries(virtual_device_driver util_driverlog)

set_target_properties(virtual_device_driver PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/win64
)

set_target_properties(virtual_device_driver PROPERTIES
    SUFFIX ".dll"
)
