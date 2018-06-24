#include <iostream>
#include <stdlib.h>
#include <chrono>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, pointCount);
}

std::vector<VizData> drawOctree(int size)
{
    Octree *tree = new Octree(1, size, vec3(0, 0, 0));
    printf("-------- visualizing octree --------\n");
    std::vector<VizData> viz = visualizeOctree(tree);
    int count = 0;
    for (const auto &elem : viz)
    {
        if (elem.size == 4)
        {
            count++;
            printf("hola from draw octree min (%f, %f, %f), size %d\n", elem.min.x, elem.min.y, elem.min.z, elem.size);
        }
        //std::cout << "min " << elem.min.x << " '" << " size " << elem.size << std::endl;
    }
    printf("number of elements %i\n", count);
    return viz;
}

void initializeOpenGL(GLuint program)
{
    setAttribPointers(program);
    setupProjection(program);
    glEnable(GL_CULL_FACE);

    // Draw wireframed
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void rotateModel(float time, GLuint program)
{
    GLint modelUniform = glGetUniformLocation(program, "Model");

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(
        model,
        time * glm::radians(180.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));
}

void drawVisualization(const float time, const GLuint program, const GLuint triangleVAO, const std::vector<float>& genericCubePoints,
                       const std::vector<VizData>& nodes)
{
    GLint modelUniform = glGetUniformLocation(program, "Model");
    for(const auto& viz : nodes)
    {
        glm::mat4 translate = glm::translate
        (
            glm::mat4(),
            viz.min
        );

        glm::mat4 rotate = glm::mat4(1.0f);
        rotate = glm::rotate(
            rotate,
            time * glm::radians(30.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 scale = glm::mat4(1.0f);
        scale[0] = glm::vec4(viz.size, 0, 0, 0);
        scale[1] = glm::vec4(0, viz.size, 0, 0);
        scale[2] = glm::vec4(0, 0, viz.size, 0);

        glm::mat4 model = glm::mat4(1.0f);
        //glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(translate * scale * rotate * model ));

        //this should be correct but isn't?
        //glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(translate * rotate * scale * model));

        glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model * rotate * translate * scale));
        //glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model * (translate * (rotate * scale))));
        render(triangleVAO, genericCubePoints.size());
    }
}

int main(void)
{
    auto t_start = std::chrono::high_resolution_clock::now();

    GLFWwindow *window = initialize();

     glEnable(GL_DEPTH_TEST); // enable depth-testing
    // glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    //GLuint triangleVAO = createTriangleVAO();
    std::vector<float> genericCubePoints = cubePoints();
    GLuint triangleVAO = createCubeVAO(genericCubePoints);
    GLuint triangleProgram = createTriangleProgram();


    module::Perlin myModule;
    double value = myModule.GetValue(1.25, 0.75, 0.5);
    std::cout << value << std::endl;

    std::vector<VizData> visualizationData = drawOctree(8);

    initializeOpenGL(triangleProgram);
    while (!glfwWindowShouldClose(window))
    {
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawVisualization(time, triangleProgram, triangleVAO, genericCubePoints, visualizationData);

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
