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

int edgeIndexLookup[6][2][4][4] = {
	// x plus 
	{
		// new
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
		// old
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
	},

	// NYI
	// x minus 
	{
		// new
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
		// old
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
	},
	// y plus 
	{
		// new
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
		// old
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
	},
	// y minus 
	{
		// new
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
		// old
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
	},
	// y plus 
	{
		// new
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
		// old
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
	},
	// y minus 
	{
		// new
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
		// old
		{{0, 0, 0, 2}, {0, 1, 0, 2}, {0, 0, 1, 2}, {0, 1, 1, 2}},
	},
};

void OctreeMesh::EnlargePlus(Direction dir)
{
	int oldCornerIdx = index(0, 0, 0, 2);
	int newCornerIdx;

	glm::vec3 newPosition;

	std::array<Octree*, 8> edgeChildren;


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

	std::array<Octree*, 8> rootChildren = {};
	for (auto& child : rootChildren)
	{
		child = nullptr;
	}
	rootChildren[oldCornerIdx] = m_tree;
	rootChildren[newCornerIdx] = new Octree(1, m_size, newPosition);
	rootChildren[newCornerIdx]->ConstructBottomUp();
	rootChildren[newCornerIdx]->MeshFromOctree(m_indices, m_vertices);
	SetupGlBuffers();

	//auto edgeIndicesNew = edgeIndexLookup[dir][0];
	//auto edgeIndicesOld = edgeIndexLookup[dir][1];
	//int idxs[2][2] = {{0, newCornerIdx}, {1, oldCornerIdx}};

	std::vector<std::tuple<int, int>> idxs = {{0, newCornerIdx}, {1, oldCornerIdx}};
	std::array<Octree*, 8> borderChildren;
	for(auto [j, childIdx] : idxs)
	{
		std::array<Octree*,8> children = rootChildren[childIdx]->GetChildren()->children;
		for (int i = 0; i < 4; i++)
		{
			auto[x, y, z, dim] = edgeIndexLookup[dir][j][i];
			int idx = index(x, y, z, dim);
			int edgeIdx = i + 4 * j;
			auto child = children[idx];
			borderChildren[edgeIdx] = child;
			Octree::GenerateVertexIndices(child, m_vertices);
		}
		j++;
	}

	std::cout << "-------------------------------- Processing edges --------------------------------" << std::endl;
	Octree::CellChildProc(borderChildren, m_indices);
	std::cout << "edges processed" << std::endl;

	std::unique_ptr<OctreeChildren> newRootChildren(new OctreeChildren
	{
		(uint8_t)((1 << oldCornerIdx) | (1 << newCornerIdx)),
		rootChildren
	});

	m_size *= 2;
	m_tree = new Octree(std::move(newRootChildren), m_size, m_position, 1);
}

void OctreeMesh::EnlargeMinus(Direction dir)
{

}

OctreeMesh::~OctreeMesh()
{
}
