# src/server/CMakeLists.txt
file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(xpilot-server ${SERVER_SOURCES})

target_include_directories(xpilot-server
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(xpilot-server
    PRIVATE
        xpcommon
        ${M_LIB}
)

install(TARGETS xpilot-server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
