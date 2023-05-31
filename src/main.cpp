#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <stdio.h>
#include <unistd.h>

#include "shader.hpp"
#include "mesh.hpp"

int main()
{
    // Change the current directory to the directory of the executable
    chdir("..");
    // print the current working directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working directory: %s\n", cwd);

    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return EXIT_FAILURE;
    }

    GLFWwindow *window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // start GLEW extension handler
    glewInit();

    // get version info
    const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte *version = glGetString(GL_VERSION);   // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS);    // depth-testing interprets a smaller value as "closer"

    std::vector<Vertex> vertices = {
        { glm::vec3(-0.5, -0.5, 0), glm::vec3(1, 0, 0) },
        { glm::vec3(0, 0.5, 0), glm::vec3(0, 1, 0) },
        { glm::vec3(0.5, -0.5, 0), glm::vec3(0, 0, 1) }
    };
    std::vector<unsigned int> indices = {
        0, 1, 2
    };

    Mesh mesh = Mesh(vertices, indices);
    Shader triangle = Shader("data/shader/triangle");

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        triangle.bind();
        triangle.setUniform("time", (float)glfwGetTime());
        mesh.draw();
        triangle.unbind();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // close GL context and any other GLFW resources
    glfwTerminate();
    return EXIT_SUCCESS;
}
