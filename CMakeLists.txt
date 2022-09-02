cmake_minimum_required(VERSION 3.23)
project(blob)

set(CMAKE_CXX_STANDARD 14)

# OpenGL
find_package(OpenGL REQUIRED)
message(STATUS "Found OpenGL in ${OpenGL_INCLUDE_DIR}")
include_directories(${OPENGL_INCLUDE_DIR})

add_subdirectory(glfw)

add_executable(blob main.cpp)

target_link_libraries(blob glfw ${OPENGL_LIBRARIES})
