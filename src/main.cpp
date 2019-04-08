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

void render(GLuint vao, int pointCount)
{
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, pointCount);
	glBindVertexArray(0);
}
void renderIndexed(int pointCount)
{
	printf("unsigned int %i, GL_UNSIGNED_SHORT \n", sizeof(unsigned int), sizeof(GL_UNSIGNED_SHORT));
	glDrawElements(GL_TRIANGLES, pointCount, GL_UNSIGNED_SHORT, 0);
}

std::vector<VizData> drawOctree(Octree* tree)
{
	printf("-------- visualizing octree --------\n");
	std::vector<VizData> viz = visualizeOctree(tree);
	return viz;
}

void initializeOpenGL(GLuint program)
{
	setupProjection(program);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BACK);

	// Draw wireframed
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void drawVisualization(const float time, const GLuint program, const GLuint vao, const int elementCount, const std::vector<VizData>& nodes)
{
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
			time * glm::radians(30.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 scale = glm::mat4(1.0f);
		scale[0] = glm::vec4(viz.size, 0, 0, 0);
		scale[1] = glm::vec4(0, viz.size, 0, 0);
		scale[2] = glm::vec4(0, 0, viz.size, 0);

		glm::mat4 model = glm::mat4(1.0f);
		//this should be correct but isn't?
		//glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model * translate * rotate * scale));

		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model * rotate * translate * scale));
		render(vao, elementCount);
	}
}

void drawMesh(GLuint program, GLuint gl_vertexBuffer, GLuint gl_indexBuffer, GLuint gl_vao, const int indexCount, const float time)
{

}

int main(void)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	GLFWwindow *window = initialize();
	GLuint triangleProgram = createTriangleProgram();

	const int octreeSize = 32;
	const int axisCount = 5;

	std::vector<OctreeMesh*> meshes;
	for (int x = 0; x < axisCount; x++)
	{
		for (int z = 0; z < axisCount; z++)
		{
			OctreeMesh* mesh = new OctreeMesh(triangleProgram, octreeSize, vec3(x * octreeSize, 0, z * octreeSize));
			mesh->Load();
			meshes.push_back(mesh);
		}
	}

	//std::vector<VizData> visualizationData = drawOctree(tree);
	// for running just a prebuilt cube without any DC
	//auto [vertexBuffer, indexBuffer] = indexedCubeTest(triangleProgram); 

	setupProjection(triangleProgram);

	// OpenGL flags, could be somewhere nicer.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); // sets cullingments
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // set to use wireframe
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

	while (!glfwWindowShouldClose(window))
	{
		glUseProgram(triangleProgram);
		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// visualize the octree
		//drawVisualization(time, triangleProgram, triangleVAO, genericCubePoints.size(), visualizationData);
		
		for (auto mesh : meshes)
		{
			mesh->Draw(time);
		}

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
