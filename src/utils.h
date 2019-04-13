//
// Created by Teemu Sarapisto on 19/07/2017.
//

#ifndef PROJECT_UTILS_H
#define PROJECT_UTILS_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "glm/glm.hpp"

#include "Octree.h"
#include "OctreeMesh.h"


GLFWwindow* initialize();
void stop(GLFWwindow* window);
GLuint createTriangleProgram();
GLuint createTriangleVAO();
std::tuple<GLuint, GLuint>  createCubeVAO(std::vector<float>& points);
GLuint createIndexVAO(const VertexBuffer& vb, const IndexBuffer& ib);
void setupProjection(GLuint program);
std::tuple<GLuint, GLuint> indexedCubeTest(GLuint program);
std::tuple<GLuint, GLuint, GLuint> indexedBufferSetup(const VertexBuffer& verts, const IndexBuffer& inds);
void bindBuffers(GLuint program, GLuint vertex, GLuint indices, GLuint vao, int stride);
std::vector<float> cubePoints();

void printBinary(uint8_t field);


#endif //PROJECT_UTILS_H
