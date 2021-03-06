﻿cmake_minimum_required(VERSION 2.8)
project(TabulaRasa)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# Where the libraries are
set(TB_LIB_DIR ${CMAKE_SOURCE_DIR}/Lib/${CMAKE_SYSTEM_NAME} CACHE FILEPATH "The path to the libraries")

# Where the source code is
set(TB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Source)

# Configuration types
if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE INTERNAL "") # makes it internal so it does not show up in the gui
else()
    if(NOT CMAKE_BUILD_TYPE) # if build type is not set (as it isn’t the first time), default to release
        message("Defaulting to release build.")
        set(CMAKE_BUILD_TYPE Debug CACHE INTERNAL "")
    endif()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
    # Set the valid options for cmake-gui drop-down list
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

# OpenGL is required and can be found on all platforms
find_package(OpenGL REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/Include/)

# /Engine source files
file(GLOB_RECURSE TB_ENGINE_SRC
    ${TB_SOURCE_DIR}/Engine/*.h
    ${TB_SOURCE_DIR}/Engine/*.cpp
)

# /Game source files
file(GLOB_RECURSE TB_GAME_SRC
    ${TB_SOURCE_DIR}/Game/*.h
    ${TB_SOURCE_DIR}/Game/*.cpp
)

# /Rendering source files
file(GLOB_RECURSE TB_RENDERING_SRC
    ${TB_SOURCE_DIR}/Rendering/*.h
    ${TB_SOURCE_DIR}/Rendering/*.cpp
)

set(SOURCE_FILES
    ${TB_ENGINE_SRC}
    ${TB_GAME_SRC}
    ${TB_RENDERING_SRC}
    ${TB_SOURCE_DIR}/main.cpp
    )

message(STATUS ${SOURCE_FILES})

if (WIN32)
    message("Target is Windows")

    set(GLFW3 ${TB_LIB_DIR}/glfw3.lib CACHE FILEPATH "glfw3.lib")

    set(GLEW32 ${TB_LIB_DIR}/glew32.lib CACHE FILEPATH "glew32.lib")
    set(GLEW32S ${TB_LIB_DIR}/glew32s.lib CACHE FILEPATH "glew32s.lib")

    set(FREETYPE ${TB_LIB_DIR}/freetype.lib CACHE FILEPATH "freetype.lib")

    set(LUA52 ${TB_LIB_DIR}/lua52.lib CACHE FILEPATH "lua52.lib")

    set(ASSIMP ${TB_LIB_DIR}/assimp.lib CACHE FILEPATH "assimp.lib")

    # Windows compiler flags
    add_definitions("/W0") #disables all warnings
    add_definitions("/Oi")
    add_definitions("/Ob1")
    add_definitions("/arch:SSE2")
    add_definitions("/fp:fast") # Fast floating point

    # Add the platform dependent code
    file(GLOB TB_PLATFORM_SRC
        ${TB_SOURCE_DIR}/Platform/WindowsPlatformFilesystem.cpp
        ${TB_SOURCE_DIR}/Platform/WindowsPlatformWindow.cpp
        )
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${TB_PLATFORM_SRC}
        )

    set(EXTERN_LIBS
        ${FREETYPEBCC}
        ${GLEW32}
        ${GLEW32S}
    )

    add_executable(TabulaRasa WIN32 ${SOURCE_FILES})
elseif (APPLE)
    message("Target is OSX")
    # let the user select where the libraries are
    set(GLFW3 ${TB_LIB_DIR}/libglfw3.a CACHE FILEPATH "libglfw3.a")

    set(GLEW32 ${TB_LIB_DIR}/libGLEW.a CACHE FILEPATH "libGLEW.a")

    set(LUA52 ${TB_LIB_DIR}/liblua52.a CACHE FILEPATH “liblua52.a”)

    set(ASSIMP ${TB_LIB_DIR}/libassimp.3.2.0.dylib CACHE FILEPATH “libassimp.3.2.0.dylib”)

    set(FREETYPE ${TB_LIB_DIR}/libfreetype.6.dylib CACHE FILEPATH “libfreetype.6.dylib”)

    # Find necessary apple libraries
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(FREETYPE FreeType)
    find_library(FORCE_FEEDBACK ForceFeedback)
    find_library(COCOA Cocoa)
    find_library(CARBON Carbon)
    find_library(IO_KIT IOKit)
    find_library(CORE_AUDIO CoreAudio)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(AUDIO_UNIT AudioUnit)
    find_library(CORE_VIDEO CoreVideo)
    find_library(ICONV Iconv)

    # Add the OS X specific libraries libraries
    set(EXTERN_LIBS
        ${GLEW32}
        ${CORE_FOUNDATION}
        ${FREETYPE}
        ${FORCE_FEEDBACK}
        ${COCOA}
        ${CARBON}
        ${IO_KIT}
        ${CORE_AUDIO}
        ${AUDIO_TOOLBOX}
        ${AUDIO_UNIT}
        ${CORE_VIDEO}
        ${ICONV}
        )

    # Windows defines the _DEBUG macro so to comply, define the _DEBUG macro
    add_definitions(-D_DEBUG)

    # Add the platform dependent code
    file(GLOB TB_PLATFORM_SRC
        ${TB_SOURCE_DIR}/Platform/OSXPlatformFilesystem.cpp
        ${TB_SOURCE_DIR}/Platform/OSXPlatformWindow.cpp
        )
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${TB_PLATFORM_SRC}
        )
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_executable(TabulaRasa MACOSX_BUNDLE ${SOURCE_FILES})
    else()
        add_executable(TabulaRasa ${SOURCE_FILES})
    endif()
endif()

set(EXTERN_LIBS
    ${EXTERN_LIBS}
    ${OPENGL_LIBRARIES}
    ${GLFW3}
    ${FREETYPE}
    ${LUA52}
    ${ASSIMP}
    )

message(STATUS “Architecture: ${CMAKE_SYSTEM_NAME}”)
message(STATUS ${EXTERN_LIBS})

target_compile_features(TabulaRasa PRIVATE cxx_constexpr)

target_link_libraries(TabulaRasa ${EXTERN_LIBS})
