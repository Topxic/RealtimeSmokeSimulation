#ifndef MESH_H
#define MESH_H

#include <vector>
#include "GL/glew.h"
#include "glm/glm.hpp"

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 textureCoord;
};

class Mesh
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

    ~Mesh();

    /**
     * @brief Draw the mesh
     * 
     * This function should be called in the rendering loop.
     * It will draw the mesh using the current binded shader.
    */
    void draw();

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

#endif // MESH_H