//
// Created by Teemu Sarapisto on 19/07/2017.
//

#include "utils.h"

#include <string>
#include <fstream>
#include <iostream>

// Mostly just mix & matched to my liking from:
// https://github.com/capnramses/antons_opengl_tutorials_book/blob/master/40_compute_shader/gl_utils.cpp
// http://www.glfw.org/docs/3.0/quick.html
// http://www.nexcius.net/2012/11/20/how-to-load-a-glsl-shader-in-opengl-using-c/

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
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
    for(int i = 7; i >= 0; i--)
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
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(640, 480, "OpenGL Boilerplate", NULL, NULL);
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


    glfwSetKeyCallback(window, key_callback);
    return window;
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

void printShaderInfoLog (GLuint shader) {
    int max_length = 4096;
    int actual_length = 0;
    char slog[4096];
    glGetShaderInfoLog (shader, max_length, &actual_length, slog);
    fprintf (stderr, "shader info log for GL index %u\n%s\n", shader, slog);
}

void printProgramInfoLog (GLuint program) {
    int max_length = 4096;
    int actual_length = 0;
    char plog[4096];
    glGetProgramInfoLog (program, max_length, &actual_length, plog);
    fprintf (stderr, "program info log for GL index %u\n%s\n", program, plog);
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

bool checkProgramErrors (GLuint program) {
    GLint success = -1;
    glGetProgramiv (program, GL_LINK_STATUS, &success);
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

	// For now I just use visual studio and this works
#ifdef _WIN32
    std::string vertStr = readFile("../simple.vert");
    const char* vertStrC = vertStr.c_str();
    std::string fragStr = readFile("../simple.frag");
    const char* fragStrC = fragStr.c_str();
#else
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

    return program;
}

GLuint createTriangleVAO() {
    float points[] = {
            0.0f,  0.5f,  0.0f,
            0.5f, -0.5f,  0.0f,
            -0.5f, -0.5f,  0.0f
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    return vao;
}

void visualizeOctree(Octree* tree)
{
    std::cout << "visualizing octree" << std::endl;
    auto rootChildField = tree->GetChildren()->field;
    //auto rootChild = (*tree->GetChildren())->children[0];
    //auto childField = (*rootChild->GetChildren())->field;

    //printBinary(childField);

    printBinary(rootChildField);
}

