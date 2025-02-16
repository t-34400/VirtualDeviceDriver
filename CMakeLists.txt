cmake_minimum_required(VERSION 3.10)

project(virtual_device_driver)

set(OPENVR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/openvr/headers)
set(OPENVR_LIB_DIR ${CMAKE_SOURCE_DIR}/openvr/lib)
set(DRIVERLOG_DIR ${CMAKE_SOURCE_DIR}/openvr/samples/drivers/utils/driverlog)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

add_subdirectory(${DRIVERLOG_DIR})
add_subdirectory(${SRC_DIR}/osc_receiver)
add_subdirectory(${SRC_DIR}/virtual_device)

file(GLOB_RECURSE SOURCE_FILES "${SRC_DIR}/*.cpp")
add_library(virtual_device_driver SHARED ${SOURCE_FILES})

target_include_directories(virtual_device_driver PRIVATE
    ${OPENVR_INCLUDE_DIR}
    ${DRIVERLOG_DIR}
    ${SRC_DIR}/osc_receiver
    ${SRC_DIR}/virtual_device    
)

target_link_libraries(virtual_device_driver 
    ${OPENVR_LIB_DIR}/win64/openvr_api.lib
    util_driverlog
    osc_receiver
    virtual_device
)

set_target_properties(virtual_device_driver PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/win64
)

set_target_properties(virtual_device_driver PROPERTIES
    SUFFIX ".dll"
)
