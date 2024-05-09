#pragma comment(lib, "OpenGL32.lib")

#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glu.h> // Include this at the top of your file
#include <GLFW/glfw3.h>

#include "Scene.h"

int main(void)
{
	GLFWwindow *window = initialize();

	Scene scene(window);
	scene.Initialize();

	while (!glfwWindowShouldClose(window))
	{
		scene.Update();
		scene.Render();

		glfwPollEvents();
		glfwSwapBuffers(window);

		GLenum err;

		// ...

		while ((err = glGetError()) != GL_NO_ERROR)
		{
			const GLubyte *errString = gluErrorString(err);
			if (errString)
			{
				printf("OpenGL error: %s\n", errString);
			}
			else
			{
				printf("Unknown OpenGL error with code: %i\n", err);
			}
		}

	}

	stop(window);
	exit(EXIT_SUCCESS);
}
