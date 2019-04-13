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
	void SpaceCallback(int key, int action);

private:
	GLuint m_program;
	GLFWwindow* m_window;
	OctreeMesh* m_mesh;
	std::vector<OctreeVisualizationData> m_visualizationData;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_t_start;
};

