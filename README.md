# Simple2DSmokeSimulation
SmokeSimulation2DOpenCL is a C++ project that implements a 2D smoke simulation using an Eulerian grid-based approach. 
It utilizes the power of the GPU by accelerating computations through compute shaders. 
The simulation consists of several steps performed in each iteration, including applying gravity on the velocity field, 
enforcing incompressibility within the field for a defined number of iterations, and applying advection.

![](SmokeSimulation2DOpenCL.gif)

## Prerequisites
Before running Simple2DSmokeSimulation, make sure you have the following dependencies installed:

C++ compiler with C++14 support
CMake (version >= 3.22.1)
OpenGL and GLEW

## Installation
Clone the repository:
```bash
git clone git@github.com:Topxic/SmokeSimulation2DOpenCL.git
cd SmokeSimulation2DOpenCL
```
Download git submodules:
```bash
git submodule update --init --recursive
```
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
make
```
Run the program:
```bash
./smoke-simulation
```
