# src/mapedit/CMakeLists.txt
file(GLOB MAPEDIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(xpilot-mapedit ${MAPEDIT_SOURCES})

target_include_directories(xpilot-mapedit
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${X11_INCLUDE_DIR}
)

target_link_libraries(xpilot-mapedit
    PRIVATE
        xpcommon
        ${X11_LIBRARIES}
        ${M_LIB}
)

install(TARGETS xpilot-mapedit
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
