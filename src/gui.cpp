#include "gui.h"
#include "window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

GUI::GUI()
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
}

GUI::~GUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::build()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Rendering Options", &show);
    ImGui::Checkbox("Anti Aliasing", &antiAliasing);
    ImGui::SliderInt("Maximal raymarching steps", &maximalSteps, 1, 300);
    ImGui::InputFloat("Maximal raymarching distance", &maximalDistance);
    ImGui::SliderFloat3("Light direction", &lightDir[0], -1, 1);
    ImGui::End();
    ImGui::Begin("Fractal Options", &show);
    ImGui::SliderInt("Iterations", &iterations, 1, 300);
    ImGui::SliderFloat("Bailout", &bailout, .95f, 5.f);
    ImGui::SliderFloat("Power", &power, 1.f, 10.f, "%.6f");
    ImGui::End();
}

void GUI::render() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::update(const std::unique_ptr<Shader> &shader)
{
    shader->setUniform("antiAliasing", antiAliasing);
    shader->setUniform("iterations", iterations);
    shader->setUniform("bailout", bailout);
    shader->setUniform("power", power);
    shader->setUniform("maximalSteps", maximalSteps);
    shader->setUniform("maximalDistance", maximalDistance);
    lightDir = glm::normalize(lightDir);
    shader->setUniform("lightDir", lightDir);
}