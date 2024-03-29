#include "OctreeMesh.h"
#include "utils.h"

#include <memory>

OctreeMesh::OctreeMesh(GLuint program, const int size, const glm::vec3 position) 
	: m_size(size), m_position(position), Mesh(program), m_visualization(program)
{
}

OctreeMesh::OctreeMesh(GLuint program, const int size, const glm::vec3 position, Octree* tree, VertexBuffer vertices, IndexBuffer indices) 
	: m_size(size), m_position(position), Mesh(program, vertices, indices), m_visualization(program), m_tree(tree)
{
	m_visualization.Build(m_tree);
}

OctreeMesh::~OctreeMesh()
{
}
void OctreeMesh::BuildOctree()
{
	m_tree = new Octree(1, m_size, m_position);
	m_tree->Construct();
	m_tree->MeshFromOctree(m_indices, m_vertices);

	m_visualization.Build(m_tree);
}

void OctreeMesh::LoadMesh()
{
	m_visualization.Initialize();
	SetupGlBuffers();
	UploadData();
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

void BuildSeam(Octree& n1, Octree& n2, Direction dir,
	VertexBuffer& vertices, IndexBuffer& indices)
{
	std::vector<std::tuple<int, const Octree&>> idxs = {{0, n1}, {1, n2}};
	std::array<Octree*, 8> borderChildren;
	for (auto& child : borderChildren)
	{
		child = nullptr;
	}
	std::vector<OctreeVisualizationData> viz;
	for(auto [j, node] : idxs)
	{
		if (!node.GetChildren())
		{
			continue;
		}
		std::array<Octree*, 8> children = node.GetChildren()->children;
		for (int i = 0; i < 4; i++)
		{
			auto[x, y, z, edgeIdx] = edgeIndexLookup[dir][j][i];
			int idx = index(x, y, z, 2);
			auto child = children[idx];
			borderChildren[edgeIdx] = child;
			Octree::GenerateVertexIndices(child, vertices);
			// if (child)
			// {
			// 	std::vector<OctreeVisualizationData> childData = VisualizeOctree(child);
			// 	viz.insert(viz.end(), childData.begin(), childData.end());
			// }
		}
		j++;
	}
	std::cout << "-------------------------------- Processing edges --------------------------------" << std::endl;
	//Octree::CellChildProc(borderChildren, indices);
	switch (dir)
	{
	case xplus:
		Octree::FaceProcX(&n1, &n2, indices);
		break;
	case xminus:
		Octree::FaceProcX(&n2, &n1, indices);
		break;

	case yplus:
		Octree::FaceProcY(&n1, &n2, indices);
		break;
	case yminus:
		Octree::FaceProcY(&n2, &n1, indices);
		break;

	case zplus:
		Octree::FaceProcZ(&n1, &n2, indices);
		break;
	case zminus:
		Octree::FaceProcZ(&n2, &n1, indices);
		break;
	}
	std::cout << "edges processed" << std::endl;
}

OctreeMesh* CreateNewMeshTask(Octree* neighbour, Octree* newOctree,
	const int size, Direction dir, const glm::vec3 position, GLuint program)
{
	VertexBuffer vertices;
	IndexBuffer indices;

	newOctree->Construct();
	newOctree->MeshFromOctree(indices, vertices);

	BuildSeam(*neighbour, *newOctree, dir, vertices, indices);

	OctreeMesh* mesh = new OctreeMesh(program, size, position, newOctree, std::move(vertices), std::move(indices));

	return mesh;
}

template<typename R>
bool is_ready(std::future<R> const& f)
	#ifdef _MSC_VER
	{ return f._Is_ready(); }
	#else
    { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }
	#endif

void OctreeMesh::CheckResults()
{
	std::vector<int> readies;
	for (int i = 0; i < m_futureMeshes.size(); i++)
	{
		auto& future = m_futureMeshes[i];
		if (is_ready(future))
		{
			//m_indices.clear();
			//m_vertices.clear();
			readies.push_back(i);
			OctreeMesh* mesh = future.get();
			mesh->LoadMesh();
			std::lock_guard<std::mutex> guard(m_childMeshMutex);
			m_childMeshes.push_back(mesh);

			m_futureMeshes.erase(m_futureMeshes.begin() + i);
		}
	}
}
void OctreeMesh::EnlargeAsync(Direction dir)
{
	auto [newCornerIdx, oldCornerIdx, newPosition] = EnlargeCorners(dir);

	std::array<Octree*, 8> rootChildren = {};
	for (auto& child : rootChildren)
	{
		child = nullptr;
	}

	rootChildren[oldCornerIdx] = m_tree;
	rootChildren[newCornerIdx] = new Octree(1, m_size, newPosition);

	std::unique_ptr<OctreeChildren> newRootChildren(new OctreeChildren
	{
		(uint8_t)((1 << oldCornerIdx) | (1 << newCornerIdx)),
		rootChildren
	});

	m_size *= 2;
	m_tree = new Octree(std::move(newRootChildren), m_size, m_position, 1);

	m_futureMeshes.push_back(
		std::future<OctreeMesh*>(std::async(std::launch::async,
			CreateNewMeshTask,
			rootChildren[oldCornerIdx],
			rootChildren[newCornerIdx],
			m_size / 2, // <- hnngh
			dir,
			m_position,
			m_gl_program))
	);
}

void OctreeMesh::Enlarge(Direction dir)
{
	// this line doesn't work for some reason? these sorta tuples work elsewhere in the 
	// codebase. "'_This &&' differs in levels of indirection from 'int' ..."
	auto [newCornerIdx, oldCornerIdx, newPosition] = EnlargeCorners(dir);

	std::array<Octree*, 8> rootChildren = {};
	for (auto& child : rootChildren)
	{
		child = nullptr;
	}

	rootChildren[oldCornerIdx] = m_tree;
	rootChildren[newCornerIdx] = new Octree(1, m_size, newPosition);
	rootChildren[newCornerIdx]->Construct();
	rootChildren[newCornerIdx]->MeshFromOctree(m_indices, m_vertices);

	// uncomment to see what seam generation is doing
	 // m_indices.clear();
	 // m_vertices.clear();

	BuildSeam(*rootChildren[oldCornerIdx], *rootChildren[newCornerIdx], dir, m_vertices, m_indices);

	UploadData();

	std::unique_ptr<OctreeChildren> newRootChildren(new OctreeChildren
	{
		(uint8_t)((1 << oldCornerIdx) | (1 << newCornerIdx)),
		rootChildren
	});

	m_size *= 2;
	m_tree = new Octree(std::move(newRootChildren), m_size, m_position, 1);

	m_visualization.Build(m_tree);
}

void OctreeMesh::Draw(const float time)
{
	// Draw octree viz
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // set to use wireframe
	m_visualization.DrawVisualization(time);

	// Draw actual mesh
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // set to GL_LINE to disable wireframe
	Mesh::Draw(time);

	std::lock_guard<std::mutex> guard(m_childMeshMutex);
	for (auto& mesh : m_childMeshes)
	{
		mesh->Draw(time);
	}
}

// TODO lookup table
std::tuple<int, int, glm::vec3> OctreeMesh::EnlargeCorners(Direction dir)
{
	int newCornerIdx, oldCornerIdx;
	glm::vec3 offset, newPosition;
	switch (dir)
	{
	case xplus:
		newCornerIdx = index(1, 0, 0, 2); 
		oldCornerIdx = index(0, 0, 0, 2); 
		offset = glm::vec3(m_size, 0, 0);
		newPosition = m_position + offset;
		break;
	case yplus:
		newCornerIdx = index(0, 1, 0, 2); 
		oldCornerIdx = index(0, 0, 0, 2); 
		offset = glm::vec3(0, m_size, 0);
		newPosition = m_position + offset;
		break;
	case zplus:
		newCornerIdx = index(0, 0, 1, 2); 
		oldCornerIdx = index(0, 0, 0, 2); 
		offset = glm::vec3(0, 0, m_size);
		newPosition = m_position + offset;
		break;
	case xminus:
		newCornerIdx = index(0, 0, 0, 2);
		oldCornerIdx = index(1, 0, 0, 2);
		offset = glm::vec3(-m_size, 0, 0);
		newPosition = m_position + offset;
		m_position += offset;
		break;
	case yminus:
		newCornerIdx = index(0, 0, 0, 2);
		oldCornerIdx = index(0, 1, 0, 2); 
		offset = glm::vec3(0, -m_size, 0);
		newPosition = m_position + offset;
		m_position += offset;
		break;
	case zminus:
		newCornerIdx = index(0, 0, 0, 2);
		oldCornerIdx = index(0, 0, 1, 2); 
		offset = glm::vec3(0, 0, -m_size);
		newPosition = m_position + offset;
		m_position += offset;
		break;
	}
	return std::tuple<int, int, glm::vec3>(newCornerIdx, oldCornerIdx, newPosition);
}
