#include <filesystem>

#include <tinylogger/tinylogger.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
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

#include "../util.h"

#ifdef _WIN32
#define ASSETS_PATH_RELATIVE "../../assets"
#else
#define ASSETS_PATH_RELATIVE "../assets"
#endif

struct CloudParams
{
    glm::vec3 cloudColor = {1, 1, 1};
    float absorption = 70.0f;
    float sampleStepSize = 0.04f;
    float time;
    float border = 0.07;

    // Noise calculation
    float period = 0.5;
    glm::ivec3 voronoiResolution = {32, 32, 32};
    float c1 = 0.566;
    float c2 = 0.913;
    float c3 = 0.986;
    glm::ivec3 fbmResolution = {32, 32, 32};
    float amplitude = 1.0;
    int octaveCount = 5;
    float persistence = 0.5;
    float lacunarity = 2.0;
    float scale = 4.0;
    int seed = 2309461;

    // Lightning
    float henyeyGreen_G = 0.9;
    float henyeyGreen_K = 0.8;
    float lightIntensity = 40.0;
    glm::vec3 lightPosition = {8.0, 8.0, 0.0};
    glm::vec3 lightColor = {0.9, 0.9, 0.5};
    float exposure = 1.0;
    float gamma = 2.2;

    // Debug utility
    bool reloadVoronoi;
    bool reloadPerlin;
    bool showPerlin;
    bool showVoronoi;
    float showTextureDepth;
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
    if (!window.init("cloud", 1920, 1080))
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
    ImGui::ColorPicker3("Cloud Color", &params.cloudColor[0]);
    ImGui::SliderFloat("Absorption", &params.absorption, 0.0f, 300.0f);
    ImGui::SliderFloat("Border", &params.border, 0.0f, 1.0f);
    ImGui::SliderFloat("Sample Step Size", &params.sampleStepSize, 0.001f, 0.1f);
    ImGui::SliderFloat("Light intensity", &params.lightIntensity, 0.0f, 1000.0f);

    static bool animateLight;
    ImGui::Checkbox("Animate light", &animateLight);

    if (animateLight)
    {
        static float theta = 0.0f;                // azimuthal angle
        static float phi = glm::half_pi<float>(); // polar angle, start at equator

        // Drift parameters
        static float driftSpeed = 0.2f;                                            // radians per second
        static float driftTarget = glm::linearRand(0.1f, glm::pi<float>() - 0.1f); // avoid exact poles

        // Update azimuth (always rotates around Y)
        theta += 0.5f * dt; // circular motion speed

        // Smoothly drift polar angle toward target
        phi = glm::mix(phi, driftTarget, 0.01f);

        // Occasionally pick a new drift target
        static float timer = 0.0f;
        timer += dt;
        if (timer > 5.0f)
        { // new target every 5 seconds
            timer = 0.0f;
            driftTarget = glm::linearRand(0.1f, glm::pi<float>() - 0.1f);
        }

        float radius = 8.0f;
        params.lightPosition = glm::vec3(
            radius * glm::sin(phi) * glm::cos(theta),
            radius * glm::cos(phi),
            radius * glm::sin(phi) * glm::sin(theta));
    }

    ImGui::SliderFloat3("Light position", &params.lightPosition[0], -10.0f, 10.0f);
    ImGui::ColorPicker3("Light color", &params.lightColor[0]);
    ImGui::SliderFloat("Exposure", &params.exposure, 0.0f, 10.0f);
    ImGui::SliderFloat("Gamma", &params.gamma, 0.0f, 10.0f);

    ImGui::SliderFloat("Noise period", &params.period, 0.0f, 1.0f);
    ImGui::InputInt3("Voronoi texture resolution", &params.voronoiResolution[0]);
    ImGui::SliderFloat("Voronoi c1", &params.c1, 0.0f, 1.0f);
    ImGui::SliderFloat("Voronoi c2", &params.c2, 0.0f, 1.0f);
    ImGui::SliderFloat("Voronoi c3", &params.c3, 0.0f, 1.0f);
    params.reloadVoronoi = ImGui::Button("Update voronoi texture");

    ImGui::InputInt3("FBM texture resolution", &params.fbmResolution[0]);
    ImGui::InputInt("Perlin octave count", &params.octaveCount);
    ImGui::InputFloat("Perlin persistence", &params.persistence);
    ImGui::InputFloat("Perlin lacunarity", &params.lacunarity);
    ImGui::InputFloat("Perlin amplitude", &params.amplitude);
    ImGui::InputFloat("Perlin scale", &params.scale);
    ImGui::InputInt("Perlin seed", &params.seed);
    params.reloadPerlin = ImGui::Button("Update perlin texture");

    ImGui::SliderFloat("Henyey-Greenstein G", &params.henyeyGreen_G, -1.0f, 1.0f);
    ImGui::SliderFloat("Henyey-Greenstein K", &params.henyeyGreen_K, 0.0f, 1.0f);

    ImGui::Checkbox("Show voronoi", &params.showVoronoi);
    ImGui::Checkbox("Show perlin", &params.showPerlin);
    ImGui::SliderFloat("Texture depth", &params.showTextureDepth, 0.0f, 1.0f);
    ImGui::End();
}

