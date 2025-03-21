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
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

typedef struct {
    glm::vec3 gridResolution = glm::vec3(1024, 1024, 1);
    float gridSpacing = 1.1f;
    int totalIterations = 30;
    glm::vec3 gravity = glm::vec3(9.81, 0, 0);
    float overrelaxation = 1.9f;
    float density = 0.002f;
    bool showVelocityField = false;
    bool showPressureField = false;
    bool interpolate = true;
    bool reset = false;
    bool useFixedDT = true;
    float fixedDT = 1/60.f;
} SmokeParams;

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
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);

    // Enable backface culling
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    // Set clear color
    glClearColor(0.088f, 0.084f, 0.084f, 1.0f);
}

static void initGLFW(graphics::Window &window, const SmokeParams &params)
{
    if (!window.init("2d-smoke-simulation", 800, 600))
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

static inline float getAspectRatio()
{
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);
    return static_cast<float>(m_viewport[2]) / static_cast<float>(m_viewport[3]);
}

static void buildGUI(SmokeParams &params, float dt) 
{
    static bool show = false;

    ImGui::NewFrame();
    ImGui::Begin("Smoke Parameters", &show);
    ImGui::Text("FPS: %.1f", 1 / dt);
    ImGui::Checkbox("Use fixed dt", &params.useFixedDT);
    ImGui::SliderInt("Incompressability Iterations", &params.totalIterations, 0, 100);
    ImGui::SliderFloat("Fixed dt", &params.fixedDT, 0.001, 0.1);
    ImGui::SliderFloat("Grid Spacing", &params.gridSpacing, 0.001, 10);
    ImGui::SliderFloat("Overrelaxation", &params.overrelaxation, 0.1, 2.0);
    ImGui::SliderFloat3("Gravity", &params.gravity[0], -10, 10);
    ImGui::SliderFloat("Density", &params.density, 0, 0.01);
    ImGui::Checkbox("Show velocity field", &params.showVelocityField);
    ImGui::Checkbox("Show pressure field", &params.showPressureField);
    ImGui::Checkbox("Interpolate", &params.interpolate);
    params.reset = ImGui::Button("Reset");
    ImGui::End();
}

static void setUniforms(graphics::Shader &shader, SmokeParams &params, float dt)
{
    shader.bind();
    shader.setUniform("gridResolution", params.gridResolution);
    if (params.useFixedDT) {
        shader.setUniform("dt", params.fixedDT);
    } else {
        shader.setUniform("dt", dt);
    }
    shader.setUniform("h", params.gridSpacing);
    shader.setUniform("overrelaxation", params.overrelaxation);
    shader.setUniform("gravity", params.gravity);
    shader.setUniform("density", params.density);
    shader.setUniform("showVelocityField", params.showVelocityField);
    shader.setUniform("showPressureField", params.showPressureField);
    shader.setUniform("interpolate", params.interpolate);
    shader.setUniform("reset", params.reset);
    shader.setUniform("totalIterations", params.totalIterations);
    shader.unbind();
    params.reset = false;
}

