#include <filesystem>

#include <gl/glew.h>

#include "graphics/window.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/buffers.h"

#include "controls/devices.h"
#include "controls/controls.h"
#include "controls/camera.h"
#include "controls/gui.h"

#include "../util.h"

#include <imgui.h>
#include <glm/gtc/random.hpp>

#ifdef _WIN32
#define ASSETS_PATH_RELATIVE "../../assets"
#else
#define ASSETS_PATH_RELATIVE "../assets"
#endif

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

static inline float getAspectRatio()
{
    GLint m_viewport[4];
    glGetIntegerv(GL_VIEWPORT, m_viewport);
    return static_cast<float>(m_viewport[2]) / static_cast<float>(m_viewport[3]);
}

static void buildGUI(float dt, controls::PropertySystem &properties)
{
    ImGui::NewFrame();

    float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;
    static float fps_history[120] = { 0 };
    static int fps_index = 0;
    fps_history[fps_index] = fps;
    fps_index = (fps_index + 1) % IM_ARRAYSIZE(fps_history);

    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::NewLine();
    ImGui::PlotLines(
        "FPS Graph",
        fps_history,
        IM_ARRAYSIZE(fps_history),
        fps_index,
        nullptr,
        0.0f,
        144.0f,
        ImVec2(0, 80)
    );
    ImGui::End();

    properties.buildGUI();

    ImGui::EndFrame();
}

static void setUniforms(graphics::Shader &shader, float dt)
{
    shader.setUniform("dt", dt);
}

