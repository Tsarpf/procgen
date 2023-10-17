#include "Scene.h"
#include <GLFW/glfw3.h>


//void Scene::SpaceCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
void Scene::KeyCallback(int key, int action) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
		case GLFW_KEY_SPACE:
			printf("space pressed\n");
			//m_center = m_center *= 2;
			// m_center.x = m_center.x * 2;
			// m_center.z = m_center.z * 2;
			//m_eye = m_eye *= 2;
			setupProjection(m_program, m_eye, m_center);

			//m_mesh->Enlarge(xminus);
			m_mesh->EnlargeAsync(xplus);
			//m_mesh->EnlargeAsync(xplus);
			break;
		case GLFW_KEY_LEFT:
			//m_orientation += 0.1;
			m_eye += glm::vec3(0, 0, -2.1f);
			m_center += glm::vec3(0, 0, -2.1f);
			setupProjection(m_program, m_eye, m_center);
			break;
		case GLFW_KEY_RIGHT:
			m_eye += glm::vec3(0, 0, 2.1f);
			m_center += glm::vec3(0, 0, 2.1f);
			//m_orientation -= 0.1;
			setupProjection(m_program, m_eye, m_center);
			break;
		case GLFW_KEY_DOWN:
			m_eye += glm::vec3(-2.1, 0, 0);
			m_center += glm::vec3(-2.1, 0, 0);
			setupProjection(m_program, m_eye, m_center);
			break;
		case GLFW_KEY_UP:
			m_eye += glm::vec3(2.1, 0, 0);
			m_center += glm::vec3(2.1, 0, 0);
			setupProjection(m_program, m_eye, m_center);
			break;
		}
		return;
	}
}

Scene::Scene(GLFWwindow* window) 
	: m_window(window), m_orientation(0)
{
	m_eye = glm::vec3(0, 80, 80);
	m_center = glm::vec3(0, 4, 8);
}

void Scene::Initialize()
{
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int, int action, int)
	{
		auto scene = reinterpret_cast<Scene*>(glfwGetWindowUserPointer(window));
		scene->KeyCallback(key, action);
	});
	glfwSetWindowUserPointer(m_window, this);

	m_t_start = std::chrono::high_resolution_clock::now();
	m_program = createTriangleProgram();

	const int octreeSize = 256;
	m_center = glm::vec3(octreeSize/2.f, octreeSize/8.f, octreeSize/2.f);
	m_eye = glm::vec3(0, octreeSize/2.f, octreeSize/2.f);

	m_mesh = new OctreeMesh(m_program, octreeSize, glm::vec3(0, 0, 0));
	m_mesh->BuildOctree();
	m_mesh->LoadMesh();

	setupProjection(m_program, m_eye, m_center);

	//glEnable(GL_CULL_FACE); // sets cullingments
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void Scene::Render()
{
	glUseProgram(m_program);
	auto t_now = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - m_t_start).count();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // set to GL_LINE to disable wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // set to GL_LINE to disable wireframe
	//glEnable(GL_CULL_FACE); // enable/disable culling

	//m_mesh->Draw(time);
	m_mesh->CheckResults(); // tad ugly, mebbe do something
	m_mesh->Draw(m_orientation);
}

Scene::~Scene()
{
}
