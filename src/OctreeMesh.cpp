#include <glm/glm.hpp> // For glm::ivec3
#include <vector> // For std::vector
#include <queue>  // For std::queue
#include <mutex>  // For std::mutex
#include <thread> // For std::thread
#include <memory>

#include "OctreeMesh.h"
#include "utils.h"
#include "OctreeUtil.h"

OctreeMesh::OctreeMesh(GLuint program, const int size, const glm::vec3 position, const uint16_t chunkSize) 
	: m_size(size), m_position(position), Mesh(program), m_visualization(program), m_chunkSize(chunkSize)
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

// Returns the offset vector for a direction (where the neighbor is relative to the node)
glm::ivec3 directionOffset(Direction dir, int nodeSize)
{
	switch (dir)
	{
	case xplus:  return glm::ivec3(-nodeSize, 0, 0); // neighbor is at -X
	case xminus: return glm::ivec3(nodeSize, 0, 0);  // neighbor is at +X
	case yplus:  return glm::ivec3(0, -nodeSize, 0); // neighbor is at -Y
	case yminus: return glm::ivec3(0, nodeSize, 0);  // neighbor is at +Y
	case zplus:  return glm::ivec3(0, 0, -nodeSize); // neighbor is at -Z
	case zminus: return glm::ivec3(0, 0, nodeSize);  // neighbor is at +Z
	default:     return glm::ivec3(0, 0, 0);
	}
}

// Check if two directions are perpendicular
bool arePerpendicular(Direction d1, Direction d2)
{
	// X directions (xplus, xminus) are perpendicular to Y (yplus, yminus) and Z (zplus, zminus)
	int axis1 = (d1 == xplus || d1 == xminus) ? 0 : (d1 == yplus || d1 == yminus) ? 1 : 2;
	int axis2 = (d2 == xplus || d2 == xminus) ? 0 : (d2 == yplus || d2 == yminus) ? 1 : 2;
	return axis1 != axis2;
}

// Get the axis for a direction (0=X, 1=Y, 2=Z)
int directionAxis(Direction dir)
{
	if (dir == xplus || dir == xminus) return 0;
	if (dir == yplus || dir == yminus) return 1;
	return 2;
}

// Check if direction is positive
bool isPositiveDirection(Direction dir)
{
	return dir == xminus || dir == yminus || dir == zminus; // neighbor at +X/+Y/+Z
}

