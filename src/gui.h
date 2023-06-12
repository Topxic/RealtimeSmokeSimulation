#ifndef GUI_H
#define GUI_H

#include "glm/glm.hpp"
#include "shader.h"
#include <memory>

class BaseGUI
{
public:
    BaseGUI();

    ~BaseGUI();

    /**
     * @brief Render the GUI
     * 
     * Should be called in the rendering loop after 
     * calling build()
    */
    virtual void render() const;

    /**
     * @brief Build the GUI
     * 
     * Build the ImGUI Frames in this function
    */
    virtual void build(float dt) = 0;

    /**
     * @brief Set uniforms
     * 
     * Update uniform values in the shader based
     * on the GUI values
    */
    virtual void setUniforms(const std::unique_ptr<Shader> &shader) = 0;

protected:
    bool show = false;

private:
    static bool initialized;
};


class MandelbulbGUI : public BaseGUI
{
public:
    void build(float dt);

    void setUniforms(const std::unique_ptr<Shader> &shader);

private:
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
};


class SmokeGUI : public BaseGUI
{
public:
    int iterations = 30;
    bool performStep = true;

    void build(float dt);

    void setUniforms(const std::unique_ptr<Shader> &shader);

private:
    float overrelaxation = 1.9;
    glm::vec2 gravity = glm::vec2(9.81, 0);
    float density = 1000.f;
    bool showVelocityField = false;
    bool showPressureField = false;
    bool interpolate = true;
    float radius = 3.f;
};

#endif // GUI_H