#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <memory>

#include "shader.h"
#include "mesh.h"
#include "controls.h"
#include "window.h"
#include "gui.h"

#include <iostream>

std::unique_ptr<Shader> mandelbulbShader;
std::unique_ptr<Mesh> screenQuad;
std::unique_ptr<GUI> gui;

static void init()
{
    // Change the current directory to the directory of the executable
    if (chdir("..") != 0)
    {
        std::cerr << "Failed to change directory" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // start GLEW extension handler
    glewInit();
    // get version info
    const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte *version = glGetString(GL_VERSION);   // version as a string
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;
}

int main()
{
    init();
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Initalize the mesh and shader
    std::vector<Vertex> vertices = {
        {glm::vec3(-1, 1, 0), glm::vec2(0, 0)},
        {glm::vec3(1, 1, 0), glm::vec2(1, 0)},
        {glm::vec3(-1, -1, 0), glm::vec2(0, 1)},
        {glm::vec3(1, -1, 0), glm::vec2(1, 1)},
    };
    std::vector<unsigned int> indices = {
        0, 1, 2,
        1, 2, 3};
    screenQuad = std::make_unique<Mesh>(vertices, indices);
    mandelbulbShader = std::make_unique<Shader>("data/shader/mandelbulb");

    // Initialize GUI
    gui = std::make_unique<GUI>();

    auto previousTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(Window::getInstance().getGLFWWindow()))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate time between frames
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        float dt = deltaTime.count();

        // Update controls
        BaseControls *controls = getControls();
        controls->update(dt);

        // Build GUI
        gui->build();

        mandelbulbShader->bind();

        // Update shader uniforms 
        mandelbulbShader->setUniform("time", (float)glfwGetTime());
        mandelbulbShader->setUniform("resolution", Window::getInstance().getResolution());
        mandelbulbShader->setUniform("cameraPosition", controls->getCameraPosition());
        mandelbulbShader->setUniform("cameraDirection", controls->getCameraDirection());
        gui->update(mandelbulbShader);

        // Draw scene
        screenQuad->draw();
        gui->render();

        mandelbulbShader->unbind();

        glfwSwapBuffers(Window::getInstance().getGLFWWindow());
    }

    return EXIT_SUCCESS;
}
