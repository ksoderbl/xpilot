# src/client/CMakeLists.txt
file(GLOB CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(xpilot-client ${CLIENT_SOURCES})

target_include_directories(xpilot-client
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${X11_INCLUDE_DIR}
)

target_link_libraries(xpilot-client
    PRIVATE
        xpcommon
        ${X11_LIBRARIES}
        ${M_LIB}
)

install(TARGETS xpilot-client
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
