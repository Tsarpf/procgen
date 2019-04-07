#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Octree.h"
#include "Mesh.h"

class OctreeMesh : public Mesh
{
public:
	//OctreeMesh();
	OctreeMesh(GLuint program, const int size, const glm::vec3 position);
	~OctreeMesh();
	void Load();

private:
	Octree* m_tree;
	glm::vec3 m_position;
	const int m_size;
};
