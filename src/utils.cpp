//
// Created by Teemu Sarapisto on 19/07/2017.
//

#include "utils.h"

#include <string>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

static void exit_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void error_callback(int error, const char *description) {
	fputs(description, stderr);
}

void printBinary(uint8_t field)
{
	std::cout << "printing field" << std::endl;
	std::string output = "";
	for (int i = 7; i >= 0; i--)
	{
		((field & (1 << i)) > 0)
			? output += "1"
			: output += "0";
	}
	std::cout << output << std::endl;
}

GLFWwindow* initialize()
{
	GLFWwindow *window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(1280, 768, "Dual contouring methods", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glewExperimental = GL_TRUE;
	glfwMakeContextCurrent(window);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(1);
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);


	glfwSetKeyCallback(window, exit_callback);
	return window;
}

// For testing simplest possible setup.
std::tuple<GLuint, GLuint> indexedCubeTest(GLuint program)
{
	GLuint vbo_cube_vertices;
	GLuint ibo_cube_elements;

	GLfloat cube_vertices[] = {
		// front		   // front colors
		-1.0, -1.0,  1.0,  1.0, 0.0, 0.0,
		 1.0, -1.0,  1.0,  0.0, 1.0, 0.0,
		 1.0,  1.0,  1.0,  0.0, 0.0, 1.0,
		-1.0,  1.0,  1.0,  1.0, 1.0, 1.0,
		// back            // back colors
		-1.0, -1.0, -1.0,  1.0, 0.0, 0.0,
		 1.0, -1.0, -1.0,  0.0, 1.0, 0.0,
		 1.0,  1.0, -1.0,  0.0, 0.0, 1.0,
		-1.0,  1.0, -1.0,  1.0, 1.0, 1.0,
	};

	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 0, 3,
		3, 7, 4,
		// left
		4, 5, 1,
		1, 0, 4,
		// right
		3, 2, 6,
		6, 7, 3,
	};
	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);


	int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	printf("size 1 %i\n", size);
	printf("size 2 %f\n", size / sizeof(GLushort));

	//glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_BACK);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

	//glViewport(0, 0, 1280, 768);

	//int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	//int count = size / sizeof(GLushort);
	//printf("count %i \n", count);
	//glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0);

	return std::tuple(vbo_cube_vertices, ibo_cube_elements);
}

void stop(GLFWwindow* window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

static std::string readFile(const char* path)
{
	std::ifstream stream(path);
	if (!stream.is_open()) {
		throw std::runtime_error("File not found!");
	}

	std::string content((std::istreambuf_iterator<char>(stream)), (std::istreambuf_iterator<char>()));
	return content;
}

void printShaderInfoLog(GLuint shader) {
	int max_length = 4096;
	int actual_length = 0;
	char slog[4096];
	glGetShaderInfoLog(shader, max_length, &actual_length, slog);
	fprintf(stderr, "shader info log for GL index %u\n%s\n", shader, slog);
}

void printProgramInfoLog(GLuint program) {
	int max_length = 4096;
	int actual_length = 0;
	char plog[4096];
	glGetProgramInfoLog(program, max_length, &actual_length, plog);
	fprintf(stderr, "program info log for GL index %u\n%s\n", program, plog);
}

bool checkShaderErrors(GLuint shader)
{
	GLint success = -1;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE)
	{
		printShaderInfoLog(shader);
		throw std::runtime_error("Shader compilation error!");
	}
	return true;
}

bool checkProgramErrors(GLuint program) {
	GLint success = -1;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success != GL_TRUE)
	{
		printProgramInfoLog(program);
		throw std::runtime_error("Program link error!");
	}
	return true;
}

GLuint createTriangleProgram()
{
	GLuint program = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef _WIN32
	printf("platform win32\n");
	std::string vertStr = readFile("../src/simple.vert");
	const char* vertStrC = vertStr.c_str();
	std::string fragStr = readFile("../src/simple.frag");
	const char* fragStrC = fragStr.c_str();
#else
	printf("platform something not-win32\n");
	std::string vertStr = readFile("../src/simple.vert");
	const char* vertStrC = vertStr.c_str();
	std::string fragStr = readFile("../src/simple.frag");
	const char* fragStrC = fragStr.c_str();
#endif

	// Vertex shader
	// The 1 and NULL stand for 1 string element that is NULL terminated. What an ugly api.
	glShaderSource(vertexShader, 1, &vertStrC, NULL);
	glCompileShader(vertexShader);
	checkShaderErrors(vertexShader);
	glAttachShader(program, vertexShader);

	// Pikkels shader
	glShaderSource(fragmentShader, 1, &fragStrC, NULL);
	glCompileShader(fragmentShader);
	checkShaderErrors(fragmentShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);
	checkProgramErrors(program);

	glUseProgram(program);

	return program;
}

GLuint createVAO(float* pointArray, int length)
{
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, length * sizeof(float), pointArray, GL_STATIC_DRAW);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	return vao;
}

