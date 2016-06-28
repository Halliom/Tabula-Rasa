cmake_minimum_required (VERSION 2.8)
project(TabulaRasa)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# Configuration types
if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
   set(CMAKE_CONFIGURATION_TYPES "Debug;Release;" CACHE INTERNAL "") # makes it internal so it does not show up in the gui
else()
   if(NOT CMAKE_BUILD_TYPE) # if build type is not set (as it isn’t the first time), default to release
      message("Defaulting to release build.")
      set(CMAKE_BUILD_TYPE Debug CACHE INTERNAL "")
   endif()
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
   # Set the valid options for cmake-gui drop-down list
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

set(GLEW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include")
set(GLEW_LIBRARY "${PROJECT_SOURCE_DIR}/Lib/")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  # using Clang
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  # using GCC
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Intel)
  # using Intel C++
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions("/W0") #disables all warnings
    add_definitions("/Oi")
    add_definitions("/Ob1")
    add_definitions("/arch:SSE2")
    add_definitions("/fp:fast")
endif()

include_directories(${PROJECT_SOURCE_DIR}/Include/)

file(GLOB_RECURSE SOURCE_FILES
    "Source/*.h"
    "Source/*.cpp"
)

message(STATUS ${SOURCE_FILES})

if (WIN32)
   set(GLEW32 "${CMAKE_SOURCE_DIR}/Lib/glew32.lib")
   set(GLEW32S "${CMAKE_SOURCE_DIR}/Lib/glew32s.lib")

   set(SDL2 "${CMAKE_SOURCE_DIR}/Lib/SDL2.lib")
   set(SDL2MAIN "${CMAKE_SOURCE_DIR}/Lib/SDL2main.lib")
   set(SDL2TEST "${CMAKE_SOURCE_DIR}/Lib/SDL2test.lib")

   set(FREETYPE "${CMAKE_SOURCE_DIR}/Lib/freetype.lib")
   set(FREETYPEBCC "${CMAKE_SOURCE_DIR}/Lib/freetype-bcc.lib")

   set(LUA52 "${CMAKE_SOURCE_DIR}/Lib/lua52.lib")

   set(ASSIMP "${CMAKE_SOURCE_DIR}/Lib/assimp.lib")
elseif (APPLE)
   # let the user select where the libraries are
   set(SDL2 “SDL2.a” CACHE FILEPATH “SDL2.a”)
   set(SDL2MAIN “SDL2main.a” CACHE FILEPATH “SDL2main.a”)
   set(SDL2TEST “SDL2test.a” CACHE FILEPATH “SDL2test.a”)

   set(LUA52 “lua52.a” CACHE FILEPATH “liblua52.a”)

   set(ASSIMP “libassimp.a” CACHE FILEPATH “libassimp.a”)

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

   set(EXTRA 
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
endif()

message(STATUS “Architecture: ${CMAKE_SYSTEM_NAME}”)

add_executable(TabulaRasa WIN32 ${SOURCE_FILES})
target_compile_features(TabulaRasa PRIVATE cxx_constexpr)
target_link_libraries(TabulaRasa ${OPENGL_LIBRARIES} ${GLEW32} ${GLEW32S} ${SDL2} ${SDL2MAIN} ${SDL2TEST} ${FREETYPE} ${LUA52} ${ASSIMP} ${EXTRA})
