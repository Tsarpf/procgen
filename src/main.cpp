#include <iostream>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#define GLM_FORCE_PURE
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
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (program);
    glBindVertexArray (vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void drawOctree()
{
    Octree* tree = new Octree(1, 8, vec3(0,0,0));
    printf("-------- visualizing octree --------\n");
    std::vector<VizData> viz = visualizeOctree(tree);
    //for(const auto& elem : viz)
    //{
    //    printf("min (%f, %f, %f), size %d", elem.min.x, elem.min.y, elem.min.z, elem.size);
    //    //std::cout << "min " << elem.min.x << " '" << " size " << elem.size << std::endl;
    //}
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
