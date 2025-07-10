cmake_minimum_required(VERSION 3.16)
project(NARC VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_SSE42 "Enable SSE 4.2 optimizations" ON)

# Find raylib
find_package(raylib 4.5 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

# Engine library
add_library(narc_engine STATIC
    engine/src/system/i_system.c
    engine/src/renderer/i_video.c
    engine/src/renderer/i_renderer.c
    engine/src/input/i_input.c
    engine/src/audio/i_audio.c
)

target_include_directories(narc_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/include
)

target_link_libraries(narc_engine PUBLIC raylib)

# Platform-specific sources
if(WIN32)
    target_sources(narc_engine PRIVATE
        engine/src/platform/win32/i_platform_win32.c
    )
    target_compile_definitions(narc_engine PRIVATE PLATFORM_WIN32)
elseif(APPLE)
    target_sources(narc_engine PRIVATE
        engine/src/platform/macos/i_platform_macos.c
    )
    target_compile_definitions(narc_engine PRIVATE PLATFORM_MACOS)
elseif(UNIX)
    target_sources(narc_engine PRIVATE
        engine/src/platform/linux/i_platform_linux.c
    )
    target_compile_definitions(narc_engine PRIVATE PLATFORM_LINUX)
endif()

# SSE 4.2 support
if(ENABLE_SSE42)
    if(MSVC)
        target_compile_options(narc_engine PRIVATE /arch:AVX)
    else()
        target_compile_options(narc_engine PRIVATE -msse4.2)
    endif()
    target_compile_definitions(narc_engine PRIVATE USE_SSE42)
endif()

# Game library
add_library(narc_game STATIC
    game/src/g_game.c
    game/src/g_player.c
    game/src/g_entities.c
    game/src/g_weapons.c
    game/src/g_raycaster.c
    game/src/g_pathfinding.c
    game/src/g_level.c
    game/src/g_menu.c
    game/src/g_hud.c
    game/src/g_config.c
)

target_include_directories(narc_game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/game/include
)

target_link_libraries(narc_game PUBLIC narc_engine)

# Main executable
add_executable(NARC
    game/src/main.c
)

target_link_libraries(NARC PRIVATE narc_game)

# Copy resources to build directory
add_custom_command(TARGET NARC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/DATA $<TARGET_FILE_DIR:NARC>/DATA
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/GFX $<TARGET_FILE_DIR:NARC>/GFX
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/SFX $<TARGET_FILE_DIR:NARC>/SFX
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Fonts $<TARGET_FILE_DIR:NARC>/Fonts
)

# Installation
install(TARGETS NARC DESTINATION bin)
install(DIRECTORY DATA GFX SFX Fonts DESTINATION share/narc)

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME "NARC")
set(CPACK_PACKAGE_VENDOR "NARC Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Not Another RayCaster - A retro FPS engine")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "ZIP;DragNDrop")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()