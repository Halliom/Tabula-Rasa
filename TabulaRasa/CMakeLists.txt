cmake_minimum_required (VERSION 2.8)
project(TabulaRasa)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

set(GLEW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include")
set(GLEW_LIBRARY "${PROJECT_SOURCE_DIR}/Lib/")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PythonLibs REQUIRED)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  # using Clang
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  # using GCC
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Intel)
  # using Intel C++
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions("/W2")
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

set(GLEW32 "${CMAKE_SOURCE_DIR}/Lib/glew32.lib")
set(GLEW32S "${CMAKE_SOURCE_DIR}/Lib/glew32s.lib")

set(SDL2 "${CMAKE_SOURCE_DIR}/Lib/SDL2.lib")
set(SDL2MAIN "${CMAKE_SOURCE_DIR}/Lib/SDL2main.lib")
set(SDL2TEST "${CMAKE_SOURCE_DIR}/Lib/SDL2test.lib")

set(FREETYPE "${CMAKE_SOURCE_DIR}/Lib/freetype.lib")
set(FREETYPEBCC "${CMAKE_SOURCE_DIR}/Lib/freetype-bcc.lib")

set(LUA52 "${CMAKE_SOURCE_DIR}/Lib/lua52.lib")

set(ASSIMP "${CMAKE_SOURCE_DIR}/Lib/assimp.lib")

add_executable(TabulaRasa WIN32 ${SOURCE_FILES})
target_link_libraries(TabulaRasa ${OPENGL_LIBRARIES} ${GLEW32} ${GLEW32S} ${SDL2} ${SDL2MAIN} ${SDL2TEST} ${FREETYPE} ${LUA52} ${ASSIMP})
