cmake_minimum_required(VERSION 3.10)

project(osc_receiver)

file(GLOB_RECURSE SOURCE_FILES "${SRC_DIR}/osc_receiver/*.cpp")

add_library(osc_receiver STATIC ${SOURCE_FILES})

target_include_directories(osc_receiver PRIVATE
    ${OPENVR_INCLUDE_DIR}
    ${DRIVERLOG_DIR}
)

target_link_libraries(osc_receiver PRIVATE
    ${OPENVR_LIB_DIR}/win64/openvr_api.lib
    util_driverlog
)

set_target_properties(osc_receiver PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/win64
)
