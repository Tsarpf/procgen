#include "OctreeMesh.h"

#include <memory>


OctreeMesh::OctreeMesh(GLuint program, const int size, const glm::vec3 position) : m_size(size), m_position(position), Mesh(program)
{
}

void OctreeMesh::Load()
{
	m_tree = new Octree(1, m_size, m_position);
	m_tree->ConstructBottomUp();
	m_tree->MeshFromOctree(m_indices, m_vertices);
	SetupGlBuffers();
}


void OctreeMesh::EnlargePlus(Direction dir)
{
	int oldCornerIdx = index(0, 0, 0, 2);
	int newCornerIdx;

	glm::vec3 newPosition;

	switch (dir)
	{
	case xplus:
		newCornerIdx = index(1, 0, 0, 2); 
		newPosition = m_position + glm::vec3(m_size, 0, 0);
		break;
	case yplus:
		newCornerIdx = index(0, 1, 0, 2); 
		newPosition = m_position + glm::vec3(0, m_size, 0);
		break;
	case zplus:
		newCornerIdx = index(0, 0, 1, 2); 
		newPosition = m_position + glm::vec3(0, 0, m_size);
		break;
	}
	// same new min for bigger octree

	std::array<Octree*, 8> children = {};
	for (auto& child : children)
	{
		child = nullptr;
	}
	children[oldCornerIdx] = m_tree;
	children[newCornerIdx] = new Octree(1, m_size, newPosition);
	children[newCornerIdx]->ConstructBottomUp();
	children[newCornerIdx]->MeshFromOctree(m_indices, m_vertices);
	SetupGlBuffers();

	std::unique_ptr<OctreeChildren> newChildren(new OctreeChildren
	{
		(uint8_t)((1 << oldCornerIdx) | (1 << newCornerIdx)),
		children
	});

	m_size *= 2;
	m_tree = new Octree(std::move(newChildren), m_size, m_position, 1);
}

void OctreeMesh::EnlargeMinus(Direction dir)
{

}

OctreeMesh::~OctreeMesh()
{
}
