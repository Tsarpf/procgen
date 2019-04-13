#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Octree.h"
#include "Mesh.h"
#include "utils.h"


enum Direction {
	xplus, xminus,
	yplus, yminus,
	zplus, zminus
};

struct OctreeVisualizationData {
	int size;
	glm::vec3 min;
};

class OctreeMesh : public Mesh
{
public:
	//OctreeMesh();
	OctreeMesh(GLuint program, const int size, const glm::vec3 position);
	~OctreeMesh();
	void Load();

	void EnlargePlus(Direction);
	void EnlargeMinus(Direction);

	Octree* GetOctree();

	static std::vector<OctreeVisualizationData> VisualizeOctree(const Octree*);

private:
	Octree* m_tree;
	glm::vec3 m_position;
	int m_size;


	void DrawVisualization(const int stride, const float time, const GLuint program,
		const GLuint vao, const GLuint vbo, const int elementCount, const std::vector<OctreeVisualizationData>& nodes);
};