GLuint createTriangleVAO() {
	float points[] = {
			0.0f,  0.5f,  0.0f,
			0.5f, -0.5f,  0.0f,
			-0.5f, -0.5f,  0.0f
	};

	return createVAO(points, 3 * 3);
}

GLuint createCubeVAO(std::vector<float>& points) {
	// (three floats per point) * (three points per triangle) * (two triangles per face) * (six faces per cube)
	// 6 * 3 * 2 * 6
	return createVAO(&points[0], points.size());
}

void setupProjection(GLuint program)
{
	glm::vec3 eye(80, 40, 80);
	glm::mat4 view = glm::lookAt(
		eye,
		glm::vec3(0.0f, 4.0f, 0.0f),
		glm::vec3(0.0f, 1.f, 0.f)
	);

	glm::mat4 model = glm::mat4(1.0f);
	model[0] = glm::vec4(1, 0, 0, 0);
	model[1] = glm::vec4(0, 1, 0, 0);
	model[2] = glm::vec4(0, 0, 1, 0);
	model[3] = glm::vec4(0, 0, 0, 1);
	GLint modelUniform = glGetUniformLocation(program, "Model");
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));

	GLint viewUniform = glGetUniformLocation(program, "View");
	printf("%i view\n", viewUniform);
	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(view));

	const float farClip = 600;
	glm::mat4 proj = glm::perspective
	(
		glm::radians(45.0f),
		1280.0f / 768.0f,
		0.1f,
		farClip
	);
	GLint projUniform = glGetUniformLocation(program, "Projection");
	glUniformMatrix4fv(projUniform, 1, GL_FALSE, glm::value_ptr(proj));
	printf("%i proj\n", projUniform);
}

std::vector<VizData> visualizeOctree(const Octree* node)
{
	std::vector<VizData> nums;

	VizData stuff = { node->m_size, node->m_min };
	nums.push_back(stuff);

	const auto octreeChildren = node->GetChildren();
	if (!octreeChildren)
	{
		return nums;
	}

	auto children = octreeChildren->children;
	for (const Octree* child : children)
	{
		if (child)
		{
			std::vector<VizData> childData = visualizeOctree(child);
			nums.insert(nums.end(), childData.begin(), childData.end());
		}
	}

	return nums;
}

std::vector<float> cubePoints() {
	std::vector<float> points =
	{
		// front face
		// pos      // color    // normal, here just placeholding not used
		1, 0, 0,    1, 0, 0,    1, 0, 0,
		0, 1, 0,    0, 1, 0,    0, 1, 0,
		1, 1, 0,    0, 0, 1,    0, 0, 1,

		1, 0, 0,    1, 0, 0,    1, 0, 0,
		0, 0, 0,    0, 1, 0,    0, 1, 0,
		0, 1, 0,    0, 0, 1,    0, 0, 1,

		// back                 
		1, 0, 1,    1, 0, 0,    1, 0, 0,
		1, 1, 1,    0, 1, 0,    0, 1, 0,
		0, 1, 1,    0, 0, 1,    0, 0, 1,

		1, 0, 1,    1, 0, 0,    1, 0, 0,
		0, 1, 1,    0, 1, 0,    0, 1, 0,
		0, 0, 1,    0, 0, 1,    0, 0, 1,

		// left                 
		0, 0, 0,    1, 0, 0,    1, 0, 0,
		0, 0, 1,    0, 1, 0,    0, 1, 0,
		0, 1, 1,    0, 0, 1,    0, 0, 1,


		0, 0, 0,    1, 0, 0,    1, 0, 0,
		0, 1, 1,    0, 0, 1,    0, 0, 1,
		0, 1, 0,    0, 1, 0,    0, 1, 0,

		// right                
		1, 0, 0,    1, 0, 0,    1, 0, 0,
		1, 1, 1,    0, 1, 0,    0, 1, 0,
		1, 0, 1,    0, 0, 1,    0, 0, 1,

		1, 0, 0,    1, 0, 0,    1, 0, 0,
		1, 1, 0,    0, 0, 1,    0, 0, 1,
		1, 1, 1,    0, 1, 0,    0, 1, 0,

		// top                  
		0, 1, 0,    1, 0, 0,    1, 0, 0,
		1, 1, 1,    0, 0, 1,    0, 0, 1,
		1, 1, 0,    0, 1, 0,    0, 1, 0,

		0, 1, 0,    1, 0, 0,    1, 0, 0,
		0, 1, 1,    0, 0, 1,    0, 0, 1,
		1, 1, 1,    0, 1, 0,    0, 1, 0,

		// bottom               
		0, 0, 0,    1, 0, 0,    1, 0, 0,
		1, 0, 0,    0, 1, 0,    0, 1, 0,
		1, 0, 1,    0, 0, 0,    0, 0, 0,

		0, 0, 0,    1, 0, 0,    1, 0, 0,
		1, 0, 1,    0, 0, 1,    0, 0, 1,
		0, 0, 1,    0, 1, 0,    0, 1, 0,
	};
	return points;
}