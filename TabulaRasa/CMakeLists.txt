cmake_minimum_required (VERSION 2.8)
project(TabulaRasa)

include_directories(${PROJECT_SOURCE_DIR}/Include/)

set(GLEW_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include")
set(GLEW_LIBRARY "${PROJECT_SOURCE_DIR}/Lib/")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

file(GLOB_RECURSE SOURCE_FILES
    "Source/*.h"
    "Source/*.cpp"
)

message(STATUS ${SOURCE_FILES})

set(GLEW32 "${CMAKE_SOURCE_DIR}/Lib/glew32.lib")
set(GLEW32S "${CMAKE_SOURCE_DIR}/Lib/glew32s.lib")

add_executable(TabulaRasa WIN32 ${SOURCE_FILES})
target_link_libraries(TabulaRasa ${OPENGL_LIBRARIES} ${GLEW32} ${GLEW32S})

install(TARGETS TabulaRasa DESTINATION ${PROJECT_SOURCE_DIR}/Game/)