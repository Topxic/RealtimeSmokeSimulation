#include <filesystem>

#include <tinylogger/tinylogger.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "graphics/window.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/buffers.h"

#include "controls/devices.h"
#include "controls/controls.h"
#include "controls/camera.h"
#include "controls/gui.h"

#include "../3d/util.h"

#ifdef _WIN32
    #define ASSETS_PATH_RELATIVE "../../assets"
#else
    #define ASSETS_PATH_RELATIVE "../assets"
#endif


struct CloudParams {
    glm::vec3 cloudColor = glm::vec3(0, 1, 0);
    float absorption = 0.9f;
    float sampleStepSize = 0.01f;
    float time;

    int voronoiSampleRes = 64;
    int voronoiTextureRes = 1024;
    bool reloadVoronoi;
};

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
    glClearColor(0.088f, 0.084f, 0.084f, 1.0f);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void initGLFW(graphics::Window &window, const CloudParams &params)
{
    if (!window.init("3d-smoke-simulation", 800, 600))
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

static void buildGUI(CloudParams &params, float dt) 
{
    static bool show = false;

    ImGui::NewFrame();
    ImGui::Begin("Smoke Parameters", &show);
    ImGui::Text("FPS: %.1f", 1 / dt);
    ImGui::SliderFloat3("Cloud Color", &params.cloudColor[0], 0, 1);
    ImGui::SliderFloat("Absorption", &params.absorption, 0.0f, 1.0f);
    ImGui::SliderFloat("Sample Step Size", &params.sampleStepSize, 0.001f, 0.1f);
    ImGui::InputInt("Voronoi sample resolution", &params.voronoiSampleRes);
    ImGui::InputInt("Voronoi texture resolution", &params.voronoiTextureRes);
    params.reloadVoronoi = ImGui::Button("Update voronoi texture");
    ImGui::End();
}

static void setUniforms(graphics::Shader &shader, CloudParams &params, float dt)
{
    static float time = 0.0f;
    time += dt;
    shader.bind();
    shader.setUniform("cloudColor", params.cloudColor);
    shader.setUniform("absorption", params.absorption);
    shader.setUniform("sampleStepSize", params.sampleStepSize);
    shader.setUniform("time", time);
    shader.unbind();
}

static float randomFloat() 
{
    return (float)(rand()) / (float)(RAND_MAX);
}

static std::vector<glm::vec4> createVoronoiNoise(const CloudParams& params) 
{
    // Create voronoi texture
    const int texRes = params.voronoiTextureRes;
    const int gridRes = params.voronoiSampleRes;
    auto samples = std::vector<glm::vec2>(gridRes * gridRes);
    auto voronoiNoise = std::vector<glm::vec4>(texRes * texRes);
    srand(time(0));
    for (int i = 0; i < gridRes * gridRes; ++i) {
        samples[i].x = randomFloat();
        samples[i].y = randomFloat();
    }

    const float cellSize = texRes / static_cast<float>(gridRes);
    for (int x = 0; x < texRes; ++x) {
        for (int y = 0; y < texRes; ++y) {

            // Maximum possible distance
            float minDistance = glm::length(cellSize);
            glm::vec2 pixel = {x, y};
            glm::ivec2 gridCell = glm::ivec2(pixel / cellSize);

            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {

                    glm::vec2 cellIdx = gridCell + glm::ivec2{i, j};      

                    int idx = cellIdx.y * gridRes + cellIdx.x;
                    if (idx >= 0 && idx < gridRes * gridRes) {
                        glm::vec2 sample = (cellIdx + samples[idx]) * cellSize;
                        glm::vec2 diff = pixel - sample;
                        float dist = glm::length(diff);
                        minDistance = glm::min(minDistance, dist);
                    } 
                }
            }
            minDistance /= glm::length(cellSize);

            voronoiNoise[y * texRes + x] = glm::vec4(minDistance, minDistance, minDistance, 1.0);
        }
    }
    return voronoiNoise;
} 

int main()
{
    // Print current working directory:
    tlog::info() << "Current working directory: " << std::filesystem::current_path();
    tlog::info() << "Assets directory: " << ASSETS_PATH_RELATIVE;

    auto params = CloudParams();

    graphics::Window window;
    initGLFW(window, params);
    initGLEW();

    // Init ImGUI
    auto gui = controls::GUI(window.getGLFWWindow());

    // Init camera and controls
    auto aspectRatio = getAspectRatio();
    auto camera = controls::Camera(
        glm::vec3(2, 1, 2),
        glm::vec3(0, 0, 0)
    );
    auto pv = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();
    auto controls = controls::OrbitalControls(0.005f, 0.1f);
    bool focused = true;
    window.hideMouse();

    // Initalize the cube mesh
    auto cube = graphics::Mesh(createCubeVertices(glm::vec3(1, 1, 1)), createCubeIndices());

    // Compile shaders
    const std::string smokeShaders = std::string(ASSETS_PATH_RELATIVE) + "/shader/cloud";
    auto cloudShader = graphics::Shader(std::vector<std::string>({smokeShaders + "/cube.vert", smokeShaders + "/cube.frag"}));
    auto voronoiNoise = createVoronoiNoise(params);
    auto voronoiTex = graphics::Texture(voronoiNoise, params.voronoiTextureRes, params.voronoiTextureRes);

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
                cloudShader.reload();
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

        { // Update
            if (params.reloadVoronoi) {
                auto voronoiNoise = createVoronoiNoise(params);
                voronoiTex.update(voronoiNoise, 0, 0, params.voronoiTextureRes, params.voronoiTextureRes);
                params.reloadVoronoi = false;
            }
            // Update shader uniforms
            setUniforms(cloudShader, params, dt);
        }

        { // Render
            cloudShader.bind();
            cloudShader.setUniform("PV", pv);
            cloudShader.setUniform("cameraPos", camera.getPosition());
            voronoiTex.bind(0);
            cube.draw(cloudShader);
            gui.render();
            voronoiTex.unbind();
            cloudShader.unbind();
        }

        glfwSwapBuffers(window.getGLFWWindow());
    }

    return EXIT_SUCCESS;
}
