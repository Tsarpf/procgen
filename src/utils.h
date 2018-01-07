//
// Created by Teemu Sarapisto on 19/07/2017.
//

#ifndef PROJECT_UTILS_H
#define PROJECT_UTILS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "glm/glm.hpp"

GLFWwindow* initialize();
void stop(GLFWwindow* window);
GLuint createTriangleProgram();
GLuint createTriangleVAO();

#endif //PROJECT_UTILS_H