static void setUniforms(graphics::Shader &shader, CloudParams &params, float dt)
{
    params.time += dt;

    shader.setUniform("cloudColor", params.cloudColor);
    shader.setUniform("absorption", params.absorption);
    shader.setUniform("sampleStepSize", params.sampleStepSize);

    shader.setUniform("time", params.time);
    shader.setUniform("dt", dt);
    shader.setUniform("border", params.border);

    shader.setUniform("showPerlin", params.showPerlin);
    shader.setUniform("showVoronoi", params.showVoronoi);
    shader.setUniform("showTextureDepth", params.showTextureDepth);

    shader.setUniform("lightIntensity", params.lightIntensity);
    shader.setUniform("lightPosition", params.lightPosition);
    shader.setUniform("lightColor", params.lightColor);
    shader.setUniform("henyeyGreen_G", params.henyeyGreen_G);
    shader.setUniform("henyeyGreen_K", params.henyeyGreen_K);
    shader.setUniform("gamma", params.gamma);
    shader.setUniform("exposure", params.exposure);
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
        glm::vec3(1, 0.5, 1),
        glm::vec3(0, 0, 0));
    auto pv = camera.getProjectionMatrix(aspectRatio) * camera.getViewMatrix();
    auto controls = controls::OrbitalControls(0.005f, 0.1f);
    bool focused = true;
    window.hideMouse();

    // Initalize the cube mesh
    auto cubeMesh = graphics::Mesh(geometry::box3d::vertices(), geometry::box3d::indices());
    auto lightMesh = graphics::Mesh(geometry::sphere3d::vertices(), geometry::sphere3d::indices());

    // Compile shaders
    const std::string cloudShaderPath = std::string(ASSETS_PATH_RELATIVE) + "/shader/cloud";
    auto cloudShader = graphics::Shader(std::vector<std::string>({cloudShaderPath + "/cube.vert", cloudShaderPath + "/cloud.frag"}));
    auto lightShader = graphics::Shader(std::vector<std::string>({cloudShaderPath + "/sphere.vert", cloudShaderPath + "/light.frag"}));

    // Create noise textures
    auto voronoiNoise = voronoi::composedVoronoiNoise(params.voronoiResolution, params.c1, params.c2, params.c3, params.period);
    auto voronoiTex = graphics::Texture3D(voronoiNoise, params.voronoiResolution);
    auto fbmNoise = perlin::noiseFBM(params.fbmResolution, params.octaveCount, params.persistence, params.lacunarity, params.amplitude, params.scale, params.seed, params.period);
    auto fbmTex = graphics::Texture3D(fbmNoise, params.fbmResolution);

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
                lightShader.reload();
                controls::Keyboard::getInstance().setKeyState(GLFW_KEY_R, false);
            }
            if (controls::Keyboard::getInstance().isPressed(GLFW_KEY_F1))
            {
                if (focused)
                {
                    window.showMouse();
                }
                else
                {
                    window.hideMouse();
                }
                controls::Keyboard::getInstance().setKeyState(GLFW_KEY_F1, false);
                focused = !focused;
            }

            gui.preBuild();
            buildGUI(params, dt);
        }

        { // Update
            if (params.reloadPerlin)
            {
                fbmNoise = perlin::noiseFBM(params.fbmResolution, params.octaveCount, params.persistence, params.lacunarity, params.amplitude, params.scale, params.seed, params.period);
                if (glm::any(glm::lessThan(fbmTex.resolution, params.fbmResolution)))
                {
                    fbmTex = graphics::Texture3D(fbmNoise, params.fbmResolution);
                }
                else
                {
                    fbmTex.update(fbmNoise, glm::ivec3(0), glm::ivec3(params.fbmResolution));
                }
                params.reloadPerlin = false;
            }
            if (params.reloadVoronoi)
            {
                voronoiNoise = voronoi::composedVoronoiNoise(params.voronoiResolution, params.c1, params.c2, params.c3, params.period);
                if (glm::any(glm::lessThan(voronoiTex.resolution, params.voronoiResolution)))
                {
                    voronoiTex = graphics::Texture3D(voronoiNoise, params.voronoiResolution);
                }
                else
                {
                    voronoiTex.update(voronoiNoise, glm::ivec3(0), glm::ivec3(params.voronoiResolution));
                }
                params.reloadVoronoi = false;
            }
        }

        { // Render

            // Render light source
            lightShader.bind();
            lightShader.setUniform("PV", pv);
            setUniforms(lightShader, params, dt);
            lightMesh.draw(lightShader);
            lightShader.unbind();

            // Render cloud
            cloudShader.bind();
            voronoiTex.bind(0);
            fbmTex.bind(1);

            cloudShader.setUniform("PV", pv);
            cloudShader.setUniform("cameraPos", camera.getPosition());
            cloudShader.setUniform("cameraFoV", camera.getFov());
            cloudShader.setUniform("voronoiTex", 0);
            cloudShader.setUniform("fbmTex", 1);
            setUniforms(cloudShader, params, dt);
            cubeMesh.draw(cloudShader);

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
