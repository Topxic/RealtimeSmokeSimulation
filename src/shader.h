#ifndef SHADER_H
#define SHADER_H

#include <string>
#include "GL/glew.h"
#include "glm/ext.hpp"

class Shader
{
public:
    explicit Shader(const std::string &name);

    void reload();

    void create();

    ~Shader();

    // Use the shader program
    void bind();

    // Stop using the shader program
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