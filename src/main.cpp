#include <tinylogger/tinylogger.h>
#include <glm/glm.hpp>

#include <filesystem>

#include "graphics/window.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/buffers.h"

#include "controls/devices.h"
#include "controls/controls.h"
#include "controls/camera.h"
#include "controls/gui.h"


const auto windowResolution = glm::vec2(600);
const auto simulationDimension = glm::vec3(800, 800, 1);

static void initGLEW()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        tlog::error() << "Error: " << glewGetErrorString(err);
        exit(EXIT_FAILURE);
    }

    // Get version info
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    if (renderer == nullptr || version == nullptr)
    {
        tlog::error() << "Failed to get OpenGL version info";
        exit(EXIT_FAILURE);
    }
    tlog::info() << "Renderer: " << renderer;
    tlog::info() << "OpenGL version supported: " << version;

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Set clear color
    glClearColor(0.098f, 0.094f, 0.094f, 1.0f);
}

static void initGLFW(graphics::Window &window)
{
    if (!window.init("2d-smoke-simulation", windowResolution.x, windowResolution.y))
    {
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window.getGLFWWindow());
    // Init user input
    glfwSetKeyCallback(window.getGLFWWindow(), controls::onKeyChange);
    glfwSetMouseButtonCallback(window.getGLFWWindow(), controls::onMouseChange);
    glfwSetCursorPosCallback(window.getGLFWWindow(), controls::onMouseMove);
    glfwSetScrollCallback(window.getGLFWWindow(), controls::onMouseScroll);
}

static float getAspectRatio()
{
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);
    return static_cast<float>(m_viewport[2]) / static_cast<float>(m_viewport[3]);
}

/*
static void setUniforms(std::unique_ptr<graphics::Shader> &shader, float h, float dt)
{
    shader->bind();
    shader->setUniform("h", h);
    shader->setUniform("dt", dt);
    shader->setUniform("screenResolution", graphics::Window::getGLFWWindow().getResolution());
    shader->setUniform("gridResolution", simulationDimension);
    shader->setUniform("totalIterations", gui->iterations);
    gui->setUniforms(shader);
    shader->unbind();
    void SmokeGUI::build(float dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Smoke Parameters", &show);
    ImGui::Text("FPS: %.1f", 1 / dt);
    reset = ImGui::Button("Reset");
    ImGui::Checkbox("Perform step", &performStep);
    ImGui::Checkbox("Show velocity field", &showVelocityField);
    ImGui::Checkbox("Show pressure field", &showPressureField);
    ImGui::Checkbox("Interpolate", &interpolate);
    ImGui::SliderFloat("Overrelaxation", &overrelaxation, 0, 4);
    ImGui::SliderInt("Incompressibility iterations", &iterations, 0, 50);
    ImGui::SliderFloat2("Gravity", &gravity[0], -10, 10);
    ImGui::SliderFloat("Density", &density, 0, 0.01);
    ImGui::End();
}

void SmokeGUI::setUniforms(const std::unique_ptr<Shader> &shader)
{
    shader->setUniform("gravity", gravity);
    shader->setUniform("overrelaxation", overrelaxation);
    shader->setUniform("density", density);
    shader->setUniform("iterations", iterations);
    shader->setUniform("showVelocityField", showVelocityField);
    shader->setUniform("showPressureField", showPressureField);
    shader->setUniform("interpolate", interpolate);
    shader->setUniform("reset", reset);
    reset = false;
}
}
*/


