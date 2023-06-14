# SmokeSimulation2DOpenCL
SmokeSimulation2DOpenCL is a C++ project that implements a 2D smoke simulation using an Eulerian grid-based approach. 
It utilizes the power of the GPU by accelerating computations through OpenCL compute shaders. 
The simulation consists of several steps performed in each iteration, including applying gravity on the velocity field, 
enforcing incompressibility within the field for a defined number of iterations, and applying advection.

![2023-06-14 14-56-22](https://github.com/Topxic/SmokeSimulation2DOpenCL/assets/50781880/bfc4b989-4a69-4e78-9e04-d4d70ea17a7f)

## Prerequisites
Before running SmokeSimulation2DOpenCL, make sure you have the following dependencies installed:

C++ compiler with C++14 support
CMake (version >= 3.22.1)
OpenGL and GLEW

## Installation
Clone the QuizManager repository:
```bash
git clone git@github.com:Topxic/SmokeSimulation2DOpenCL.git
cd SmokeSimulation2DOpenCL
```
Download git submodules:
```bash
git submodule update --init --recursive
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
