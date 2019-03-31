//
// Created by Teemu Sarapisto on 19/07/2017.
//

#include "utils.h"

#include <string>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

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

    window = glfwCreateWindow(800, 600, "Dual contouring methods", NULL, NULL);
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

// Attrib pointers bind to specific VAOs, should probabaly add these right after
// binding the VAO?
void setAttribPointers(GLuint program)
{
	int stride = 9 * sizeof(float); // should use sizeof vertex struct for this
    // three position elements per vertex
    GLint posAttrib = glGetAttribLocation(program, "position");
    printf("%i posAttrib\n", posAttrib);
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, stride, NULL);

    GLint colorAttrib = glGetAttribLocation(program, "inColor");
    printf("%i colorAttrib\n", colorAttrib);
    glEnableVertexAttribArray(colorAttrib);
	void* offset = (void*)(3*sizeof(float));
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE, stride, offset);

    GLint normalAttrib = glGetAttribLocation(program, "normal");
    printf("%i normalAttrib\n", normalAttrib);
    glEnableVertexAttribArray(normalAttrib);
	offset = (void*)(6 * sizeof(float));
    glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, stride, offset);
}

GLuint createIndexVAO(const VertexBuffer& vb, const IndexBuffer& ib)
{
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vb.size() * sizeof(Vertex), vb.data(), GL_STATIC_DRAW);

	GLuint ibo = 0;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * ib.size(), ib.data(), GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	// clean up?
	glBindVertexArray(0);
	return vao;
}

GLuint createTriangleProgram()
{
    GLuint program = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef _WIN32
	printf("is win32");
    std::string vertStr = readFile("../src/simple.vert");
    const char* vertStrC = vertStr.c_str();
    std::string fragStr = readFile("../src/simple.frag");
    const char* fragStrC = fragStr.c_str();
#else
	printf("is not win32");
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
    glm::vec3 eye(-10, 32, 32);
    glm::mat4 view = glm::lookAt(
        eye,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.f, 0.f)
    );

    glm::mat4 model = glm::mat4(1.0f);
    model[0] = glm::vec4(1, 0, 0, 0);
    model[1] = glm::vec4(0, 1, 0, 0);
    model[2] = glm::vec4(0, 0, 1, 0);
    //model[3] = glm::vec4(0, 0, 0, 1);
    GLint modelUniform = glGetUniformLocation(program, "Model");
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(model));

    GLint viewUniform = glGetUniformLocation(program, "View");
    printf("%i view\n", viewUniform);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective
    (
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f,
        200.0f
    );
    GLint projUniform = glGetUniformLocation(program, "Projection");
    glUniformMatrix4fv(projUniform, 1, GL_FALSE, glm::value_ptr(proj));
    printf("%i proj\n", projUniform);
}

std::vector<VizData> visualizeOctree(const Octree* node)
{
    std::vector<VizData> nums;

    VizData stuff = {node->m_size, node->m_min};
    nums.push_back(stuff);

    const auto octreeChildren = node->GetChildren();
    if (!octreeChildren) 
    {
        return nums;
    }

    auto children = octreeChildren->children;
    for(const Octree* child : children)
    {
        if(child)
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