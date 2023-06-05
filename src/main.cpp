#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdio.h>
#include <unistd.h>
#include <chrono>
#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.h"
#include "mesh.h"
#include "controls.h"
#include "window.h"

#include <iostream>

std::unique_ptr<Shader> mandelbulbShader;
std::unique_ptr<Mesh> screenQuad;

int main()
{
    // Change the current directory to the directory of the executable
    chdir("..");
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working directory: %s\n", cwd);

    // Init ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(Window::getInstance().getGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
    bool showDebug = false;

    // start GLEW extension handler
    glewInit();
    // get version info
    const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte *version = glGetString(GL_VERSION);   // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS);    // depth-testing interprets a smaller value as "closer"

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

    // Mandelbulb configuration
    int iterations = 25;
    float bailout = 3.0;
    float power = 8.0;

    // Raymarching configuration
    int maximalSteps = 120;
    float maximalDistance = 300.0;

    // Light configuration
    glm::vec3 lightDir = glm::vec3(0, 1, 0);
    bool antiAliasing = false;

    auto previousTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(Window::getInstance().getGLFWWindow()))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Rendering Options", &showDebug);
        ImGui::Checkbox("Anti Aliasing", &antiAliasing);
        ImGui::SliderInt("Maximal raymarching steps", &maximalSteps, 1, 300);
        ImGui::InputFloat("Maximal raymarching distance", &maximalDistance);
        ImGui::SliderFloat3("Light direction", &lightDir[0], -1, 1);
        ImGui::End();
        ImGui::Begin("Fractal Options", &showDebug);
        ImGui::SliderInt("Iterations", &iterations, 1, 300);
        ImGui::SliderFloat("Bailout", &bailout, .95f, 5.f);
        ImGui::SliderFloat("Power", &power, 1.f, 10.f, "%.6f");
        ImGui::End();
        lightDir = glm::normalize(lightDir);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentTime - previousTime;
        float dt = deltaTime.count();
        previousTime = currentTime;
        BaseControls *controls = getControls();
        controls->update(dt);

        mandelbulbShader->bind();
        mandelbulbShader->setUniform("antiAliasing", antiAliasing);
        mandelbulbShader->setUniform("time", (float)glfwGetTime());
        mandelbulbShader->setUniform("resolution", Window::getInstance().getResolution());
        mandelbulbShader->setUniform("cameraPosition", controls->getCameraPosition());
        mandelbulbShader->setUniform("cameraDirection", controls->getCameraDirection());

        mandelbulbShader->setUniform("iterations", iterations);
        mandelbulbShader->setUniform("bailout", bailout);
        mandelbulbShader->setUniform("power", power);
        mandelbulbShader->setUniform("maximalSteps", maximalSteps);
        mandelbulbShader->setUniform("maximalDistance", maximalDistance);
        mandelbulbShader->setUniform("lightDir", lightDir);
        screenQuad->draw();
        mandelbulbShader->unbind();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(Window::getInstance().getGLFWWindow());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return EXIT_SUCCESS;
}
