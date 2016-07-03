cmake_minimum_required(VERSION 2.8)
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
    set(GLEW32 ${TB_LIB_DIR}/glew32.lib)
    set(GLEW32S ${TB_LIB_DIR}/glew32s.lib)

    set(SDL2 ${TB_LIB_DIR}/SDL2.lib)
    set(SDL2MAIN ${TB_LIB_DIR}/SDL2main.lib)
    set(SDL2TEST ${TB_LIB_DIR}/SDL2test.lib)

    set(FREETYPE ${TB_LIB_DIR}/freetype.lib)
    set(FREETYPEBCC ${TB_LIB_DIR}/freetype-bcc.lib)

    set(LUA52 ${TB_LIB_DIR}/lua52.lib)

    set(ASSIMP ${TB_LIB_DIR}/assimp.lib)

    # Windows compiler flags
    add_definitions("/W0") #disables all warnings
    add_definitions("/Oi")
    add_definitions("/Ob1")
    add_definitions("/arch:SSE2")
    add_definitions("/fp:fast") # Fast floating point

    # Add the platform dependent code
    file(GLOB TB_PLATFORM_SRC
        ${TB_SOURCE_DIR}/Platform/WindowsPlatformFilesystem.*
        ${TB_SOURCE_DIR}/Platform/WindowsPlatformWindow.*
        )
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${TB_PLATFORM_SRC}
        )

    add_executable(TabulaRasa WIN32 ${SOURCE_FILES})
elseif (APPLE)
    # let the user select where the libraries are
    set(SDL2 “${TB_LIB_DIR}/libSDL2.a” CACHE FILEPATH “libSDL2.a”)
    set(SDL2MAIN “${TB_LIB_DIR}/libSDL2main.a” CACHE FILEPATH “libSDL2main.a”)
    set(SDL2TEST “${TB_LIB_DIR}/libSDL2_test.a” CACHE FILEPATH “libSDL2_test.a”)

    set(LUA52 “${TB_LIB_DIR}/liblua52.a” CACHE FILEPATH “liblua52.a”)

    set(ASSIMP “${TB_LIB_DIR}/libassimp.3.2.0.dylib” CACHE FILEPATH “libassimp.3.2.0.dylib”)

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
        ${TB_SOURCE_DIR}/Platform/OSXPlatformFilesystem.*
        ${TB_SOURCE_DIR}/Platform/OSXPlatformWindow.*
        )
    message(STATUS ${TB_PLATFORM_SRC})
    set(SOURCE_FILES
        ${SOURCE_FILES}
        ${TB_PLATFORM_SRC}
        )

    add_executable(TabulaRasa ${SOURCE_FILES})
endif()

set(EXTERN_LIBS
    ${EXTERN_LIBS}
    ${SDL2}
    ${SDL2MAIN}
    ${SDL2TEST}
    ${FREETYPE}
    ${LUA52}
    ${ASSIMP}
    )

message(STATUS “Architecture: ${CMAKE_SYSTEM_NAME}”)

target_compile_features(TabulaRasa PRIVATE cxx_constexpr)

target_link_libraries(TabulaRasa ${EXTERN_LIBS})
