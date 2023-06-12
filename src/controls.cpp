#include "controls.h"
#include "glm/gtc/matrix_transform.hpp"

double BaseControls::lastX = 0.0;
double BaseControls::lastY = 0.0;
double BaseControls::scrollX = 0.0;
double BaseControls::scrollY = 0.0;
bool BaseControls::leftMousePressed = false;
bool BaseControls::rightMousePressed = false;
bool BaseControls::forwardPressed = false;
bool BaseControls::backwardPressed = false;
bool BaseControls::leftPressed = false;
bool BaseControls::rightPressed = false;
bool BaseControls::enabled = true;
bool BaseControls::reload = false;

glm::vec3 BaseControls::cameraUp = glm::vec3(0, 1, 0);
glm::vec3 BaseControls::cameraPosition;
glm::vec3 BaseControls::cameraDirection;

static std::unique_ptr<OrbitControls> orbitControls = std::make_unique<OrbitControls>();
static std::unique_ptr<FreeControls> freeControls = std::make_unique<FreeControls>();
static BaseControls *activeControls = orbitControls.get();

BaseControls *getControls()
{
    return activeControls;
}

BaseControls::BaseControls()
{
    auto glfwWindow = Window::getInstance().getGLFWWindow();
    glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
    glfwSetKeyCallback(glfwWindow, keyCallback);
    glfwSetScrollCallback(glfwWindow, scrollCallback);
}

void BaseControls::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    BaseControls *controls = static_cast<BaseControls *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        controls->leftMousePressed = (action == GLFW_PRESS);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        controls->rightMousePressed = (action == GLFW_PRESS);
    }
}

void BaseControls::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    BaseControls *controls = static_cast<BaseControls *>(glfwGetWindowUserPointer(window));
    controls->scrollX = xoffset;
    controls->scrollY = yoffset;
}

void BaseControls::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    BaseControls *controls = static_cast<BaseControls *>(glfwGetWindowUserPointer(window));
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
        {
            controls->setEnabled(!controls->getEnabled());
            if (controls->getEnabled())
            {
                glfwSetInputMode(Window::getInstance().getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else
            {
                glfwSetInputMode(Window::getInstance().getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        break;
    case GLFW_KEY_1:
        if (action == GLFW_PRESS)
        {
            activeControls = orbitControls.get();
        }
        break;
    case GLFW_KEY_2:
        if (action == GLFW_PRESS)
        {
            activeControls = freeControls.get();
        }
        break;
    // Movement control keys
    case GLFW_KEY_W:
        if (action == GLFW_PRESS)
        {
            controls->forwardPressed = true;
        }
        if (action == GLFW_RELEASE)
        {
            controls->forwardPressed = false;
        }
        break;
    case GLFW_KEY_A:
        if (action == GLFW_PRESS)
        {
            controls->leftPressed = true;
        }
        if (action == GLFW_RELEASE)
        {
            controls->leftPressed = false;
        }
        break;
    case GLFW_KEY_D:
        if (action == GLFW_PRESS)
        {
            controls->rightPressed = true;
        }
        if (action == GLFW_RELEASE)
        {
            controls->rightPressed = false;
        }
        break;
    case GLFW_KEY_S:
        if (action == GLFW_PRESS)
        {
            controls->backwardPressed = true;
        }
        if (action == GLFW_RELEASE)
        {
            controls->backwardPressed = false;
        }
        break;
    case GLFW_KEY_R:
        reload = true;
        break;
    default:
        break;
    }
}

void BaseControls::setEnabled(bool enabled)
{
    this->enabled = enabled;
}

const bool BaseControls::getEnabled() const
{
    return enabled;
}

glm::vec3 BaseControls::getCameraPosition() const
{
    return cameraPosition;
}

glm::vec3 BaseControls::getCameraDirection() const
{
    return cameraDirection;
}

glm::vec3 BaseControls::getCameraUp() const
{
    return cameraUp;
}

glm::mat4 BaseControls::getViewMatrix() const
{
    return glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);
}

void OrbitControls::update(float deltaTime)
{
    auto glfwWindow = Window::getInstance().getGLFWWindow();
    double mouseX, mouseY;
    glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);
    if (enabled)
    {
        float deltaX = static_cast<float>(mouseX - lastX);
        float deltaY = static_cast<float>(mouseY - lastY);

        theta += glm::radians(deltaX * rotationSpeed);
        phi -= glm::radians(2 * deltaY * rotationSpeed);

        radius += scrollY * zoomSpeed;
        radius = glm::clamp(radius, minRadius, maxRadius);
        scrollY = 0.f;
    }
    lastX = mouseX;
    lastY = mouseY;
    updateCamera();
}

void OrbitControls::setRotationSpeed(float speed)
{
    rotationSpeed = speed;
}

void OrbitControls::setZoomSpeed(float speed)
{
    zoomSpeed = speed;
}

void OrbitControls::setRadiusLimits(float minRadius, float maxRadius)
{
    minRadius = minRadius;
    maxRadius = maxRadius;
    radius = glm::clamp(radius, minRadius, maxRadius);
}

void OrbitControls::updateCamera()
{
    // Clamp up and down vector to prevent camera orientation flipping
    phi = glm::clamp(phi, 0.1f, M_PIf - 0.1f);
    float x = radius * glm::sin(phi) * glm::cos(theta);
    float y = radius * glm::cos(phi);
    float z = radius * glm::sin(phi) * glm::sin(theta);
    cameraPosition = glm::vec3(x, y, z);
    cameraDirection = glm::normalize(-cameraPosition);
}

void FreeControls::update(float deltaTime)
{
    auto glfwWindow = Window::getInstance().getGLFWWindow();
    glm::vec3 right = glm::cross(cameraDirection, cameraUp);

    // Update camera direction
    double mouseX, mouseY;
    glfwGetCursorPos(glfwWindow, &mouseX, &mouseY);
    if (enabled)
    {
        float deltaX = mouseSpeed * static_cast<float>(mouseX - lastX);
        float deltaY = mouseSpeed * static_cast<float>(mouseY - lastY);
        cameraDirection = glm::normalize(cameraDirection + deltaX * right - deltaY * cameraUp);
    }
    lastX = mouseX;
    lastY = mouseY;

    // Update camera position
    glm::cross(cameraDirection, cameraUp);
    glm::vec3 dir = glm::vec3(0);
    if (forwardPressed)
    {
        dir += cameraDirection;
    }
    if (backwardPressed)
    {
        dir -= cameraDirection;
    }
    if (leftPressed)
    {
        dir -= right;
    }
    if (rightPressed)
    {
        dir += right;
    }
    if (glm::length(dir) > 0)
    {
        dir = glm::normalize(dir);
    }
    cameraPosition += movementSpeed * deltaTime * dir;
}

void FreeControls::setMovementSpeed(float speed)
{
    movementSpeed = speed;
}

void FreeControls::setMouseSpeed(float speed)
{
    mouseSpeed = speed;
}