// Build edge seam between 4 chunks that share an edge
// This handles the "green box" case where perpendicular face seams meet
void BuildEdgeSeam(Octree* newChunk, Octree* neighbor1, Octree* neighbor2, Octree* diagonal,
	Direction dir1, Direction dir2, VertexBuffer& vertices, IndexBuffer& indices)
{
	if (!newChunk || !neighbor1 || !neighbor2 || !diagonal)
	{
		std::cout << "Skipping edge seam - missing chunk(s)" << std::endl;
		return;
	}

	// Generate vertex indices for diagonal neighbor (face seams already generated for others)
	Octree::GenerateVertexIndices(diagonal, vertices);

	// Determine which EdgeProc to call based on the perpendicular directions
	int axis1 = directionAxis(dir1);
	int axis2 = directionAxis(dir2);

	// The edge is aligned with the third axis (not axis1 or axis2)
	// EdgeProcXY handles Z-aligned edges (when we have X and Y neighbors)
	// EdgeProcXZ handles Y-aligned edges (when we have X and Z neighbors)
	// EdgeProcYZ handles X-aligned edges (when we have Y and Z neighbors)

	// Determine node ordering based on their positions
	// For EdgeProc, nodes are ordered by position in the plane perpendicular to the edge:
	// n0 at (min, min), n1 at (max, min), n2 at (min, max), n3 at (max, max)

	bool pos1 = isPositiveDirection(dir1); // neighbor1 at positive side
	bool pos2 = isPositiveDirection(dir2); // neighbor2 at positive side

	Octree* n0 = nullptr;
	Octree* n1 = nullptr;
	Octree* n2 = nullptr;
	Octree* n3 = nullptr;

	// Map chunks to n0-n3 based on their relative positions
	// The quadrant mapping depends on which axes we're dealing with
	if (axis1 == 0 && axis2 == 1) // X + Y neighbors -> Z-aligned edge (EdgeProcXY)
	{
		// For XY plane: n0=(-X,-Y), n1=(+X,-Y), n2=(-X,+Y), n3=(+X,+Y)
		if (!pos1 && !pos2) { // neighbors at -X, -Y
			n0 = diagonal; n1 = neighbor2; n2 = neighbor1; n3 = newChunk;
		} else if (pos1 && !pos2) { // neighbors at +X, -Y
			n0 = neighbor2; n1 = diagonal; n2 = newChunk; n3 = neighbor1;
		} else if (!pos1 && pos2) { // neighbors at -X, +Y
			n0 = neighbor1; n1 = newChunk; n2 = diagonal; n3 = neighbor2;
		} else { // neighbors at +X, +Y
			n0 = newChunk; n1 = neighbor1; n2 = neighbor2; n3 = diagonal;
		}
		std::cout << "Processing XY edge seam (Z-aligned)" << std::endl;
		Octree::EdgeProcXY(n0, n1, n2, n3, indices);
	}
	else if (axis1 == 0 && axis2 == 2) // X + Z neighbors -> Y-aligned edge (EdgeProcXZ)
	{
		// For XZ plane: n0=(-X,-Z), n1=(+X,-Z), n2=(-X,+Z), n3=(+X,+Z)
		if (!pos1 && !pos2) { // neighbors at -X, -Z
			n0 = diagonal; n1 = neighbor2; n2 = neighbor1; n3 = newChunk;
		} else if (pos1 && !pos2) { // neighbors at +X, -Z
			n0 = neighbor2; n1 = diagonal; n2 = newChunk; n3 = neighbor1;
		} else if (!pos1 && pos2) { // neighbors at -X, +Z
			n0 = neighbor1; n1 = newChunk; n2 = diagonal; n3 = neighbor2;
		} else { // neighbors at +X, +Z
			n0 = newChunk; n1 = neighbor1; n2 = neighbor2; n3 = diagonal;
		}
		std::cout << "Processing XZ edge seam (Y-aligned)" << std::endl;
		Octree::EdgeProcXZ(n0, n1, n2, n3, indices);
	}
	else if (axis1 == 1 && axis2 == 2) // Y + Z neighbors -> X-aligned edge (EdgeProcYZ)
	{
		// For YZ plane: n0=(-Y,-Z), n1=(+Y,-Z), n2=(-Y,+Z), n3=(+Y,+Z)
		if (!pos1 && !pos2) { // neighbors at -Y, -Z
			n0 = diagonal; n1 = neighbor2; n2 = neighbor1; n3 = newChunk;
		} else if (pos1 && !pos2) { // neighbors at +Y, -Z
			n0 = neighbor2; n1 = diagonal; n2 = newChunk; n3 = neighbor1;
		} else if (!pos1 && pos2) { // neighbors at -Y, +Z
			n0 = neighbor1; n1 = newChunk; n2 = diagonal; n3 = neighbor2;
		} else { // neighbors at +Y, +Z
			n0 = newChunk; n1 = neighbor1; n2 = neighbor2; n3 = diagonal;
		}
		std::cout << "Processing YZ edge seam (X-aligned)" << std::endl;
		Octree::EdgeProcYZ(n0, n1, n2, n3, indices);
	}
	else if (axis1 == 1 && axis2 == 0) // Same as axis1=0, axis2=1 but swapped
	{
		std::swap(neighbor1, neighbor2);
		std::swap(pos1, pos2);
		if (!pos1 && !pos2) {
			n0 = diagonal; n1 = neighbor2; n2 = neighbor1; n3 = newChunk;
		} else if (pos1 && !pos2) {
			n0 = neighbor2; n1 = diagonal; n2 = newChunk; n3 = neighbor1;
		} else if (!pos1 && pos2) {
			n0 = neighbor1; n1 = newChunk; n2 = diagonal; n3 = neighbor2;
		} else {
			n0 = newChunk; n1 = neighbor1; n2 = neighbor2; n3 = diagonal;
		}
		std::cout << "Processing XY edge seam (Z-aligned) [swapped]" << std::endl;
		Octree::EdgeProcXY(n0, n1, n2, n3, indices);
	}
	else if (axis1 == 2 && axis2 == 0) // Same as axis1=0, axis2=2 but swapped
	{
		std::swap(neighbor1, neighbor2);
		std::swap(pos1, pos2);
		if (!pos1 && !pos2) {
			n0 = diagonal; n1 = neighbor2; n2 = neighbor1; n3 = newChunk;
		} else if (pos1 && !pos2) {
			n0 = neighbor2; n1 = diagonal; n2 = newChunk; n3 = neighbor1;
		} else if (!pos1 && pos2) {
			n0 = neighbor1; n1 = newChunk; n2 = diagonal; n3 = neighbor2;
		} else {
			n0 = newChunk; n1 = neighbor1; n2 = neighbor2; n3 = diagonal;
		}
		std::cout << "Processing XZ edge seam (Y-aligned) [swapped]" << std::endl;
		Octree::EdgeProcXZ(n0, n1, n2, n3, indices);
	}
	else if (axis1 == 2 && axis2 == 1) // Same as axis1=1, axis2=2 but swapped
	{
		std::swap(neighbor1, neighbor2);
		std::swap(pos1, pos2);
		if (!pos1 && !pos2) {
			n0 = diagonal; n1 = neighbor2; n2 = neighbor1; n3 = newChunk;
		} else if (pos1 && !pos2) {
			n0 = neighbor2; n1 = diagonal; n2 = newChunk; n3 = neighbor1;
		} else if (!pos1 && pos2) {
			n0 = neighbor1; n1 = newChunk; n2 = diagonal; n3 = neighbor2;
		} else {
			n0 = newChunk; n1 = neighbor1; n2 = neighbor2; n3 = diagonal;
		}
		std::cout << "Processing YZ edge seam (X-aligned) [swapped]" << std::endl;
		Octree::EdgeProcYZ(n0, n1, n2, n3, indices);
	}
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

OctreeMesh* CreateNewMeshWithNeighboursTask(std::vector<std::pair<Octree*, Direction>> neighbours, Octree* newOctree,
	const int size, const glm::vec3 position, GLuint program, Octree* rootNode, int chunkSize)
{
	VertexBuffer vertices;
	IndexBuffer indices;

	newOctree->Construct();
	newOctree->MeshFromOctree(indices, vertices);

	// Process face seams with direct neighbors
	for (const auto& neighbourDir : neighbours)
	{
		BuildSeam(*neighbourDir.first, *newOctree, neighbourDir.second, vertices, indices);
	}

	// Process edge seams for perpendicular neighbor pairs (the "green box" case)
	// For each pair of perpendicular neighbors, find the diagonal and process the edge seam
	for (size_t i = 0; i < neighbours.size(); i++)
	{
		for (size_t j = i + 1; j < neighbours.size(); j++)
		{
			Direction dir1 = neighbours[i].second;
			Direction dir2 = neighbours[j].second;

			if (!arePerpendicular(dir1, dir2))
				continue;

			Octree* neighbor1 = neighbours[i].first;
			Octree* neighbor2 = neighbours[j].first;

			// Calculate diagonal position: newOctree position + offset from dir1 + offset from dir2
			glm::ivec3 newChunkPos = newOctree->m_min;
			glm::ivec3 diagonalPos = newChunkPos + directionOffset(dir1, chunkSize) + directionOffset(dir2, chunkSize);

			// Find the diagonal neighbor
			Octree* diagonal = FindNode(rootNode, diagonalPos, chunkSize);

			std::cout << "Edge seam: looking for diagonal at (" << diagonalPos.x << ", " << diagonalPos.y << ", " << diagonalPos.z << ")" << std::endl;

			if (diagonal)
			{
				std::cout << "Found diagonal neighbor, processing edge seam" << std::endl;
				BuildEdgeSeam(newOctree, neighbor1, neighbor2, diagonal, dir1, dir2, vertices, indices);
			}
			else
			{
				std::cout << "Diagonal neighbor not found, skipping edge seam" << std::endl;
			}
		}
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

glm::ivec3 OctreeMesh::AddNewChunk(glm::ivec3 newPosition, uint16_t chunkSize)
{
	// 0) check if chunk already exists
	Octree* node = FindNode(m_tree, newPosition, chunkSize);
	if (node)
	{
		// print that it already exists along with coordinates
		printf("Chunk already exists at (%i, %i, %i) with size (%i) \n", newPosition.x, newPosition.y, newPosition.z, chunkSize);
		return newPosition;
	}


	// 1) check if needs expansion
    Direction expandDir = expansionDirection(m_tree, newPosition, chunkSize);
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
	Octree* newNode = Octree::CreateNewSubTree(m_tree, newPosition, chunkSize); 


	// 4) find neighbours of the new node (which may have different parents so this needs to be a recursive algorithm)
	std::vector<std::pair<Octree*, Direction>> neighbours = FindNeighbors(m_tree, newPosition, chunkSize);

	// 5) process new node
	//		a) just run the Construct function on the new node as usual
	//		a) implement algorithm that moves down from the root node, use the index(x,y,z,2) function to find what child to visit for each level
	// 6) process seams by looping through the neighbours 
	m_futureMeshes.push_back(
		std::future<OctreeMesh*>(std::async(std::launch::async,
			CreateNewMeshWithNeighboursTask,
			std::move(neighbours),
			newNode,
			static_cast<int>(chunkSize),
			m_position,
			m_gl_program,
			m_tree,
			static_cast<int>(chunkSize)))
	);

	return newPosition;
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

	return AddNewChunk(chunkCursor, chunkSize);
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

void OctreeMesh::SpiralGenerate(int width, int height, bool generateColumns) {
    // Initialize the grid size
    int gridSize = 2 * width + 1;

    // Initialize directions: right, down, left, up (in 2D plane)
    std::vector<glm::ivec3> directions = {
        glm::ivec3(0, 0, m_chunkSize),  // right
        glm::ivec3(m_chunkSize, 0, 0),  // down
        glm::ivec3(0, 0, -m_chunkSize), // left
        glm::ivec3(-m_chunkSize, 0, 0)  // up
    };

    glm::ivec3 basePosition(0, 0, 0); // Centered at (0, 0, 0)
    int directionIndex = 0;
    int step = 1, change = 0;
    int count = 0;

    auto addPositionToQueue = [&](glm::ivec3 pos) {
        std::lock_guard<std::mutex> lock(queueMutex);
        chunkQueue.push(pos);
    };

    if (generateColumns) {
        // Loop over x, z and generate entire column for each x, z position
        for (int i = 1; i <= gridSize * gridSize; ++i) {
            for (int y = height / 2; y > -height / 2; --y) {
                glm::ivec3 position = basePosition + glm::ivec3(0, y * m_chunkSize, 0);
                addPositionToQueue(position);
            }

            if (count == step) {
                directionIndex = (directionIndex + 1) % 4;
                if (directionIndex % 2 == 0) {
                    step++;
                }
                count = 0;
            }

            basePosition += directions[directionIndex];
            count++;
        }
    } else {
        // Loop over each y level and generate the spiral for each y level
        for (int y = -height / 2; y < height / 2; y++) {
            glm::ivec3 levelBasePosition(0, y * m_chunkSize, 0);
            basePosition = levelBasePosition;
            directionIndex = 0;
            step = 1;
            count = 0;

            for (int i = 1; i <= gridSize * gridSize; ++i) {
                glm::ivec3 position = basePosition;
                addPositionToQueue(position);

                if (count == step) {
                    directionIndex = (directionIndex + 1) % 4;
                    if (directionIndex % 2 == 0) {
                        step++;
                    }
                    count = 0;
                }

                basePosition += directions[directionIndex];
                count++;
            }
        }
    }

    // Start processing the queue
    std::thread(&OctreeMesh::ProcessQueue, this).detach();
}



void OctreeMesh::ProcessQueue()
{
	while (true)
	{
		glm::ivec3 currentPosition;
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (chunkQueue.empty())
			{
				break;
			}
			currentPosition = chunkQueue.front();
			chunkQueue.pop();
		}

		// Ensure thread safety while calling AddNewChunk
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			AddNewChunk(currentPosition, m_chunkSize);
		}

		// Wait for current mesh tasks to complete before proceeding
		WaitForMeshCompletion();
	}
}

void OctreeMesh::WaitForMeshCompletion()
{
	while (true)
	{
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (m_futureMeshes.empty())
			{
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Sleep to avoid busy-waiting
	}
}