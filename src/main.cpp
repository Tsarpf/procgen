#include <iostream>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "noise.lib")

#include <noise/noise.h>

#include "utils.h"
#include "Octree.h"

using namespace noise;
using namespace glm;

#define GLSL(src) #src

void render(GLuint vao, GLuint program)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (program);
    glBindVertexArray (vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void drawOctree()
{
    // abua
    Octree* tree = new Octree(1, 8, glm::vec3(0,0,0));
}

int main(void) {
    GLFWwindow* window = initialize();

    //glEnable(GL_DEPTH_TEST); // enable depth-testing
    //glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    GLuint triangleVAO = createTriangleVAO();
    GLuint triangleProgram = createTriangleProgram();

	module::Perlin myModule;
	double value = myModule.GetValue(1.25, 0.75, 0.5);
	std::cout << value << std::endl;

    drawOctree();

    while (!glfwWindowShouldClose(window)) {

        render(triangleVAO, triangleProgram);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    stop(window);
    exit(EXIT_SUCCESS);
}
