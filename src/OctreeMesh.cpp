#include "OctreeMesh.h"



OctreeMesh::OctreeMesh(GLuint program, const int size, const glm::vec3 position) : m_size(size), m_position(position), Mesh(program)
{
}

void OctreeMesh::Load()
{
	m_tree = new Octree(1, m_size, m_position);
	m_tree->MeshFromOctree(m_indices, m_vertices);
	SetupGlBuffers();
}


OctreeMesh::~OctreeMesh()
{
}
