#SimpleRealtimeSmokeSimulation
SimpleRealtimeSmokeSimulation is a lightweight C++ application that demonstrates a real-time 2D smoke simulation using a grid-based solver for incompressible fluid dynamics. The simulation is GPU-accelerated using OpenGL compute shaders, enabling interactive performance.
![](SmokeSimulation2DOpenCL.gif)

## Prerequisites
Before building and running SimpleRealtimeSmokeSimulation, make sure you have the following dependencies installed:
- C++ compiler with C++14 support
- CMake (version >= 3.22.1)
- OpenGL and GLEW

## Installation
Clone the repository and dependencies:
```bash
git clone git@github.com:Topxic/RealtimeSmokeSimulation.git --recursive
```
### Linux
Install dependencies:
```bash
sudo apt-get update
sudo apt-get install cmake g++ libx11-dev libxrandr-dev libxinerama-dev libgl1-mesa-dev libglew-dev libglfw3 libglfw3-dev
```
Build cmake project:
```bash
mkdir build
cd build
cmake ..
make all
```
### Windows (TODO)
