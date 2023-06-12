#ifndef CONTROLS_H
#define CONTROLS_H

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "window.h"
#include <memory>

// Abstract control class. Should not be used directly.
class BaseControls
{
public:
    static bool reload;

    /**
     * @brief Enable or disable the controls.
     *
     *  If the controls are disabled, the camera will not be updated based on mouse and keyboard inputs. 
     *
     * @param enabled Whether the controls should be enabled or not.
     */
    void setEnabled(bool enabled);

    const bool getEnabled() const;

    glm::vec3 getCameraPosition() const;

    glm::vec3 getCameraDirection() const;

    glm::vec3 getCameraUp() const;

    glm::mat4 getViewMatrix() const;

    /**
     * @brief Update the camera position and direction based on mouse and keyboard inputs.
     * 
     * This function should be called every frame. 
     * It will update the camera position and direction based on mouse and keyboard inputs.
     * Each control class implements this function differently.
     *
     * @param deltaTime The time since the last update.
     */
    virtual void update(float deltaTime) = 0;

protected:
    static double lastX;
    static double lastY;
    static double scrollX;
    static double scrollY;
    static bool leftMousePressed;
    static bool rightMousePressed;
    static bool forwardPressed;
    static bool backwardPressed;
    static bool leftPressed;
    static bool rightPressed;
    static bool enabled;

    static glm::vec3 cameraUp;
    static glm::vec3 cameraPosition;
    static glm::vec3 cameraDirection;

    // Hide constructor
    BaseControls();

private:
    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};


class OrbitControls : public BaseControls
{
public:
    void update(float deltaTime);

    void setRotationSpeed(float speed);

    void setZoomSpeed(float speed);

    void setRadiusLimits(float minRadius, float maxRadius);

private:
    float radius = 1.5f;
    float theta = 0.f;
    float phi = 1.5f;

    float rotationSpeed = 0.1f;
    float zoomSpeed = 0.01f;
    float minRadius = 1.0f;
    float maxRadius = 10.0f;

    void updateCamera();
};


class FreeControls : public BaseControls
{
public:
    void update(float deltaTime);

    void setMovementSpeed(float speed);

    void setMouseSpeed(float speed);

private:
    float movementSpeed = 0.1f;
    float mouseSpeed = 0.01f;
};

BaseControls *getControls();

#endif // CONTROLS_H
