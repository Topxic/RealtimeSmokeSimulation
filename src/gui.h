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

class SmokeGUI : public BaseGUI
{
public:
    int iterations = 30;
    bool performStep = true;
    bool reset = false;

    void build(float dt);

    void setUniforms(const std::unique_ptr<Shader> &shader);

private:
    float overrelaxation = 1.9;
    glm::vec2 gravity = glm::vec2(9.81, 0);
    float density = 0.006f;
    bool showVelocityField = false;
    bool showPressureField = false;
    bool interpolate = true;
};

#endif // GUI_H