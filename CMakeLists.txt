cmake_minimum_required(VERSION 3.22.1)
project(blob)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

# Build GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(
        inc
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        external/glm/glm
)

file(GLOB SOURCES src/*.cpp)
add_executable(blob ${SOURCES} inc/shader.hpp)

target_link_libraries(
        blob
        glfw
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
)
