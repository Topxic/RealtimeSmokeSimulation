#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <iostream>
#include "GL/glew.h"
#include "glm/ext.hpp"

class Shader
{
public:
    explicit Shader(const std::vector<std::string> &&files);

    /**
     * @brief Reload the shader program
     *
     * Reloads the shader source code and recompiles it.
     * If the reload fails, the old shader will be used and
     * an error message will be printed to the console.
     */
    void reload();

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

    /**
     * @brief Dispatch the compute shader
     *
     * Dispatch the compute shader with the given work group size.
     *
     */
    void dispatch(int xDispatches, int yDispatches);

    void setUniform(const std::string &name, int value) const;

    void setUniform(const std::string &name, float value) const;

    void setUniform(const std::string &name, bool value) const;

    void setUniform(const std::string &name, const glm::vec2 &value) const;

    void setUniform(const std::string &name, const glm::vec3 &value) const;

    void setUniform(const std::string &name, const glm::vec4 &value) const;

    void setUniform(const std::string &name, const glm::mat4 &value) const;

    template <typename T>
    static GLuint createBuffer(std::vector<T> data, unsigned int binding)
    {
        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_DYNAMIC_DRAW);
        std::cout << "Binded buffer of size " << data.size() * sizeof(T) << " to binding point " << binding << std::endl;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, bufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        return bufferID;
    }

    static void destroyBuffer(GLuint bufferID)
    {
        glDeleteBuffers(1, &bufferID);
    }

private:
    std::vector<std::string> files;
    GLuint programID;
    std::vector<GLuint> shaderIDs;

    void create();

    void destroy();
};

#endif // SHADER_H