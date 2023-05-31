#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include "glm/ext.hpp"

class Shader
{
public:
    explicit Shader(const std::string &name)
    {
        // Read the shader source code from file
        std::string vertexCode = readFile(name + ".vert");
        std::string fragmentCode = readFile(name + ".frag");

        // Compile shaders
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str());
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

        // Create shader program and link shaders
        ID = createShaderProgram(vertexShader, fragmentShader);

        // Delete the individual shaders as they're no longer needed
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    ~Shader()
    {
        glDeleteProgram(ID);
    }

    // Use the shader program
    void bind()
    {
        glUseProgram(ID);
    }

    // Stop using the shader program
    void unbind()
    {
        glUseProgram(0);
    }

    void setUniform(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setUniform(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setUniform(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setUniform(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void setUniform(const std::string &name, const glm::vec4 &value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

private:
    GLuint ID;

    std::string readFile(const std::string &file)
    {
        std::ifstream fileStream(file);
        if (!fileStream.is_open())
        {
            throw std::runtime_error("Failed to open file: " + file);
        }

        std::string content(
            (std::istreambuf_iterator<char>(fileStream)),
            (std::istreambuf_iterator<char>()));
        fileStream.close();

        return content;
    }

    GLuint compileShader(GLenum shaderType, const char *shaderCode)
    {
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderCode, NULL);
        glCompileShader(shader);

        // Check compilation errors
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "Failed to compile shader: " << infoLog << std::endl;
        }

        return shader;
    }

    // Create shader program and link shaders
    GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader)
    {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Check linking errors
        int success;
        char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "Failed to link shader program: " << infoLog << std::endl;
        }

        return program;
    }
};

#endif // SHADER_H