# src/replay/CMakeLists.txt
file(GLOB REPLAY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(xpilot-replay ${REPLAY_SOURCES})

target_include_directories(xpilot-replay
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/client
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${X11_INCLUDE_DIR}
)

target_link_libraries(xpilot-replay
    PRIVATE
        xpcommon
        ${X11_LIBRARIES}
        ${M_LIB}
)

install(TARGETS xpilot-replay
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
