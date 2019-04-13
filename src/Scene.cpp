#include "Scene.h"
#include <GLFW/glfw3.h>

//#define GLSL(src) #src

//void Scene::SpaceCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
void Scene::SpaceCallback(int key, int action) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		printf("space pressed\n");
		//g_the_mesh_todo_refactor_to_somewhere->EnlargePlus(xplus);
		//g_the_mesh_todo_refactor_to_somewhere->EnlargePlus(yplus);
		//g_the_mesh_todo_refactor_to_somewhere->EnlargePlus(zplus);
		//g_the_mesh_todo_refactor_to_somewhere->EnlargeMinus(xminus);
		//g_the_mesh_todo_refactor_to_somewhere->EnlargeMinus(yminus);
		m_mesh->EnlargeMinus(zminus);
		m_visualizationData = OctreeMesh::VisualizeOctree(m_mesh->GetOctree());
	}
}




Scene::Scene(GLFWwindow* window) : m_window(window)
{
	
}

void Scene::Render()
{
	glUseProgram(m_program);
	auto t_now = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - m_t_start).count();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// visualize the octree
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // set to use wireframe
	//drawVisualization(6, time, m_program, vao, vbo, genericCubePoints.size(), g_visualizationData);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // set to disable wireframe

	glEnable(GL_CULL_FACE);

	m_mesh->Draw(time);
}


void Scene::Initialize()
{
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int, int action, int)
	{
		auto scene = reinterpret_cast<Scene*>(glfwGetWindowUserPointer(window));
		scene->SpaceCallback(key, action);
	});
	glfwSetWindowUserPointer(m_window, this);

	m_t_start = std::chrono::high_resolution_clock::now();
	m_program = createTriangleProgram();

	const int octreeSize = 32;
	const int axisCount = 3;

	m_mesh = new OctreeMesh(m_program, octreeSize, glm::vec3(0, 0, 0));
	m_mesh->Load();

	setupProjection(m_program);

	// glEnable(GL_CULL_FACE); // sets cullingments
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); // set to use wireframe
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//TODO: refactor to some sort of octree visualizer class
	m_visualizationData = OctreeMesh::VisualizeOctree(m_mesh->GetOctree());
	std::vector<float> genericCubePoints = cubePoints();
	auto [vao, vbo] = createCubeVAO(genericCubePoints);
}

Scene::~Scene()
{
}
