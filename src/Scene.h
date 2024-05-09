#pragma once

//#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

#include "OctreeMesh.h"
#include "utils.h"
#include "Octree.h"

class Scene
{
public:
	Scene(GLFWwindow*);
	~Scene();
	void Initialize();
	void Render();
	void Update();
	void KeyCallback(int key, int action);

private:
	// Other
	GLuint m_program;
	GLFWwindow* m_window;
	OctreeMesh* m_mesh;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_t_start;
	float m_time;
	glm::vec3 m_eye;
	glm::vec3 m_center;
	bool m_wireframe = true;
	uint16_t m_chunkSize = 64;
	glm::ivec3 m_chunkCursor; // this could be eg. the chunk the the player is in, should only really be used for the root node
};

