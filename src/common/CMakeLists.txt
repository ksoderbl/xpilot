# src/common/CMakeLists.txt
file(GLOB COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_library(xpcommon STATIC ${COMMON_SOURCES})

target_include_directories(xpcommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/common
)

target_compile_definitions(xpcommon
    PUBLIC
        HAVE_CONFIG_H=1
)

set_target_properties(xpcommon PROPERTIES
    OUTPUT_NAME xpcommon
)
