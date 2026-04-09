# src/CMakeLists.txt
add_subdirectory(common)

if(XPILOT_BUILD_CLIENT)
    add_subdirectory(client)
endif()

if(XPILOT_BUILD_SERVER)
    add_subdirectory(server)
endif()

if(XPILOT_BUILD_REPLAY)
    add_subdirectory(replay)
endif()

if(XPILOT_BUILD_MAPEDIT)
    add_subdirectory(mapedit)
endif()
