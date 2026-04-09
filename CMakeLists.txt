# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

project(xpilot VERSION 4.5.5 LANGUAGES C)

include(GNUInstallDirs)

option(XPILOT_BUILD_CLIENT "Build xpilot-client" ON)
option(XPILOT_BUILD_SERVER "Build xpilot-server" ON)
option(XPILOT_BUILD_REPLAY "Build xpilot-replay" ON)
option(XPILOT_BUILD_MAPEDIT "Build xpilot-mapedit" ON)
option(XPILOT_INSTALL_DATA "Install xpilot data files" ON)
option(XPILOT_INSTALL_MAN "Install man pages" ON)

find_package(X11 REQUIRED)
find_library(M_LIB m REQUIRED)

# add_compile_options(
#     -Wall
#     -Wextra
#     -Wpedantic
#     -Wno-missing-field-initializers
#     -Wno-unused-parameter
# )

add_compile_options(
    -Wall
    -Wextra
    -Wformat=2
    -Wundef
    -Wshadow
    -Wpointer-arith
    -Wcast-qual
    -Wwrite-strings
    -Wmissing-declarations
    -Wuninitialized
    -Winit-self
    -Wredundant-decls
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wold-style-definition
    -Wno-unused-parameter
)

add_subdirectory(src)

if(XPILOT_INSTALL_DATA)
    install(DIRECTORY lib/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/xpilot
        OPTIONAL
    )
endif()

if(XPILOT_INSTALL_MAN)
    install(DIRECTORY doc/man/
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man6
        OPTIONAL
        FILES_MATCHING PATTERN "*.6"
    )

    install(FILES
        doc/man/xpilot.man
        doc/man/xpilots.man
        doc/man/xp-replay.man
        doc/man/xp-mapedit.man
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man6
        OPTIONAL
    )
endif()
