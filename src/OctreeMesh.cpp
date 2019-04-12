#include "OctreeMesh.h"
#include "utils.h"

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

Octree* OctreeMesh::GetOctree()
{
	return m_tree;
}

int edgeIndexLookup[6][2][4][4] = {
	// x plus 
	{
		// old
		{{1, 0, 0, 0}, {1, 1, 0, 2}, {1, 0, 1, 4}, {1, 1, 1, 6}},
		// new
		{{0, 0, 0, 1}, {0, 1, 0, 3}, {0, 0, 1, 5}, {0, 1, 1, 7}},
	},

	// NYI
	// x minus 
	{
		// old
		{{0, 1, 0, 0}, {1, 1, 1, 2}, {1, 1, 0, 4}, {1, 1, 1, 6}},
		// new
		{{0, 0, 0, 1}, {0, 0, 1, 3}, {0, 0, 0, 5}, {0, 0, 1, 7}},
	},

	// doing 
	// y plus 
	{
		// old
		{{0, 1, 0, 0}, {1, 1, 0, 1}, {0, 1, 1, 4}, {1, 1, 1, 5}},
		// new
		{{0, 0, 0, 2}, {1, 0, 0, 3}, {0, 0, 1, 6}, {1, 0, 1, 7}},
	},


	// NYI
	// y minus 
	{
		// old
		{{1, 0, 0, 0}, {1, 0, 1, 2}, {1, 1, 0, 4}, {1, 1, 1, 6}},
		// new
		{{0, 0, 0, 1}, {0, 0, 1, 3}, {0, 1, 0, 5}, {0, 1, 1, 7}},
	},

	// NYI
	// y plus 
	{
		// old
		{{1, 0, 0, 0}, {1, 0, 1, 2}, {1, 1, 0, 4}, {1, 1, 1, 6}},
		// new
		{{0, 0, 0, 1}, {0, 0, 1, 3}, {0, 1, 0, 5}, {0, 1, 1, 7}},
	},

	// NYI
	// y minus 
	{
		// old
		{{1, 0, 0, 0}, {1, 0, 1, 2}, {1, 1, 0, 4}, {1, 1, 1, 6}},
		// new
		{{0, 0, 0, 1}, {0, 0, 1, 3}, {0, 1, 0, 5}, {0, 1, 1, 7}},
	},
};

extern std::vector<VizData> g_visualizationData;
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

	std::array<Octree*, 8> rootChildren = {};
	for (auto& child : rootChildren)
	{
		child = nullptr;
	}

	//m_indices.clear();
	//m_vertices.clear();

	rootChildren[oldCornerIdx] = m_tree;
	rootChildren[newCornerIdx] = new Octree(1, m_size, newPosition);
	rootChildren[newCornerIdx]->ConstructBottomUp();
	rootChildren[newCornerIdx]->MeshFromOctree(m_indices, m_vertices);

	std::vector<std::tuple<int, int>> idxs = {{0, oldCornerIdx}, {1, newCornerIdx}};
	std::array<Octree*, 8> borderChildren;
	for (auto& child : borderChildren)
	{
		child = nullptr;
	}
	std::vector<VizData> viz;
	for(auto [j, childIdx] : idxs)
	{
		if (!rootChildren[childIdx]->GetChildren())
		{
			continue;
		}
		std::array<Octree*, 8> children = rootChildren[childIdx]->GetChildren()->children;
		for (int i = 0; i < 4; i++)
		{
			auto[x, y, z, edgeIdx] = edgeIndexLookup[dir][j][i];
			int idx = index(x, y, z, 2);
			auto child = children[idx];
			borderChildren[edgeIdx] = child;
			Octree::GenerateVertexIndices(child, m_vertices);

			if (child)
			{
				std::vector<VizData> childData = visualizeOctree(child);
				viz.insert(viz.end(), childData.begin(), childData.end());
			}
		}
		j++;
	}
	auto res = std::remove_if(viz.begin(), viz.end(), [](VizData v) { return v.size != 1; });
	viz.erase(res, viz.end());
	g_visualizationData = viz;

	std::cout << "-------------------------------- Processing edges --------------------------------" << std::endl;
	Octree::CellChildProc(borderChildren, m_indices);
	std::cout << "edges processed" << std::endl;
	SetupGlBuffers();

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