int main()
{
    // Print current working directory:
    tlog::info() << "Current working directory: " << std::filesystem::current_path();

    graphics::Window window;
    initGLFW(window);
    initGLEW();

    // Init ImGUI
    auto gui = controls::GUI(window.getGLFWWindow());

    // Init camera and controls
    auto aspectRatio = getAspectRatio();
    auto camera = controls::Camera(
        glm::vec3(0, 3, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(0, 0, 1)
    );
    auto pv = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();
    auto controls = controls::OrbitalControls(0.015f, 0.1f);
    bool focused = true;
    window.hideMouse();

    // Initalize the mesh and shader
    std::vector<graphics::Mesh::Vertex> vertices = {
        {glm::vec3(-1, 1, 0), glm::vec3(1, 0, 0), glm::vec2(0, 0)},
        {glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), glm::vec2(1, 0)},
        {glm::vec3(-1, -1, 0), glm::vec3(1, 0, 0), glm::vec2(0, 1)},
        {glm::vec3(1, -1, 0), glm::vec3(1, 0, 0), glm::vec2(1, 1)},
    };
    std::vector<unsigned int> indices = {
        2, 1, 0,
        1, 2, 3
    };
    auto screenQuad = graphics::Mesh(vertices, indices);

    // Compile shaders
    auto applyGravityShader = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/applyGravity.comp"}));
    auto forceIncompressibility = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/forceIncompressibility.comp"}));
    auto extrapolate = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/extrapolate.comp"}));
    auto advectVelocities = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/advectVelocities.comp"}));
    auto copyVelocityBuffer = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/copyVelocityBuffer.comp"}));
    auto advectSmoke = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/advectSmoke.comp"}));
    auto copySmokeBuffer = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/copySmokeBuffer.comp"}));
    auto smokeRenderShader = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/smoke.vert", "../assets/shader/smoke/smoke.frag"}));

    // Initialize SSBOs
    std::vector<float> uValues((simulationDimension.x + 1) * simulationDimension.y, 0);
    std::vector<float> vValues(simulationDimension.x * (simulationDimension.y + 1), 0);
    std::vector<float> pressure(simulationDimension.x * simulationDimension.y, 1.f);
    std::vector<float> obstacles(simulationDimension.x * simulationDimension.y, 1.f);
    std::vector<float> smoke(simulationDimension.x * simulationDimension.y, 1.f);

    const int xDispatches = simulationDimension.x / 16 + 1;
    const int yDispatches = simulationDimension.y / 16 + 1;

    auto uBuffer = graphics::SSBO<float>(uValues, 0);
    auto vBuffer = graphics::SSBO<float>(vValues, 1);
    auto nextUBuffer = graphics::SSBO<float>(uValues, 2);
    auto nextVBuffer = graphics::SSBO<float>(vValues, 3);
    auto obstacleBuffer = graphics::SSBO<float>(obstacles, 4);
    auto pressureBuffer = graphics::SSBO<float>(pressure, 5);
    auto smokeBuffer = graphics::SSBO<float>(smoke, 6);
    auto nextSmokeBuffer = graphics::SSBO<float>(smoke, 7);

    auto currTime = std::chrono::steady_clock::now();
    auto prevTime = currTime;
    while (!glfwWindowShouldClose(window.getGLFWWindow()) && !controls::Keyboard::getInstance().isPressed(GLFW_KEY_ESCAPE))
    {
        // Calculate delta time between frames
        auto currTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currTime - prevTime).count();
        auto prevTime = currTime;

        // Poll keyboard and mouse events
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        { // Update IO

            // Update camera
            aspectRatio = getAspectRatio();
            if (focused)
                controls.update(camera, dt);
            pv = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();

            // Reload shader
            if (controls::Keyboard::getInstance().isPressed(GLFW_KEY_R))
            {
                applyGravityShader.reload();
                forceIncompressibility.reload();
                extrapolate.reload();
                advectVelocities.reload();
                copyVelocityBuffer.reload();
                advectSmoke.reload();
                copySmokeBuffer.reload();
                smokeRenderShader.reload();
                controls::Keyboard::getInstance().setKeyState(GLFW_KEY_R, false);
            }
            if (controls::Keyboard::getInstance().isPressed(GLFW_KEY_F1))
            {
                if (focused) {
                    window.showMouse();
                } else {
                    window.hideMouse();
                }
                controls::Keyboard::getInstance().setKeyState(GLFW_KEY_F1, false);
                focused = !focused;
            }

            gui.build();
        }

        { // Update smoke simulation

            // Udpate shader uniforms
            //setUniforms(applyGravityShader, h, dt);
            //setUniforms(forceIncompressibility, h, dt);
            //setUniforms(extrapolate, h, dt);
            //setUniforms(advectVelocities, h, dt);
            //setUniforms(copyVelocityBuffer, h, dt);
            //setUniforms(advectSmoke, h, dt);
            //setUniforms(copySmokeBuffer, h, dt);
            //setUniforms(smokeRenderShader, h, dt);

            // Dispatch compute shaders
            applyGravityShader.dispatch(simulationDimension);
            // 2 executions per iteration for preventing race conditions by evaluating in checkboard pattern
            for (int i = 0; /*gui.performStep && i < 2 * gui.iterations*/ i < 10; i++)
            {
                forceIncompressibility.bind();
                forceIncompressibility.setUniform("currentIteration", i);
                forceIncompressibility.unbind();
                forceIncompressibility.dispatch(simulationDimension);
            }
            extrapolate.dispatch(simulationDimension);
            advectVelocities.dispatch(simulationDimension);
            copyVelocityBuffer.dispatch(simulationDimension);
            advectSmoke.dispatch(simulationDimension);
            copySmokeBuffer.dispatch(simulationDimension);
        }

        { // Render
            smokeRenderShader.bind();
            smokeRenderShader.setUniform("PV", pv);
            screenQuad.draw(smokeRenderShader);
            gui.render();
            smokeRenderShader.unbind();
        }

        glfwSwapBuffers(window.getGLFWWindow());
    }

    return EXIT_SUCCESS;
}
