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

void MandelbulbGUI::build(float dt)
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

void MandelbulbGUI::setUniforms(const std::unique_ptr<Shader> &shader)
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

void SmokeGUI::build(float dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Smoke Parameters", &show);
    ImGui::Text("FPS: %.1f", 1 / dt);
    ImGui::Checkbox("Perform step", &performStep);
    ImGui::Checkbox("Show velocity field", &showVelocityField);
    ImGui::Checkbox("Show pressure field", &showPressureField);
    ImGui::Checkbox("Interpolate", &interpolate);
    ImGui::SliderFloat("Overrelaxation", &overrelaxation, 0, 4);
    ImGui::SliderInt("Incompressibility iterations", &iterations, 0, 200);
    ImGui::SliderFloat2("Gravity", &gravity[0], -10, 10);
    ImGui::SliderFloat("Density", &density, 500, 2000);
    ImGui::SliderFloat("Radius", &radius, 1.f, 10.f);
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
    shader->setUniform("radius", radius);
}