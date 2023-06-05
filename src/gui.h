#ifndef GUI_H
#define GUI_H

#include "glm/glm.hpp"
#include "shader.h"
#include <memory>

class GUI
{
public:
    GUI();

    ~GUI();

    /**
     * @brief Build the GUI
     * 
     * Build the ImGUI Frames in this function
    */
    void build();

    /**
     * @brief Render the GUI
     * 
     * Should be called in the rendering loop after 
     * calling build()
    */
    void render() const;

    /**
     * @brief Update shader
     * 
     * Update uniform values in the shader based
     * on the GUI values
    */
    void update(const std::unique_ptr<Shader> &shader);

private:
    bool show = false;

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

#endif // GUI_H