#include "window.h"

#include <stdexcept>
#include <stdio.h>

Window::~Window()
{
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

const glm::vec2 Window::getResolution() const
{
    int width, height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    return glm::vec2(width, height);
}

GLFWwindow *Window::getGLFWWindow() const 
{
    return glfwWindow;
}

Window::Window(unsigned int width, unsigned int height)
{
    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        throw std::runtime_error("Could not start GLFW3");
    }

    glfwWindow = glfwCreateWindow(width, height, "Hello shader", NULL, NULL);
    if (!glfwWindow)
    {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        throw std::runtime_error("Could not open window with GLFW3");
    }
    glfwSetFramebufferSizeCallback(glfwWindow, framebufferSizeCallback);
    glfwMakeContextCurrent(glfwWindow);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}