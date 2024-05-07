#include "Scene.h"
#include <GLFW/glfw3.h>
#include <map>

void Scene::KeyCallback(int key, int action) {
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_SPACE:
			printf("space pressed\n");
			setupProjection(m_program, m_eye, m_center);
			m_mesh->EnlargeAsync(xplus);
			break;
		}
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

	const int octreeSize = 128;
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

void Scene::Update() {
	glm::vec3 direction = glm::normalize(m_eye - m_center);
	glm::vec3 right = glm::normalize(glm::cross(m_center - m_eye, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::mat4 rotation;

	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
		m_eye -= direction * 2.1f;
		m_center -= direction * 2.1f;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
		m_eye += direction * 2.1f;
		m_center += direction * 2.1f;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) {
		rotation = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), right);
		m_center = glm::vec3(rotation * glm::vec4(m_center - m_eye, 1.0f)) + m_eye;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), right);
		m_center = glm::vec3(rotation * glm::vec4(m_center - m_eye, 1.0f)) + m_eye;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		rotation = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_center = glm::vec3(rotation * glm::vec4(m_center - m_eye, 1.0f)) + m_eye;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		m_center = glm::vec3(rotation * glm::vec4(m_center - m_eye, 1.0f)) + m_eye;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		m_eye -= right * 2.1f;
		m_center -= right * 2.1f;
		setupProjection(m_program, m_eye, m_center);
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
		m_eye += right * 2.1f;
		m_center += right * 2.1f;
		setupProjection(m_program, m_eye, m_center);
	}
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
