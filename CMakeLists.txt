cmake_minimum_required(VERSION 3.24)

project(DeathLeak LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DEATHLEAK_BUILD_MAC_SMOKE "Build the macOS port smoke executable" ON)

if(DEATHLEAK_BUILD_MAC_SMOKE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    add_library(DeathLeakPortableCore STATIC
        Game/src/Core/DataValue.cpp
        Game/src/Core/Logger.cpp
        Game/src/Core/Timing.cpp
        Game/src/Core/Vector2.cpp
        Game/src/Core/Vector3.cpp
        Game/src/Core/Vector4.cpp
        Game/src/Core/XmlDocument.cpp
        Game/tinyxml/tinystr.cpp
        Game/tinyxml/tinyxml.cpp
        Game/tinyxml/tinyxmlerror.cpp
        Game/tinyxml/tinyxmlparser.cpp
    )

    target_include_directories(DeathLeakPortableCore PRIVATE
        Game/src
        Game/src/Core
        Game/tinyxml
    )

    target_compile_definitions(DeathLeakPortableCore PRIVATE
        DEATHLEAK_PLATFORM_MAC=1
    )

    add_executable(DeathLeakMacSmoke
        Game/src/Backends/Mac/MacSmokeMain.cpp
        Game/src/Backends/Mac/MacFileSystem.cpp
        Game/src/Backends/Mac/MacSdlInputSystem.cpp
        Game/src/Backends/Mac/MacSdlPlatformApp.cpp
        Game/src/Backends/Mac/MacSdlRenderer.cpp
    )

    target_include_directories(DeathLeakMacSmoke PRIVATE
        Game/src
        Game/src/Core
        Game/tinyxml
        ${SDL2_INCLUDE_DIRS}
    )

    target_compile_definitions(DeathLeakMacSmoke PRIVATE
        DEATHLEAK_PLATFORM_MAC=1
        SDL_MAIN_HANDLED=1
    )

    target_compile_options(DeathLeakMacSmoke PRIVATE
        ${SDL2_CFLAGS_OTHER}
    )

    target_link_directories(DeathLeakMacSmoke PRIVATE
        ${SDL2_LIBRARY_DIRS}
    )

    target_link_libraries(DeathLeakMacSmoke PRIVATE
        DeathLeakPortableCore
        ${SDL2_LIBRARIES}
    )
endif()