int main()
{
    // Print current working directory:
    tlog::info() << "Current working directory: " << std::filesystem::current_path();
    tlog::info() << "Assets directory: " << ASSETS_PATH_RELATIVE;

    auto propertiews = controls::PropertySystem(std::string(ASSETS_PATH_RELATIVE) + "/config/cloudProperties.json");

    graphics::Window window("Cloud", 1920, 1080);
    glfwMakeContextCurrent(window.getGLFWWindow());
    initGLEW();

    // Init ImGUI
    auto gui = controls::GUI(window.getGLFWWindow());

    // Init camera and controls
    auto aspectRatio = getAspectRatio();
    auto camera = controls::PerspectiveCamera(
        glm::vec3(2.0, 2.0, 2.0),
        glm::vec3(0, 0, 0));
    auto orbitControls = std::make_unique<controls::OrbitalControls>(0.005f, 0.1f, window.getContext(), glm::vec3(0.0, 0.0, 0.0));
    auto freeControls = std::make_unique<controls::FreeControls>(0.005f, 0.01f, window.getContext());
    controls::Controls *activeControls = orbitControls.get();

    bool focused = true;
    window.hideMouse();
    controls::Keyboard &keyboard = *window.getContext().keyboard;
    controls::Mouse &mouse = *window.getContext().mouse;

    // Initalize the cube mesh
    auto quad2DMesh = graphics::Mesh(geometry::quad2d::vertices(), geometry::quad2d::indices());

    // Compile shaders
    const std::string cloudShaderPath = std::string(ASSETS_PATH_RELATIVE) + "/shader/cloud";
    auto cloudShader = graphics::Shader(std::vector<std::string>({cloudShaderPath + "/quad.vert", cloudShaderPath + "/quad.frag"}));

    // Create noise textures
    auto voronoiNoise = voronoi::composedVoronoiNoise(
        propertiews.getValue<glm::ivec3>("voronoiResolution"),
        propertiews.getValue<float>("c1"),
        propertiews.getValue<float>("c2"),
        propertiews.getValue<float>("c3"),
        propertiews.getValue<float>("period"));
    auto voronoiTex = graphics::Texture3D(
        voronoiNoise,
        propertiews.getValue<glm::ivec3>("voronoiResolution"));

    auto fbmNoise = perlin::noiseFBM(
        propertiews.getValue<glm::ivec3>("fbmResolution"),
        propertiews.getValue<int>("octaveCount"),
        propertiews.getValue<float>("persistence"),
        propertiews.getValue<float>("lacunarity"),
        propertiews.getValue<float>("amplitude"),
        propertiews.getValue<float>("scale"),
        propertiews.getValue<int>("seed"),
        propertiews.getValue<float>("period"));
    auto fbmTex = graphics::Texture3D(
        fbmNoise,
        propertiews.getValue<glm::ivec3>("fbmResolution"));

    auto currTime = std::chrono::steady_clock::now();
    auto prevTime = currTime;
    while (!glfwWindowShouldClose(window.getGLFWWindow()))
    {
        // Calculate delta time between frames
        currTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currTime - prevTime).count();
        prevTime = currTime;

        // Poll keyboard and mouse events
        mouse.beginFrame();
        keyboard.beginFrame();
        glfwPollEvents();
        if (keyboard.down(GLFW_KEY_ESCAPE))
        {
            break;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        { // Update IO

            // Update camera
            aspectRatio = getAspectRatio();
            if (focused)
                activeControls->update(camera, dt);

            // Reload shader
            if (keyboard.pressed(GLFW_KEY_R))
            {
                cloudShader.reload();
            }
            if (keyboard.pressed(GLFW_KEY_F1))
            {
                if (focused)
                {
                    window.showMouse();
                }
                else
                {
                    window.hideMouse();
                }
                focused = !focused;
            }
            if (keyboard.pressed(GLFW_KEY_F2))
            {
                if (activeControls == orbitControls.get())
                {
                    activeControls = freeControls.get();
                }
                else
                {
                    activeControls = orbitControls.get();
                }
            }
        }

        { // Update
            gui.preBuild();
            buildGUI(dt, propertiews);

            if (propertiews.getValue<bool>("reloadFBM"))
            {
                auto fbmResolution = propertiews.getValue<glm::ivec3>("fbmResolution");
                fbmNoise = perlin::noiseFBM(
                    fbmResolution,
                    propertiews.getValue<int>("octaveCount"),
                    propertiews.getValue<float>("persistence"),
                    propertiews.getValue<float>("lacunarity"),
                    propertiews.getValue<float>("amplitude"),
                    propertiews.getValue<float>("scale"),
                    propertiews.getValue<int>("seed"),
                    propertiews.getValue<float>("period"));
                if (glm::any(glm::lessThan(fbmTex.resolution, fbmResolution)))
                {
                    fbmTex = graphics::Texture3D(fbmNoise, fbmResolution);
                }
                else
                {
                    fbmTex.update(fbmNoise, glm::ivec3(0), fbmResolution);
                }
                propertiews.setValue<bool>("reloadFBM", false);
            }

            if (propertiews.getValue<bool>("reloadVoronoi"))
            {
                auto voronoiResolution = propertiews.getValue<glm::ivec3>("voronoiResolution");
                voronoiNoise = voronoiNoise = voronoi::composedVoronoiNoise(
                    voronoiResolution,
                    propertiews.getValue<float>("c1"),
                    propertiews.getValue<float>("c2"),
                    propertiews.getValue<float>("c3"),
                    propertiews.getValue<float>("period"));
                if (glm::any(glm::lessThan(voronoiTex.resolution, voronoiResolution)))
                {
                    voronoiTex = graphics::Texture3D(voronoiNoise, voronoiResolution);
                }
                else
                {
                    voronoiTex.update(voronoiNoise, glm::ivec3(0), voronoiResolution);
                }
                propertiews.setValue<bool>("reloadVoronoi", false);
            }
        }

        { // Render
            // Render cloud
            cloudShader.bind();
            voronoiTex.bind(0);
            fbmTex.bind(1);

            cloudShader.setUniform("cameraEye", camera.getEye());
            cloudShader.setUniform("cameraCenter", camera.getCenter());
            cloudShader.setUniform("cameraUp", camera.getUp());
            cloudShader.setUniform("fovRad", glm::radians(camera.getFov()));
            cloudShader.setUniform("aspect", getAspectRatio());
            cloudShader.setUniform("voronoiTex", 0);
            cloudShader.setUniform("fbmTex", 1);
            quad2DMesh.draw(cloudShader);

            voronoiTex.unbind();
            fbmTex.unbind();
            cloudShader.unbind();

            // Render GUI
            gui.render();
        }

        glfwSwapBuffers(window.getGLFWWindow());
    }

    return EXIT_SUCCESS;
}
