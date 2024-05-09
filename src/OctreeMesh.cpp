#include "OctreeMesh.h"
#include "utils.h"
#include "OctreeUtil.h"

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
		auto childrenPtr = node.GetChildren();
		if (!childrenPtr)
		{
			continue;
		}
		std::array<Octree*, 8> children = childrenPtr->children;
		for (int i = 0; i < 4; i++)
		{
			auto[x, y, z, edgeIdx] = edgeIndexLookup[dir][j][i];
			int idx = octreeIndex(x, y, z);
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
	std::cout << "-------------------------------- Processing seam edges --------------------------------" << std::endl;
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

OctreeMesh* CreateNewMeshWithNeighboursTask(std::vector<Octree*>& neighbours, Octree* newOctree,
	const int size, Direction dir, const glm::vec3 position, GLuint program)
{
	VertexBuffer vertices;
	IndexBuffer indices;

	newOctree->Construct();
	newOctree->MeshFromOctree(indices, vertices);

	for (auto neighbour : neighbours)
	{
		BuildSeam(*neighbour, *newOctree, dir, vertices, indices);
	}

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

void OctreeMesh::Draw(const float time, uint32_t mode)
{
	// Draw octree viz
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // set to use wireframe
	//m_visualization.DrawVisualization(time);

	// enum for modes
	// 0 = fill
	// 1 = wireframe
	Mesh::Draw(time, mode);

	std::lock_guard<std::mutex> guard(m_childMeshMutex);
	for (auto& mesh : m_childMeshes)
	{
		mesh->Draw(time, mode);
	}
}

void OctreeMesh::Enlarge(Direction dir, uint16_t chunkSize)
{
    auto [newCornerIdx, oldCornerIdx, offset] = EnlargeCorners(dir);

    std::array<Octree*, 8> rootChildren = {};
    std::fill(rootChildren.begin(), rootChildren.end(), nullptr);

    rootChildren[oldCornerIdx] = m_tree;

	// in addition to the old node being placed into a new parent, 
	// we can already create a new empty node into the new parent, as we know we wanted to expand into some direction
	// The position of this new node is the current position + offset, 
	// where offset is the size of the current octree before expansion, sign adjusted for the direction of expansion
    rootChildren[newCornerIdx] = new Octree(1, m_size, m_position + offset); 
	// the other 6 children should be empty

    std::shared_ptr<OctreeChildren> newRootChildren(new OctreeChildren
    {
		// TODO: what if one of them is actually empty? should check fo dat at some point after sampling?
        (uint8_t)((1 << oldCornerIdx) | (1 << newCornerIdx)), // will contain just the old one, and the new (currently empty) one
        rootChildren
    });

    if (dir == xminus || dir == yminus || dir == zminus) {
        m_position += offset; // move the root node by offset TODO always get position from the root node instead of separately keeping track of it here
    }

    m_size *= 2;

    printf("new root octree with already made children min (%f, %f, %f), size %i\n", m_position.x, m_position.y, m_position.z, m_size);
    m_tree = new Octree(std::move(newRootChildren), m_size, m_position, 1);

	// could return the new node here, but its just the parent that will later contain the actual node we want to create,
	// we still need to find the actual chunkSize node inside it, might as well traverse from root node, 
	// probably isn't more than a few hops in the tree. This is never going to be a bottle neck I think.
}

glm::ivec3 OctreeMesh::AddNewChunk(glm::ivec3 chunkCursor, Direction dir, uint16_t chunkSize)
 {
	switch (dir)
	{
	case xplus:
		chunkCursor.x += chunkSize;
		break;
	case yplus:
		chunkCursor.y += chunkSize;
		break;
	case zplus:
		chunkCursor.z += chunkSize;
		break;
	case xminus:
		chunkCursor.x -= chunkSize;
		break;
	case yminus:
		chunkCursor.y -= chunkSize;
		break;
	case zminus:
		chunkCursor.z -= chunkSize;
		break;
	}	

	// 1) check if needs expansion
    Direction expandDir = expansionDirection(m_tree, chunkCursor, chunkSize);
    if (expandDir != Direction::nodir) {
		// 2) do expansion if needed
		//		a) this is probably handled ok by existing parts of the EnlargeAsync, just the position where the meshtask needs to run is wrong now
        Enlarge(expandDir, chunkSize);
    }
	// 3) place new node at given position. the task is to find the parent node of the new node from the root node down, 
			// and place this new node into its children in the correct index 
			// (i gave code for figuring out the index of the child assuming we know the position of the parent). 
			// I guess we should be looking for the node that is 2x chunksize in all cases, as its children will be 1x chunksize what we were looking for, no?
	// new pos is just currentChunkPos + chunkSize in the direction of the expansion

	// now we initialize the tree all the way until the new node to be meshed
	// note that this is different from above possible expansion, as there we just double the size of the root octree which may be much larger than the chunk size
	Octree* newNode = Octree::CreateNewSubTree(m_tree, chunkCursor, chunkSize); 


	// 4) find neighbours of the new node (which may have different parents so this needs to be a recursive algorithm)
	std::vector<Octree*> neighbours = FindNeighbors(m_tree, chunkCursor, chunkSize);

	// 5) process new node
	//		a) just run the Construct function on the new node as usual
	//		a) implement algorithm that moves down from the root node, use the index(x,y,z,2) function to find what child to visit for each level
	// 6) process seams by looping through the neighbours 
	m_futureMeshes.push_back(
		std::future<OctreeMesh*>(std::async(std::launch::async,
			CreateNewMeshWithNeighboursTask,
			neighbours,
			newNode,
			chunkSize,
			dir,
			m_position,
			m_gl_program))
	);

	return chunkCursor;
}

std::tuple<int, int, glm::vec3> OctreeMesh::EnlargeCorners(Direction dir)
{
    int newCornerIdx, oldCornerIdx;
    glm::vec3 offset;
    switch (dir)
    {
    case xplus:
		// If we didn't fit into the old octree, and we are moving to xplus direction
		// we create a bigger one, place the old one into `oldCornerIdx`
        newCornerIdx = index(1, 0, 0, 2); 
        oldCornerIdx = index(0, 0, 0, 2); 
        offset = glm::vec3(m_size, 0, 0); // unused so shouldn't this be zero?
        break;
    case yplus:
        newCornerIdx = index(0, 1, 0, 2); 
        oldCornerIdx = index(0, 0, 0, 2); 
        offset = glm::vec3(0, m_size, 0);
        break;
    case zplus:
        newCornerIdx = index(0, 0, 1, 2); 
        oldCornerIdx = index(0, 0, 0, 2); 
        offset = glm::vec3(0, 0, m_size);
        break;
    case xminus:
        newCornerIdx = index(0, 0, 0, 2);
        oldCornerIdx = index(1, 0, 0, 2);
        offset = glm::vec3(-m_size, 0, 0); // for negative direction the octree
        break;
    case yminus:
        newCornerIdx = index(0, 0, 0, 2);
        oldCornerIdx = index(0, 1, 0, 2); 
        offset = glm::vec3(0, -m_size, 0);
        break;
    case zminus:
        newCornerIdx = index(0, 0, 0, 2);
        oldCornerIdx = index(0, 0, 1, 2); 
        offset = glm::vec3(0, 0, -m_size);
        break;
    }
    return std::tuple<int, int, glm::vec3>(newCornerIdx, oldCornerIdx, offset);
}
