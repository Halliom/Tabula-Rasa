cmake_minimum_required (VERSION 2.8)
project(TabulaRasa)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

set(GLEW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include")
set(GLEW_LIBRARY "${PROJECT_SOURCE_DIR}/Lib/")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PythonLibs REQUIRED)

message(${PYTHON_INCLUDE_DIRS})
message(${PYTHON_LIBRARIES})

if(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
    add_definitions(/W2)
endif()

include_directories(${PROJECT_SOURCE_DIR}/Include/ ${PYTHON_INCLUDE_DIRS})

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

add_executable(TabulaRasa WIN32 ${SOURCE_FILES})
target_link_libraries(TabulaRasa ${OPENGL_LIBRARIES} ${GLEW32} ${GLEW32S} ${SDL2} ${SDL2MAIN} ${SDL2TEST} ${PYTHON_LIBRARIES})

install(TARGETS TabulaRasa DESTINATION ${PROJECT_SOURCE_DIR}/Game/)