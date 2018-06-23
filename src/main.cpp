#include <iostream>
#include <stdlib.h>
#include <chrono>

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

void render(GLuint vao, int pointCount)
{
    // Draw wireframed
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao);

    //glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_TRIANGLES, 0, pointCount);
}

void drawOctree()
{
    Octree *tree = new Octree(1, 8, vec3(0, 0, 0));
    printf("-------- visualizing octree --------\n");
    std::vector<VizData> viz = visualizeOctree(tree);
    int count = 0;
    for (const auto &elem : viz)
    {
        if (elem.size == 1)
        {
            count++;
        }
        printf("hola from draw octree min (%f, %f, %f), size %d\n", elem.min.x, elem.min.y, elem.min.z, elem.size);
        //std::cout << "min " << elem.min.x << " '" << " size " << elem.size << std::endl;
    }
    printf("number of elements %i\n", count);
}

int main(void)
{
    auto t_start = std::chrono::high_resolution_clock::now();

    GLFWwindow *window = initialize();

    // glEnable(GL_DEPTH_TEST); // enable depth-testing
    // glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    //GLuint triangleVAO = createTriangleVAO();
    std::vector<float> genericCubePoints = cubePoints();
    GLuint triangleVAO = createCubeVAO(genericCubePoints);
    GLuint triangleProgram = createTriangleProgram();

    setAttribPointers(triangleProgram);

    module::Perlin myModule;
    double value = myModule.GetValue(1.25, 0.75, 0.5);
    std::cout << value << std::endl;

    //drawOctree();

    setupProjection(triangleProgram);
    while (!glfwWindowShouldClose(window))
    {
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        render(triangleVAO, genericCubePoints.size());

        glfwPollEvents();
        glfwSwapBuffers(window);

        GLenum err;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            printf("haz error %i \n", err);
        }
    }

    stop(window);
    exit(EXIT_SUCCESS);
}
