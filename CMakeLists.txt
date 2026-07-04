cmake_minimum_required(VERSION 3.24)

project(DeathLeak LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DEATHLEAK_BUILD_MAC_SMOKE "Build the macOS port smoke executable" ON)

if(DEATHLEAK_BUILD_MAC_SMOKE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)

    set(DEATHLEAK_MAC_BUNDLE_EXECUTABLE_NAME "DeathLeakMacSmoke")
    set(DEATHLEAK_MAC_BUNDLE_IDENTIFIER "com.deathleak.smoke")
    set(DEATHLEAK_MAC_BUNDLE_NAME "Death Leak")
    set(DEATHLEAK_MAC_BUNDLE_SHORT_VERSION_STRING "0.1")
    set(DEATHLEAK_MAC_BUNDLE_VERSION "0.1")

    set(DEATHLEAK_MAC_SMOKE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/DeathLeakMacSmoke-Info.plist")
    configure_file(
        "${CMAKE_SOURCE_DIR}/Game/src/Backends/Mac/MacInfo.plist.in"
        "${DEATHLEAK_MAC_SMOKE_PLIST}"
        @ONLY
    )

    add_library(DeathLeakPortableCore STATIC
        Game/src/Core/DataValue.cpp
        Game/src/Core/Logger.cpp
        Game/src/Core/StringManager.cpp
        Game/src/Core/Timing.cpp
        Game/src/Core/Vector2.cpp
        Game/src/Core/Vector3.cpp
        Game/src/Core/Vector4.cpp
        Game/src/Core/XmlDocument.cpp
        Game/src/Engine/Runtime/GameLoopController.cpp
        Game/tinyxml/tinystr.cpp
        Game/tinyxml/tinyxml.cpp
        Game/tinyxml/tinyxmlerror.cpp
        Game/tinyxml/tinyxmlparser.cpp
        Game/src/PlayerLevelManager.cpp
        Game/src/SaveManager.cpp
        Game/src/Settings.cpp
        Game/src/SineWave.cpp
        Game/src/FeatureUnlockManager.cpp
        Game/src/LevelProperties.cpp
        Game/src/Animation/Animation.cpp
        Game/src/Animation/AnimationPart.cpp
        Game/src/Animation/AnimationSequence.cpp
        Game/src/Animation/AnimationSkeleton.cpp
        Game/src/GameObjects/Material.cpp
        Game/src/GameObjects/MaterialManager.cpp
        Game/src/GameObjects/LevelObjectRecord.cpp
        Game/src/GameObjects/SpriteRecord.cpp
    )

    target_include_directories(DeathLeakPortableCore PRIVATE
        Game/src
        Game/src/Core
        Game/src/AI
        Game/src/Animation
        Game/src/Audio
        Game/src/Backends/Mac
        Game/src/Effects
        Game/src/Engine
        Game/src/Engine/Platform
        Game/src/Engine/Runtime
        Game/src/GameObjects
        Game/src/Misc
        Game/src/Particles
        Game/src/Utils
        Game/src/UI
        Game/tinyxml
    )

    target_compile_definitions(DeathLeakPortableCore PRIVATE
        DEATHLEAK_PLATFORM_MAC=1
    )

    add_executable(DeathLeakMacSmoke MACOSX_BUNDLE
        Game/src/Backends/Mac/MacGameBootstrap.cpp
        Game/src/Backends/Mac/MacLaunchPaths.cpp
        Game/src/Backends/Mac/MacSmokeMain.cpp
        Game/src/Backends/Mac/MacFileSystem.cpp
        Game/src/Backends/Mac/MacNullAudioSystem.cpp
        Game/src/Backends/Mac/MacNullSteamService.cpp
        Game/src/Backends/Mac/MacSdlInputSystem.cpp
        Game/src/Backends/Mac/MacSdlPlatformApp.cpp
        Game/src/Backends/Mac/MacSdlRenderer.cpp
    )

    target_include_directories(DeathLeakMacSmoke PRIVATE
        Game/src
        Game/src/Core
        Game/src/AI
        Game/src/Animation
        Game/src/Audio
        Game/src/Backends/Mac
        Game/src/Effects
        Game/src/Engine
        Game/src/Engine/Platform
        Game/src/Engine/Runtime
        Game/src/GameObjects
        Game/src/Misc
        Game/src/Particles
        Game/src/Utils
        Game/src/UI
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
        "-framework AppKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework ImageIO"
    )

    set_target_properties(DeathLeakMacSmoke PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${DEATHLEAK_MAC_SMOKE_PLIST}"
        MACOSX_BUNDLE_BUNDLE_NAME "Death Leak"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.deathleak.smoke"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    )

    add_custom_command(TARGET DeathLeakMacSmoke POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:DeathLeakMacSmoke>/Contents/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Game/Media" "$<TARGET_BUNDLE_DIR:DeathLeakMacSmoke>/Contents/Resources/Media"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Game/XmlFiles" "$<TARGET_BUNDLE_DIR:DeathLeakMacSmoke>/Contents/Resources/XmlFiles"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Game/ShaderFiles" "$<TARGET_BUNDLE_DIR:DeathLeakMacSmoke>/Contents/Resources/ShaderFiles"
    )

    file(GLOB_RECURSE DEATHLEAK_MAC_GAME_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/Game/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/Game/tinyxml/*.cpp"
    )
    list(FILTER DEATHLEAK_MAC_GAME_SOURCES EXCLUDE REGEX ".*/Game/src/Windows/.*")
    list(FILTER DEATHLEAK_MAC_GAME_SOURCES EXCLUDE REGEX ".*/Game/src/Core/Graphics\\.cpp$")
    list(FILTER DEATHLEAK_MAC_GAME_SOURCES EXCLUDE REGEX ".*/Game/src/program\\.cpp$")
    list(FILTER DEATHLEAK_MAC_GAME_SOURCES EXCLUDE REGEX ".*/Game/src/Backends/Mac/MacSmokeMain\\.cpp$")
    list(FILTER DEATHLEAK_MAC_GAME_SOURCES EXCLUDE REGEX ".*/Game/src/Backends/Mac/MacGameBootstrap\\.cpp$")

    list(APPEND DEATHLEAK_MAC_GAME_SOURCES
        "${CMAKE_SOURCE_DIR}/Game/src/Backends/Mac/MacLaunchPaths.cpp"
        "${CMAKE_SOURCE_DIR}/Game/src/Core/GraphicsMac.cpp"
    )

    add_executable(DeathLeakMacGame MACOSX_BUNDLE
        ${DEATHLEAK_MAC_GAME_SOURCES}
    )

    set(DEATHLEAK_MAC_BUNDLE_EXECUTABLE_NAME "DeathLeakMacGame")
    set(DEATHLEAK_MAC_BUNDLE_IDENTIFIER "com.deathleak.game")
    set(DEATHLEAK_MAC_BUNDLE_NAME "Death Leak")
    set(DEATHLEAK_MAC_BUNDLE_SHORT_VERSION_STRING "0.1")
    set(DEATHLEAK_MAC_BUNDLE_VERSION "0.1")

    set(DEATHLEAK_MAC_GAME_PLIST "${CMAKE_CURRENT_BINARY_DIR}/DeathLeakMacGame-Info.plist")
    configure_file(
        "${CMAKE_SOURCE_DIR}/Game/src/Backends/Mac/MacInfo.plist.in"
        "${DEATHLEAK_MAC_GAME_PLIST}"
        @ONLY
    )

    target_include_directories(DeathLeakMacGame PRIVATE
        Game/src
        Game/src/Core
        Game/src/AI
        Game/src/Animation
        Game/src/Audio
        Game/src/Backends/Mac
        Game/src/Effects
        Game/src/Engine
        Game/src/Engine/Platform
        Game/src/Engine/Runtime
        Game/src/GameObjects
        Game/src/Misc
        Game/src/Particles
        Game/src/Utils
        Game/src/UI
        Game/tinyxml
        ${SDL2_INCLUDE_DIRS}
    )

    target_compile_definitions(DeathLeakMacGame PRIVATE
        DEATHLEAK_PLATFORM_MAC=1
        SDL_MAIN_HANDLED=1
    )

    target_compile_options(DeathLeakMacGame PRIVATE
        ${SDL2_CFLAGS_OTHER}
    )

    target_link_directories(DeathLeakMacGame PRIVATE
        ${SDL2_LIBRARY_DIRS}
    )

    target_link_libraries(DeathLeakMacGame PRIVATE
        ${SDL2_LIBRARIES}
        "-framework AppKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework ImageIO"
    )

    set_target_properties(DeathLeakMacGame PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${DEATHLEAK_MAC_GAME_PLIST}"
        MACOSX_BUNDLE_BUNDLE_NAME "Death Leak"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.deathleak.game"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    )

    add_custom_command(TARGET DeathLeakMacGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:DeathLeakMacGame>/Contents/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Game/Media" "$<TARGET_BUNDLE_DIR:DeathLeakMacGame>/Contents/Resources/Media"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Game/XmlFiles" "$<TARGET_BUNDLE_DIR:DeathLeakMacGame>/Contents/Resources/XmlFiles"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/Game/ShaderFiles" "$<TARGET_BUNDLE_DIR:DeathLeakMacGame>/Contents/Resources/ShaderFiles"
    )
endif()
