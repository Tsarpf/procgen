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

Octree* GetOctreeDrawData(VertexBuffer& vBuffer, IndexBuffer& iBuffer, int size, const vec3& position)
{
	Octree* tree = new Octree(1, size, position);

	printf("--------- Octree initialized, meshing ---------------\n");
	tree->MeshFromOctree(iBuffer, vBuffer);
	return tree;
}

void drawMesh(GLuint program, GLuint gl_vertexBuffer, GLuint gl_indexBuffer, GLuint gl_vao, const int indexCount, const float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model[0] = glm::vec4(1, 0, 0, 0);
	model[1] = glm::vec4(0, 1, 0, 0);
	model[2] = glm::vec4(0, 0, 1, 0);
	model[3] = glm::vec4(0, 0, 0, 1);

	model = glm::translate(model, vec3(-5, -5, -5));

	GLint modelUniform = glGetUniformLocation(program, "Model");
	glm::mat4 rotate = glm::rotate(
		model,
		time * glm::radians(60.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(rotate));

	// draw it
	bindBuffers(program, gl_vertexBuffer, gl_indexBuffer, gl_vao, sizeof(Vertex));
	renderIndexed(indexCount);

}

int main(void)
{
	auto t_start = std::chrono::high_resolution_clock::now();

	GLFWwindow *window = initialize();
	GLuint triangleProgram = createTriangleProgram();

	const int octreeSize = 32;

	VertexBuffer vBuffer;
	IndexBuffer iBuffer;
	Octree* tree = GetOctreeDrawData(vBuffer, iBuffer, octreeSize, vec3(0, 0, 0));
	auto[gl_vertexBuffer, gl_indexBuffer, gl_vao] = indexedBufferSetup(vBuffer, iBuffer);

	VertexBuffer vBuffer_2;
	IndexBuffer iBuffer_2;
	Octree* tree_2 = GetOctreeDrawData(vBuffer_2, iBuffer_2, octreeSize, vec3(0, octreeSize, 0));
	auto[gl_vertexBuffer_2, gl_indexBuffer_2, gl_vao_2] = indexedBufferSetup(vBuffer_2, iBuffer_2);

	// doesn't work with these commented out lol
	 //std::vector<float> genericCubePoints = cubePoints();
	 //GLuint triangleVAO = createCubeVAO(genericCubePoints);

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
		
		// draw the actual dual contoured mesh
		drawMesh(triangleProgram, gl_vertexBuffer, gl_indexBuffer, gl_vao, iBuffer.size(), time);
		drawMesh(triangleProgram, gl_vertexBuffer_2, gl_indexBuffer_2, gl_vao_2, iBuffer_2.size(), time);


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
