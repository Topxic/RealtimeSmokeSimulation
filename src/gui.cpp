#include "gui.h"
#include "window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

bool BaseGUI::initialized = false;

BaseGUI::BaseGUI()
{
    if (!initialized)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer backends
        auto window = Window::getInstance().getGLFWWindow();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");
        initialized = true;
    }
}

BaseGUI::~BaseGUI()
{
    if (initialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        initialized = false;
    }
}

void BaseGUI::render() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

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