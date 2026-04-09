# src/client/CMakeLists.txt
file(GLOB CLIENT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# These are optional / platform-specific legacy audio backends.
# The current Automake build only includes ncdaudio.c, not these.
list(REMOVE_ITEM CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/afaudio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/esdaudio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/hpaudio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/rplayaudio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/debugaudio.c"
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
