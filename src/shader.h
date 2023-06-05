#ifndef SHADER_H
#define SHADER_H

#include <string>
#include "GL/glew.h"
#include "glm/ext.hpp"

class Shader
{
public:
    explicit Shader(const std::string &name);

    /**
     * @brief Reload the shader
     * 
     * Reloads the shader source code and recompiles it. 
     * If the reload fails, the old shader will be used and
     * an error message will be printed to the console.
    */
    void reload();

    /**
     * @brief Create the shader
     * 
     * Creates the shader from the source code.
    */
    void create();

    ~Shader();

    /**
     * @brief Bind the shader
     * 
     * Bind the shader to the current OpenGL context.
     * Every draw call or uniform update after this function will use this shader.
    */
    void bind();

    /**
     * @brief Unbind the shader
     * 
     * Unbind the shader from the current OpenGL context.
    */
    void unbind();

    void setUniform(const std::string &name, int value) const;

    void setUniform(const std::string &name, float value) const;

    void setUniform(const std::string &name, bool value) const;

    void setUniform(const std::string &name, const glm::vec2 &value) const;

    void setUniform(const std::string &name, const glm::vec3 &value) const;

    void setUniform(const std::string &name, const glm::vec4 &value) const;

    void setUniform(const std::string &name, const glm::mat4 &value) const;

private:
    std::string name;
    GLuint ID;
};

#endif // SHADER_H