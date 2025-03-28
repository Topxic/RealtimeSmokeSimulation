cmake_minimum_required(VERSION 3.22.1)

# Define the main project
project(SmokeSimulations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Add EasyOpenGL as a subdirectory
add_subdirectory(external/EasyOpenGL)

# Define 2D Smoke Simulation project
set(PROJECT_2D "2d-smoke-simulation")
file(GLOB SRC_FILES_2D src/2d/*.cpp)
add_executable(${PROJECT_2D} ${SRC_FILES_2D})
target_link_libraries(${PROJECT_2D} PRIVATE EasyOpenGL)

# Define 3D Smoke Simulation project
set(PROJECT_3D "3d-smoke-simulation")
file(GLOB SRC_FILES_3D src/3d/*.cpp)
add_executable(${PROJECT_3D} ${SRC_FILES_3D})
target_link_libraries(${PROJECT_3D} PRIVATE EasyOpenGL)
