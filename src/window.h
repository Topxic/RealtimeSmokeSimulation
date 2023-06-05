#ifndef WINDOW_H
#define WINDOW_H

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

class Window
{
public:
    static Window& getInstance()
    {
        static Window instance(800, 600);
        return instance;
    }

    ~Window();

    const glm::vec2 getResolution() const;

    GLFWwindow *getGLFWWindow() const;

    // Prevent copying
    Window(const Window&) = delete;
    void operator=(const Window&) = delete;

private:
    Window(unsigned int width, unsigned int height);

    GLFWwindow* glfwWindow;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

#endif // WINDOW_H