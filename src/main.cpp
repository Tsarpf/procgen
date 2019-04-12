#include <iostream>
#include <stdlib.h>
#include <chrono>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

//#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "noise.lib")


#include "utils.h"
#include "Octree.h"
#include "OctreeMesh.h"

using namespace glm;

#define GLSL(src) #src


//TODO refactor to some scene manager thing mebbe
OctreeMesh* g_the_mesh_todo_refactor_to_somewhere;
std::vector<VizData> g_visualizationData;

static void space_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		printf("space pressed\n");
		//g_the_mesh_todo_refactor_to_somewhere->EnlargePlus(xplus);
		g_the_mesh_todo_refactor_to_somewhere->EnlargePlus(yplus);
		g_visualizationData = visualizeOctree(g_the_mesh_todo_refactor_to_somewhere->GetOctree());
	}
}


void drawVisualization(const int stride, const float time, const GLuint program, const GLuint vao, const GLuint vbo, const int elementCount, const std::vector<VizData>& nodes)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLint posAttrib = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(
		posAttrib, // attribute
		3,                 // number of elements per vertex, here (x,y,z)
		GL_FLOAT,          
		GL_FALSE,         
		stride * sizeof(float),          
		0               
	);

	GLint colorAttrib = glGetAttribLocation(program, "inColor");
	glEnableVertexAttribArray(colorAttrib);
	glVertexAttribPointer(
		colorAttrib, // attribute
		3,                 // number of elements per vertex, here (R,G,B)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		stride * sizeof(float),                 // no extra data between each position
		(void*)(3 * sizeof(float))                  // offset of first element
	);

	printf("draw visualization thing\n");
	GLint modelUniform = glGetUniformLocation(program, "Model");
	for (const auto& viz : nodes)
	{
		glm::mat4 translate = glm::translate
		(
			glm::mat4(1.0f),
			viz.min
		);

		glm::mat4 rotate = glm::mat4(1.0f);
		rotate = glm::rotate(
			rotate,
			time * glm::radians(60.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 scale = glm::mat4(1.0f);
		scale[0] = glm::vec4(viz.size, 0, 0, 0);
		scale[1] = glm::vec4(0, viz.size, 0, 0);
		scale[2] = glm::vec4(0, 0, viz.size, 0);

		//this should be correct but isn't?
		//glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model * translate * rotate * scale));

		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(rotate * translate * scale));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, elementCount);
	}
	glBindVertexArray(0);
}

int main(void)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	GLFWwindow *window = initialize();
	glfwSetKeyCallback(window, space_callback);

	GLuint triangleProgram = createTriangleProgram();

	const int octreeSize = 32;
	const int axisCount = 3;

	//std::vector<OctreeMesh*> meshes;
	//for (int x = 0; x < axisCount; x++)
	//{
	//	for (int z = 0; z < axisCount; z++)
	//	{
	//		OctreeMesh* mesh = new OctreeMesh(triangleProgram, octreeSize, vec3(x * octreeSize, 0, z * octreeSize));
	//		mesh->Load();
	//		meshes.push_back(mesh);
	//	}
	//}
	OctreeMesh* mesh = new OctreeMesh(triangleProgram, octreeSize, vec3(0, 0, 0));
	mesh->Load();

	g_the_mesh_todo_refactor_to_somewhere = mesh;

	// for running just a prebuilt cube without any DC
	//auto [vertexBuffer, indexBuffer] = indexedCubeTest(triangleProgram); 

	setupProjection(triangleProgram);

	// OpenGL flags, could be somewhere nicer.
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE); // sets cullingments

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // set to use wireframe
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

	//TODO: refactor to some sort of octree visualizer class
	g_visualizationData = visualizeOctree(mesh->GetOctree());
	std::vector<float> genericCubePoints = cubePoints();
	auto [vao, vbo] = createCubeVAO(genericCubePoints);
	while (!glfwWindowShouldClose(window))
	{
		glUseProgram(triangleProgram);
		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// visualize the octree
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // set to use wireframe
		drawVisualization(6, time, triangleProgram, vao, vbo, genericCubePoints.size(), g_visualizationData);
		
		//for (auto mesh : meshes)
		//{
		//	mesh->Draw(time);
		//}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // set to disable wireframe

		glEnable(GL_CULL_FACE);

		mesh->Draw(time);

		glfwPollEvents();
		glfwSwapBuffers(window);

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			printf("haz error %i \n", err);
		}
	}

	stop(window);
	exit(EXIT_SUCCESS);
}
