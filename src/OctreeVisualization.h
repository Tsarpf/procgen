#pragma once
#include <vector>
#include <GL/glew.h>
#include "glm/glm.hpp"

struct OctreeVisualizationData {
	int size;
	glm::vec3 min;
};

class Octree;
class OctreeVisualization
{
public:
	OctreeVisualization(GLuint program);
	~OctreeVisualization();

	void DrawVisualization(const float time);
	void Build(const Octree*);
private:
	const int m_stride = 6;
	GLuint m_program;
	GLuint m_vao;
	GLuint m_vbo;
	std::vector<OctreeVisualizationData> m_visualizationData;
	std::vector<float> m_cubePoints =
	{
		// front face
		// pos      // color
		1, 0, 0,    1, 0, 0,
		0, 1, 0,    0, 1, 0,
		1, 1, 0,    0, 0, 1,

		1, 0, 0,    1, 0, 0,
		0, 0, 0,    0, 1, 0,
		0, 1, 0,    0, 0, 1,

		// back             
		1, 0, 1,    1, 0, 0,
		1, 1, 1,    0, 1, 0,
		0, 1, 1,    0, 0, 1,

		1, 0, 1,    1, 0, 0,
		0, 1, 1,    0, 1, 0,
		0, 0, 1,    0, 0, 1,

		// left             
		0, 0, 0,    1, 0, 0,
		0, 0, 1,    0, 1, 0,
		0, 1, 1,    0, 0, 1,


		0, 0, 0,    1, 0, 0,
		0, 1, 1,    0, 0, 1,
		0, 1, 0,    0, 1, 0,

		// right            
		1, 0, 0,    1, 0, 0,
		1, 1, 1,    0, 1, 0,
		1, 0, 1,    0, 0, 1,

		1, 0, 0,    1, 0, 0,
		1, 1, 0,    0, 0, 1,
		1, 1, 1,    0, 1, 0,

		// top              
		0, 1, 0,    1, 0, 0,
		1, 1, 1,    0, 0, 1,
		1, 1, 0,    0, 1, 0,

		0, 1, 0,    1, 0, 0,
		0, 1, 1,    0, 0, 1,
		1, 1, 1,    0, 1, 0,

		// bottom           
		0, 0, 0,    1, 0, 0,
		1, 0, 0,    0, 1, 0,
		1, 0, 1,    0, 0, 0,

		0, 0, 0,    1, 0, 0,
		1, 0, 1,    0, 0, 1,
		0, 0, 1,    0, 1, 0,
	};
};