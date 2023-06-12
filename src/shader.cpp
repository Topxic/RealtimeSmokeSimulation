#include "shader.h"

#include <iostream>
#include <fstream>
#include <vector>

static GLuint createShaderProgram(const std::vector<GLuint> &shaderIDs)
{
    GLuint program = glCreateProgram();
    for (const GLuint shaderID : shaderIDs)
    {
        glAttachShader(program, shaderID);
    }
    glLinkProgram(program);

    // Check linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Failed to link shader program: " << infoLog << std::endl;
    }

    return program;
}

static std::string readFile(const std::string &file)
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

    for (size_t pos = content.find("#include"); pos != std::string::npos; pos = content.find("#include"))
    {
        size_t start = content.find("\"", pos);
        size_t end = content.find("\"", start + 1);
        std::string includeFile = content.substr(start + 1, end - start - 1);
        content.replace(pos, end - pos + 1, readFile(includeFile));
    }

    return content;
}

static GLuint compileShader(GLenum shaderType, const char *shaderCode)
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
        std::cerr << "Failed to compile shader: " << infoLog << std::endl;
    }

    return shader;
}

Shader::Shader(const std::vector<std::string> &&files) : files(std::move(files))
{
    create();
}

Shader::~Shader()
{
    destroy();
}

void Shader::reload()
{
    destroy();
    create();
}

void Shader::create()
{
    for (const auto &file : files)
    {
        // Read the shader source code from file
        std::string code = readFile(file);

        // Compile shaders
        std::cout << "Compiling shader: " << file << std::endl;
        if (file.find(".vert") != std::string::npos)
        {
            GLuint shaderID = compileShader(GL_VERTEX_SHADER, code.c_str());
            shaderIDs.push_back(shaderID);
        }
        else if (file.find(".frag") != std::string::npos)
        {
            GLuint shaderID = compileShader(GL_FRAGMENT_SHADER, code.c_str());
            shaderIDs.push_back(shaderID);
        }
        else if (file.find(".comp") != std::string::npos)
        {
            GLuint shaderID = compileShader(GL_COMPUTE_SHADER, code.c_str());
            shaderIDs.push_back(shaderID);
        }
        else
        {
            std::cerr << "Shader type not supported" << std::endl;
        }
    }

    // Create shader program and link shaders
    programID = createShaderProgram(shaderIDs);
}

void Shader::destroy()
{
    for (const GLuint shaderID : shaderIDs)
    {
        glDetachShader(programID, shaderID);
        glDeleteShader(shaderID);
    }
    glDeleteProgram(programID);
}

void Shader::bind()
{
    glUseProgram(programID);
}

void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::setUniform(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setUniform(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setUniform(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setUniform(const std::string &name, const glm::vec2 &value) const
{
    glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::vec4 &value) const
{
    glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setUniform(const std::string &name, const glm::mat4 &value) const
{
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::dispatch(int xDispatches, int yDispatches)
{
    glUseProgram(programID);
    glDispatchCompute(xDispatches, yDispatches, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
