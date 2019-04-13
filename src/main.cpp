#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "noise.lib")

#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Scene.h"

int main(void)
{

	GLFWwindow *window = initialize();

	Scene scene(window);
	scene.Initialize();

	while (!glfwWindowShouldClose(window))
	{
		scene.Render();

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
