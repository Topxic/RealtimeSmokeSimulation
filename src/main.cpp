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

std::unique_ptr<Shader> applyGravityShader;
std::unique_ptr<Shader> forceIncompressibility;
std::unique_ptr<Shader> extrapolate;
std::unique_ptr<Shader> advectVelocities;
std::unique_ptr<Shader> copyVelocityBuffer;
std::unique_ptr<Shader> advectSmoke;
std::unique_ptr<Shader> copySmokeBuffer;
std::unique_ptr<Shader> smokeRenderShader;

std::unique_ptr<Mesh> screenQuad;
std::unique_ptr<SmokeGUI> gui;

const auto simulationDimension = glm::vec2(800);

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

    // Initialize GUI
    gui = std::make_unique<SmokeGUI>();

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
}

static void setUniforms(std::unique_ptr<Shader> &shader, float h, float dt)
{
    shader->bind();
    shader->setUniform("h", h);
    shader->setUniform("dt", dt);
    shader->setUniform("screenResolution", Window::getInstance().getResolution());
    shader->setUniform("gridResolution", simulationDimension);
    shader->setUniform("totalIterations", gui->iterations);
    gui->setUniforms(shader);
    shader->unbind();
}

int main()
{
    init();

    applyGravityShader = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/applyGravity.comp"}));
    forceIncompressibility = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/forceIncompressibility.comp"}));
    extrapolate = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/extrapolate.comp"}));
    advectVelocities = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/advectVelocities.comp"}));
    copyVelocityBuffer = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/copyVelocityBuffer.comp"}));
    advectSmoke = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/advectSmoke.comp"}));
    copySmokeBuffer = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/copySmokeBuffer.comp"}));
    smokeRenderShader = std::make_unique<Shader>(std::vector<std::string>({"data/shader/smoke/smoke.vert", "data/shader/smoke/smoke.frag"}));

    // Initialize SSBOs
    std::vector<float> uValues((simulationDimension.x + 1) * simulationDimension.y, 0);
    std::vector<float> vValues(simulationDimension.x * (simulationDimension.y + 1), 0);
    std::vector<float> pressure(simulationDimension.x * simulationDimension.y, 1.f);
    std::vector<float> obstacles(simulationDimension.x * simulationDimension.y, 1.f);
    std::vector<float> smoke(simulationDimension.x * simulationDimension.y, 1.f);

    const int xDispatches = simulationDimension.x / 16 + 1;
    const int yDispatches = simulationDimension.y / 16 + 1;

    GLuint uBuffer = Shader::createBuffer(uValues, 0);
    GLuint vBuffer = Shader::createBuffer(vValues, 1);
    GLuint nextUBuffer = Shader::createBuffer(uValues, 2);
    GLuint nextVBuffer = Shader::createBuffer(vValues, 3);
    GLuint obstacleBuffer = Shader::createBuffer(obstacles, 4);
    GLuint pressureBuffer = Shader::createBuffer(pressure, 5);
    GLuint smokeBuffer = Shader::createBuffer(smoke, 6);
    GLuint nextSmokeBuffer = Shader::createBuffer(smoke, 7);

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
        if (controls->reload)
        {
            applyGravityShader->reload();
            forceIncompressibility->reload();
            extrapolate->reload();
            advectVelocities->reload();
            copyVelocityBuffer->reload();
            advectSmoke->reload();
            copySmokeBuffer->reload();
            smokeRenderShader->reload();
            controls->reload = false;
        }

        // Build GUI
        gui->build(dt);

        // Calculate grid spacing
        glm::vec2 ratio = Window::getInstance().getResolution() / simulationDimension;
        float h = glm::min(ratio.x, ratio.y);

        // Udpate shader uniforms
        dt = 1 / 120.f;
        setUniforms(applyGravityShader, h, dt);
        setUniforms(forceIncompressibility, h, dt);
        setUniforms(extrapolate, h, dt);
        setUniforms(advectVelocities, h, dt);
        setUniforms(copyVelocityBuffer, h, dt);
        setUniforms(advectSmoke, h, dt);
        setUniforms(copySmokeBuffer, h, dt);
        setUniforms(smokeRenderShader, h, dt);

        // Dispatch compute shaders
        applyGravityShader->dispatch(xDispatches, yDispatches);
        // 2 executions per iteration for preventing race conditions by evaluating in checkboard pattern
        for (int i = 0; gui->performStep && i < 2 * gui->iterations; i++)
        {
            forceIncompressibility->bind();
            forceIncompressibility->setUniform("currentIteration", i);
            forceIncompressibility->unbind();
            forceIncompressibility->dispatch(xDispatches, yDispatches);
        }
        extrapolate->dispatch(xDispatches, yDispatches);
        advectVelocities->dispatch(xDispatches, yDispatches);
        copyVelocityBuffer->dispatch(xDispatches, yDispatches);
        advectSmoke->dispatch(xDispatches, yDispatches);
        copySmokeBuffer->dispatch(xDispatches, yDispatches);

        // Render scene
        smokeRenderShader->bind();
        screenQuad->draw();
        gui->render();
        smokeRenderShader->unbind();

        glfwSwapBuffers(Window::getInstance().getGLFWWindow());
    }

    Shader::destroyBuffer(uBuffer);
    Shader::destroyBuffer(vBuffer);
    Shader::destroyBuffer(nextUBuffer);
    Shader::destroyBuffer(nextVBuffer);
    Shader::destroyBuffer(obstacleBuffer);
    Shader::destroyBuffer(pressureBuffer);
    Shader::destroyBuffer(smokeBuffer);
    Shader::destroyBuffer(nextSmokeBuffer);

    return EXIT_SUCCESS;
}
