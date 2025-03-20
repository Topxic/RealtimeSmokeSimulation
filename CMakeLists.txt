cmake_minimum_required(VERSION 3.22.1)
set(PROJECT_NAME "2d-smoke-simulation")

cmake_minimum_required(VERSION 3.5)
project(${PROJECT_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Add GLFramework as a subdirectory (assuming it is cloned in "libs/GLFramework")
add_subdirectory(external/EasyOpenGL)

# Include the GLFramework headers
include_directories(
    external/EasyOpenGL/include
    external/EasyOpenGL/libs/glm
    external/EasyOpenGL/libs/imgui
    external/EasyOpenGL/libs/imgui/backends
    external/EasyOpenGL/libs/stb
    external/EasyOpenGL/libs/tinylogger
    external/EasyOpenGL/libs/tinyobjloader
)

# Source files for the application
file(GLOB SRC_FILES src/*.cpp)

# Create the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link against the GLFramework library
target_link_libraries(${PROJECT_NAME} PRIVATE EasyOpenGL glfw)
