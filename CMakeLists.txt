cmake_minimum_required(VERSION 3.22.1)
set(PROJECT_TITLE "mandelbulb")
project(${PROJECT_TITLE})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Werror -Wall -Wno-long-long -pedantic")

# Build GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        external/imgui
        external/imgui/backends
        external/glm/glm
)

file(GLOB SOURCES src/*)
# Add ImGUI
file(GLOB IMGUI external/imgui/*.cpp)
file(GLOB IMGUI_BACKEND external/imgui/backends/imgui_impl_glfw.cpp external/imgui/backends/imgui_impl_opengl3.cpp)
add_executable(${PROJECT_TITLE} ${SOURCES} ${IMGUI} ${IMGUI_BACKEND})

target_link_libraries(
        ${PROJECT_TITLE}
        glfw
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
)