int main()
{
    // Print current working directory:
    tlog::info() << "Current working directory: " << std::filesystem::current_path();

    auto params = SmokeParams();

    graphics::Window window;
    initGLFW(window, params);
    initGLEW();

    // Init ImGUI
    auto gui = controls::GUI(window.getGLFWWindow());

    // Init camera and controls
    auto aspectRatio = getAspectRatio();
    auto camera = controls::Camera(
        glm::vec3(0, 0, 1.2),
        glm::vec3(0, 0, -1),
        glm::vec3(0, 1, 0)
    );
    auto pv = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();
    auto controls = controls::OrbitalControls(0.015f, 0.1f);
    bool focused = true;
    window.hideMouse();

    // Initalize the mesh and shader
    std::vector<graphics::Mesh::Vertex> vertices = {
        {glm::vec3(1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 0)},
        {glm::vec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 1)},
        {glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)},
        {glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1)},
    };
    std::vector<unsigned int> indices = {
        0, 1, 2,
        3, 2, 1
    };
    auto screenQuad = graphics::Mesh(vertices, indices);

    // Compile shaders
    auto applyGravityShader = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/applyGravity.comp"}));
    auto forceIncompressibility = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/forceIncompressibility.comp"}));
    auto extrapolate = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/extrapolate.comp"}));
    auto advectVelocities = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/advectVelocities.comp"}));
    auto copyVelocityBuffer = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/copyVelocityBuffer.comp"}));
    auto advectSmoke = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/advectSmoke.comp"}));
    auto copySmokeBuffer = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/copySmokeBuffer.comp"}));
    auto smokeRenderShader = graphics::Shader(std::vector<std::string>({"../assets/shader/smoke/2d/quad.vert", "../assets/shader/smoke/2d/quad.frag"}));

    // Initialize SSBOs
    std::vector<float> uValues((params.gridResolution.x + 1) * params.gridResolution.y, 0);
    std::vector<float> vValues(params.gridResolution.x * (params.gridResolution.y + 1), 0);
    std::vector<float> pressure(params.gridResolution.x * params.gridResolution.y, 1.f);
    std::vector<float> obstacles(params.gridResolution.x * params.gridResolution.y, 1.f);
    std::vector<float> smoke(params.gridResolution.x * params.gridResolution.y, 1.f);

    const int xDispatches = params.gridResolution.x / 16 + 1;
    const int yDispatches = params.gridResolution.y / 16 + 1;

    auto uBuffer = graphics::SSBO<float>(uValues, 0);
    auto vBuffer = graphics::SSBO<float>(vValues, 1);
    auto nextUBuffer = graphics::SSBO<float>(uValues, 2);
    auto nextVBuffer = graphics::SSBO<float>(vValues, 3);
    auto obstacleBuffer = graphics::SSBO<float>(obstacles, 4);
    auto pressureBuffer = graphics::SSBO<float>(pressure, 5);
    auto smokeBuffer = graphics::SSBO<float>(smoke, 6);
    auto nextSmokeBuffer = graphics::SSBO<float>(smoke, 7);

    auto dispatchSize = glm::ivec3(params.gridResolution) / 16 + 1;
 
    auto currTime = std::chrono::steady_clock::now();
    auto prevTime = currTime;
    while (!glfwWindowShouldClose(window.getGLFWWindow()) && !controls::Keyboard::getInstance().isPressed(GLFW_KEY_ESCAPE))
    {
        // Calculate delta time between frames
        currTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currTime - prevTime).count();
        prevTime = currTime;

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

            gui.preBuild();
            buildGUI(params, dt);
        }

        { // Update smoke simulation

            // Update shader uniforms
            setUniforms(applyGravityShader, params, dt);
            setUniforms(forceIncompressibility, params, dt);
            setUniforms(extrapolate, params, dt);
            setUniforms(advectVelocities, params, dt);
            setUniforms(copyVelocityBuffer, params, dt);
            setUniforms(advectSmoke, params, dt);
            setUniforms(copySmokeBuffer, params, dt);
            setUniforms(smokeRenderShader, params, dt);

            // Dispatch compute shaders
            applyGravityShader.dispatch(dispatchSize);
            // Execute twice per iteration for preventing race conditions by evaluating in checkboard pattern
            for (int i = 0; i < 2 * params.totalIterations; i++)
            {
                forceIncompressibility.bind();
                forceIncompressibility.setUniform("currentIteration", i);
                forceIncompressibility.unbind();
                forceIncompressibility.dispatch(dispatchSize);
            }
            extrapolate.dispatch(dispatchSize);
            advectVelocities.dispatch(dispatchSize);
            copyVelocityBuffer.dispatch(dispatchSize);
            advectSmoke.dispatch(dispatchSize);
            copySmokeBuffer.dispatch(dispatchSize);
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
