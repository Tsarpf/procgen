#include "OctreeMesh.h"
#include "utils.h"

#include <memory>


OctreeMesh::OctreeMesh(GLuint program, const int size, const glm::vec3 position) 
	: m_size(size), m_position(position), Mesh(program), m_visualization(program)
{
}

OctreeMesh::~OctreeMesh()
{
}

void OctreeMesh::Load()
{
	m_tree = new Octree(1, m_size, m_position);
	m_tree->ConstructBottomUp();
	m_tree->MeshFromOctree(m_indices, m_vertices);
	SetupGlBuffers();

	m_visualization.Build(m_tree);
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

	// x minus 
	{
		// old
		{{0, 0, 0, 1}, {0, 1, 0, 3}, {0, 0, 1, 5}, {0, 1, 1, 7}},
		// new
		{{1, 0, 0, 0}, {1, 1, 0, 2}, {1, 0, 1, 4}, {1, 1, 1, 6}},
	},

	// y plus 
	{
		// old
		{{0, 1, 0, 0}, {1, 1, 0, 1}, {0, 1, 1, 4}, {1, 1, 1, 5}},
		// new
		{{0, 0, 0, 2}, {1, 0, 0, 3}, {0, 0, 1, 6}, {1, 0, 1, 7}},
	},

	// y minus 
	{
		// old
		{{0, 0, 0, 2}, {1, 0, 0, 3}, {0, 0, 1, 6}, {1, 0, 1, 7}},
		// new
		{{0, 1, 0, 0}, {1, 1, 0, 1}, {0, 1, 1, 4}, {1, 1, 1, 5}},
	},

	// z plus 
	{
		// old
		{{0, 0, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 2}, {1, 1, 1, 3}},
		// new
		{{0, 0, 0, 4}, {1, 0, 0, 5}, {0, 1, 0, 6}, {1, 1, 0, 7}},
	},

	// z minus 
	{
		// old
		{{0, 0, 0, 4}, {1, 0, 0, 5}, {0, 1, 0, 6}, {1, 1, 0, 7}},
		// new
		{{0, 0, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 2}, {1, 1, 1, 3}},
	},
};

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
	std::vector<OctreeVisualizationData> viz;
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

			//if (child)
			//{
			//	std::vector<OctreeVisualizationData> childData = VisualizeOctree(child);
			//	viz.insert(viz.end(), childData.begin(), childData.end());
			//}
		}
		j++;
	}
	// auto res = std::remove_if(viz.begin(), viz.end(), [](OctreeVisualizationData v) { return v.size != 1; });
	// viz.erase(res, viz.end());
	// g_visualizationData = viz;

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
	m_visualization.Build(m_tree);
}
// TODO merge with EnlargePlus
void OctreeMesh::EnlargeMinus(Direction dir)
{

	int newCornerIdx = index(0, 0, 0, 2); 
	int oldCornerIdx;

	glm::vec3 offset;

	switch (dir)
	{
	case xminus:
		oldCornerIdx = index(1, 0, 0, 2);
		offset = glm::vec3(m_size, 0, 0);
		break;
	case yminus:
		oldCornerIdx = index(0, 1, 0, 2); 
		offset = glm::vec3(0, m_size, 0);
		break;
	case zminus:
		oldCornerIdx = index(0, 0, 1, 2); 
		offset = glm::vec3(0, 0, m_size);
		break;
	}
	glm::vec3 newPosition = m_position - offset;

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
	std::vector<OctreeVisualizationData> viz;
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

			// if (child)
			// {
			// 	std::vector<OctreeVisualizationData> childData = VisualizeOctree(child);
			// 	viz.insert(viz.end(), childData.begin(), childData.end());
			// }
		}
		j++;
	}
	// auto res = std::remove_if(viz.begin(), viz.end(), [](OctreeVisualizationData v) { return v.size != 1; });
	// viz.erase(res, viz.end());
	// g_visualizationData = viz;

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
	m_position -= offset;
	m_tree = new Octree(std::move(newRootChildren), m_size, m_position, 1);

	m_visualization.Build(m_tree);
}

void OctreeMesh::Draw(const float time)
{
	m_visualization.DrawVisualization(time);
	Mesh::Draw(time);
}