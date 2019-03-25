//
// Created by Teemu Sarapisto on 20/07/2017.
//
#include <array>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <memory>

#include "Octree.h"
#include "utils.h"

const glm::vec3 CHILD_MIN_OFFSETS[] =
{
        glm::vec3(0,0,0),
        glm::vec3(0,0,1),
        glm::vec3(0,1,0),
        glm::vec3(0,1,1),
        glm::vec3(1,0,0),
        glm::vec3(1,0,1),
        glm::vec3(1,1,0),
        glm::vec3(1,1,1),
};

Octree::~Octree()
{
    /////// NYI
    printf("destroying objects");

}


// Used to initialize with children
Octree::Octree(std::unique_ptr<OctreeChildren> children, int size, glm::vec3 min, int resolution)
: m_children(std::move(children)), m_resolution(resolution), m_size(size), m_min(min)
{
    printf("with children min (%f, %f, %f), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    //printf("min (%f, %f, %f), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    //std::cout << "lapsoset" <<  m_children.get() << std::endl;
    printBinary(m_children->field);
}


// Called when creating a completely new octree from scratch
Octree::Octree(const int resolution, const int size, const glm::vec3 min)
    : m_resolution(resolution), m_size(size), m_min(min), m_children(nullptr)
{
    printf("new octree min (%f, %f, %f), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    /*
	if (resolution == size) {
        std::cout << "creating a leaf" << std::endl;
		ConstructLeaf(resolution, min);
	}
	else
	{
        std::cout << "constructing from bottom up" << std::endl;
		ConstructBottomUp(resolution, size, min);
	}
    */
    std::cout << "constructing from bottom up" << std::endl;
    ConstructBottomUp(resolution, size, min);
}

// Leaf node constructor
Octree::Octree(const int resolution, glm::vec3 min)
    : m_resolution(resolution), m_size(resolution), m_min(min), m_children(nullptr), m_leaf(true)
{
    m_children = std::unique_ptr<OctreeChildren>(nullptr);
    printf("leaf node min (%f, %f, %f), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    // NYI
    // Create leaf node draw data here.
    //std::cout << "creating leaf node at resolution: " << resolution << std::endl;
    //std::cout << "and min: " <<  min.x << min.y << min.z << std::endl;
}



int index(int x, int y, int z, int dimensionLength)
{
	return x + dimensionLength * (y + dimensionLength * z);
}

bool Octree::HasSomethingToRender()
{
    //NYI
    return true;
}

bool Octree::Sample(const glm::vec3 pos)
{
    return pos.x == 0 ? true : false;
}

Octree* Octree::ConstructLeaf(const int resolution, const glm::vec3 min)
{
    printf("construct leaf min (%f, %f, %f)\n", min.x, min.y, min.z);
    uint8_t field = 0;
    std::array<Octree*, 8> children = {};
    for(float x = 0; x < 2; x++)
    {
        for(float y = 0; y < 2; y++)
        {
            for(float z = 0; z < 2; z++)
            {
                glm::ivec3 offset = glm::ivec3(x, y, z);
                int cornerIdx = index(offset.x % 2, offset.y % 2, offset.z % 2, 2);
                bool solid = Sample(glm::ivec3(min.x + offset.x, min.y + offset.y, min.z + offset.z));
                if (solid)
                {
                    std::cout << "is solid" << std::endl;
                    field |= (1 << cornerIdx);
                    glm::vec3 childPos = glm::vec3(x + min.x, y + min.y, z + min.z) * (float)resolution;

					// TODO: micro opt: maybe do new Octree :ing for each child only after checking field != 0xFF
                    children[cornerIdx] = new Octree(resolution, childPos);
                }
            }
        }
    }

    //printBinary(field);
    if (field > 0) 
    {
		// TODO: maybe check here field = 0xFF aka all children exist so we can possibly
		// approximate all children with a single vertex. Check QEFs. Set m_leaf'ness.

        std::cout << "field is over 0" << std::endl;
        auto c = std::unique_ptr<OctreeChildren>(new OctreeChildren());
        c->field = field;
        //printBinary(c->field);
        c->children = children;
        Octree* o = new Octree(std::move(c), resolution * 2, min, resolution);
        return o;
    }
    else
    {
        //std::cout << "not over 0 " << field << std::endl;
    }

    return nullptr;
}

void Octree::ConstructBottomUp(const int resolution, const int size, const glm::vec3 min)
{
    // First loop through the area in 1x1x1 cubes, then 2x2x2, etc.

    int cubeSize = resolution * 2;
    int childCountPerAxis = size / cubeSize;
    int parentCountPerAxis = childCountPerAxis / 2;
    int childCount = pow(childCountPerAxis, 3);
    int parentCount = childCount / 8;
    std::vector<std::unique_ptr<OctreeChildren>> currentSizeNodes(childCount);
    std::vector<std::unique_ptr<OctreeChildren>> parentSizeNodes(parentCount);
    for(int i = 0; i < currentSizeNodes.size(); i++)
    {
        currentSizeNodes[i] = nullptr;
    }
    for(int i = 0; i < parentSizeNodes.size(); i++)
    {
        parentSizeNodes[i] = nullptr;
    }
    while (cubeSize != size)
    {
        std::cout << "cube size" << cubeSize << std::endl;
        for (int x = 0; x < size; x += cubeSize)
        {
            for (int y = 0; y < size; y += cubeSize)
            {
                for (int z = 0; z < size; z += cubeSize)
                {
                    std::cout << "ses x, y, z: " << x << y << z << std::endl;
                    printf("sos xyz (%i, %i, %i)\n", x, y, z);

                    const int childIdxX = x / cubeSize;
                    const int childIdxY = y / cubeSize;
                    const int childIdxZ = z / cubeSize;
                    const int parentIdxX = childIdxX / 2;
                    const int parentIdxY = childIdxY / 2;
                    const int parentIdxZ = childIdxZ / 2;
                    const int parentIdx = index(parentIdxX, parentIdxY, parentIdxZ, parentCountPerAxis);
                    const int childIdx = index(childIdxX, childIdxY, childIdxZ, size / cubeSize);
                    const int cornerIdx = index(childIdxX % 2, childIdxY % 2, childIdxZ % 2, 2);
                    const glm::vec3 pos = min + glm::vec3(x, y, z);
                    Octree* node = nullptr;
                    if (cubeSize == resolution * 2)
                    {
                        node = Octree::ConstructLeaf(resolution, pos); // null if nothing to draw
                        if (node)
                        {
                            if (!parentSizeNodes[parentIdx])
                            {
                                std::array<Octree*, 8> children = {};
                                for(auto& child : children)
                                {
                                    child = nullptr;
                                }
                                children[cornerIdx] = node;
                                printf("node min stuff at constructbottomup (%f, %f, %f)\n", node->m_min.x, node->m_min.y, node->m_min.z);
                                parentSizeNodes[parentIdx] = std::unique_ptr<OctreeChildren>(new OctreeChildren
                                {
                                    (uint8_t)(1 << cornerIdx),
                                    children
                                });
                            }
                            else // parent list already created
                            {
                                parentSizeNodes[parentIdx]->field |= (1 << cornerIdx);
                                parentSizeNodes[parentIdx]->children[cornerIdx] = node;
                            }
                        }
                        else // node is null
                        {
                            std::cout << "no stuff in node" << std::endl;
                            // TODO: really do nothing?
                        }
                    }
                    else // cubesize > resolution * 2
                    {
                        // is a container element, should just contain previously created children, if any
                        // check if this node has children
                        if(currentSizeNodes[childIdx]) // 
                        {
                            //std::cout << "has children" << std::endl;
                            // Create a new octree node whose children are the ones created in previous
                            // loop iterations parents that are now currentSized
                            node = new Octree(std::move(currentSizeNodes[childIdx]), cubeSize, pos, resolution);

                            //////////////////////////////////////////////////////////// This is the same as lines 180 ->
                            if (!parentSizeNodes[parentIdx])
                            {
                                //std::cout << "parent array not created" << std::endl;
                                std::array<Octree*, 8> children = {};
                                children[cornerIdx] = node;
                                parentSizeNodes[parentIdx] = std::unique_ptr<OctreeChildren>(new OctreeChildren
                                {
                                    (uint8_t)(1 << cornerIdx),
                                    children
                                });
                            }
                            else // parent list already created
                            {
                                //std::cout << "parent array already created" << std::endl;
                                parentSizeNodes[parentIdx]->field |= (1 << cornerIdx);
                                parentSizeNodes[parentIdx]->children[cornerIdx] = node;
                            }
                            //////////////////////////////////////////////////////////// end copypasta from 180
                        }
                        else
                        {
                            //std::cout << "no children" << std::endl;
                        }
                    }
                }
            }
        }
        cubeSize *= 2;
        childCountPerAxis = size / cubeSize;
        parentCountPerAxis = childCountPerAxis / 2;
        childCount = pow(childCountPerAxis, 3);
        parentCount = childCount / 8;


        std::cout << "childCount" << childCount << std::endl;
        std::cout << "parentCount" << parentCount << std::endl;

        //currentSizeNodes = std::move(parentSizeNodes);
        currentSizeNodes = std::vector<std::unique_ptr<OctreeChildren>>();
        for(auto & node : parentSizeNodes)
        {
            currentSizeNodes.push_back(std::move(node));
        }
        parentSizeNodes = std::vector<std::unique_ptr<OctreeChildren>>(parentCount);
        for(int i = 0; i < parentSizeNodes.size(); i++)
        {
            parentSizeNodes[i] = nullptr;
        }
    }
    
    // currentSizeNodes length should be 1 now.
    m_children = std::move(currentSizeNodes[0]);
}

OctreeChildren* Octree::GetChildren() const
{
	//std::cout << "------------------------------------" << std::endl;
    //std::cout << "printtibusiness" << std::endl;
    //std::cout << m_size << std::endl;
    //std::cout << "exists" << (m_children ? "true" : "false") << std::endl;
    if(!m_children)
    {
        return nullptr;
    }
    //std::cout << m_children.get() << std::endl;
    //printBinary(m_children.get()->field);
    //printf("min (%f, %f, %f), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    //std::cout << "lapsoset" <<  m_children.get() << std::endl;
    //std::cout << "------------------------------------" << std::endl;
    return m_children.get() ? m_children.get() : nullptr;
}

void Octree::MeshFromOctree()
{
	// TODO: Give vertex buffer as a parameter to generatevertexindices
	//std::vector<int> vertexBuffer;
	GenerateVertexIndices();

	// TODO: give index buffer as parameter
	CellProc();
}

void Octree::GenerateVertexIndices()
{
	// Go through all leaf nodes, create a vertex
	// Add vertex index info to node so it can be accessed in CellProc

	// Mebbe just create the vertices in the middle of the node at first,
	// work with sharper features etc. later?
}

// indices 
// 0 = 0,0,0 = 0 + 2 * (0 + 2 * 0)
// 1 = 1,0,0 = 1 + 2 * (0 + 2 * 0)

// 2 = 0,1,0 = 0 + 2 * (1 + 2 * 0)
// 3 = 1,1,0 = 1 + 2 * (1 + 2 * 0)

// 4 = 0,0,1 = 0 + 2 * (0 + 2 * 1)
// 5 = 1,0,1 = 1 + 2 * (0 + 2 * 1)

// 6 = 0,1,1 = 0 + 2 * (1 + 2 * 1)
// 7 = 1,1,1 = 1 + 2 * (1 + 2 * 1)

// Faces:
// X: [0, 1], [4, 5], [2, 3], [6, 7]
// Y: [0, 2], [1, 3], [4, 6], [5, 7]
// Z: [0, 4], [1, 5], [2, 6], [3, 7]

// Edges:
// XY: [0, 1, 2, 3], [4, 5, 6, 7]
// XZ: [0, 1, 4, 5], [2, 3, 6, 7]
// YZ: [0, 2, 4, 6], [1, 3, 5, 7]

// XY/XZ/YZ means the cubes are ordered in that way, eg for XZ the cubes are on the bottom (0 1 4 5) and top four (2 3 6 7)

void Octree::CellProc()
{
	if (!m_leaf)
	{
		const auto& children = m_children->children;

		// Cells
		for (int i = 0; i < 8; i++)
		{
			if (children[i])
			{
				children[i]->CellProc();
			}
		}

		// Faces
		FaceProcX(*children[0], *children[1]);
		FaceProcX(*children[4], *children[5]);
		FaceProcX(*children[2], *children[3]);
		FaceProcX(*children[6], *children[7]);

		FaceProcY(*children[0], *children[2]);
		FaceProcY(*children[1], *children[3]);
		FaceProcY(*children[4], *children[6]);
		FaceProcY(*children[5], *children[7]);

		FaceProcZ(*children[0], *children[4]);
		FaceProcZ(*children[1], *children[5]);
		FaceProcZ(*children[2], *children[6]);
		FaceProcZ(*children[3], *children[7]);

		// Edges
		EdgeProcXY(*children[0], *children[1], *children[2], *children[3]);
		EdgeProcXY(*children[4], *children[5], *children[6], *children[7]);

		EdgeProcXZ(*children[0], *children[1], *children[4], *children[5]);
		EdgeProcXZ(*children[2], *children[3], *children[6], *children[7]);

		EdgeProcYZ(*children[0], *children[2], *children[4], *children[6]);
		EdgeProcYZ(*children[1], *children[3], *children[5], *children[7]);
	}
	
	// TODO else do nothing?
	//...? todo checking if already leaf node, if so, pass current node instead of some child
}

void Octree::EdgeProcXY(const Octree& n0, const Octree& n1, const Octree& n2, const Octree& n3)
{
	//todo checking if already leaf node, if so, pass current node instead of some child
	if (n0.m_leaf && n1.m_leaf && n2.m_leaf && n3.m_leaf)
	{
		return ProcessEdge(n0, n1, n2, n3);
	}
	
	EdgeProcXY(*n0.m_children->children[3], *n1.m_children->children[2], *n2.m_children->children[1], *n3.m_children->children[0]);
	EdgeProcXY(*n0.m_children->children[7], *n1.m_children->children[6], *n2.m_children->children[5], *n3.m_children->children[4]);
}
void Octree::EdgeProcXZ(const Octree& n0, const Octree& n1, const Octree& n2, const Octree& n3)
{
	//todo checking if already leaf node, if so, pass current node instead of some child
	if (n0.m_leaf && n1.m_leaf && n2.m_leaf && n3.m_leaf)
	{
		return ProcessEdge(n0, n1, n2, n3);
	}

	EdgeProcXZ(*n0.m_children->children[5], *n1.m_children->children[4], *n2.m_children->children[1], *n3.m_children->children[0]);
	EdgeProcXZ(*n0.m_children->children[7], *n1.m_children->children[6], *n2.m_children->children[3], *n3.m_children->children[2]);
}
void Octree::EdgeProcYZ(const Octree& n0, const Octree& n1, const Octree& n2, const Octree& n3)
{
	//todo checking if already leaf node, if so, pass current node instead of some child
	if (n0.m_leaf && n1.m_leaf && n2.m_leaf && n3.m_leaf)
	{
		return ProcessEdge(n0, n1, n2, n3);
	}

	EdgeProcYZ(*n0.m_children->children[6], *n1.m_children->children[4], *n2.m_children->children[2], *n3.m_children->children[0]);
	EdgeProcYZ(*n0.m_children->children[7], *n1.m_children->children[5], *n2.m_children->children[3], *n3.m_children->children[1]);
}

// Faces spawn four calls to faceproc, 
// and four calls to edgeproc
void Octree::FaceProcX(const Octree& n0, const Octree& n1)
{
	//todo checking if already leaf node, if so, pass current node instead of some child
	FaceProcX(*n0.m_children->children[1], *n1.m_children->children[0]);
	FaceProcX(*n0.m_children->children[3], *n1.m_children->children[2]);
	FaceProcX(*n0.m_children->children[5], *n1.m_children->children[4]);
	FaceProcX(*n0.m_children->children[7], *n1.m_children->children[6]);

	EdgeProcXY(*n0.m_children->children[1], *n1.m_children->children[0], *n0.m_children->children[3], *n1.m_children->children[2]);
	EdgeProcXY(*n0.m_children->children[5], *n1.m_children->children[4], *n0.m_children->children[7], *n1.m_children->children[6]);

	EdgeProcXZ(*n0.m_children->children[3], *n1.m_children->children[2], *n0.m_children->children[7], *n1.m_children->children[6]);
	EdgeProcXZ(*n0.m_children->children[1], *n1.m_children->children[0], *n0.m_children->children[5], *n1.m_children->children[4]);
}
void Octree::FaceProcY(const Octree& n0, const Octree& n1)
{
	//todo checking if already leaf node, if so, pass current node instead of some child
	FaceProcX(*n0.m_children->children[2], *n1.m_children->children[0]);
	FaceProcX(*n0.m_children->children[1], *n1.m_children->children[1]);
	FaceProcX(*n0.m_children->children[6], *n1.m_children->children[4]);
	FaceProcX(*n0.m_children->children[7], *n1.m_children->children[5]);

	// todo edgeprocs
}
void Octree::FaceProcZ(const Octree& n0, const Octree& n1)
{
	//todo checking if already leaf node, if so, pass current node instead of some child
	FaceProcX(*n0.m_children->children[4], *n1.m_children->children[0]);
	FaceProcX(*n0.m_children->children[5], *n1.m_children->children[1]);
	FaceProcX(*n0.m_children->children[6], *n1.m_children->children[2]);
	FaceProcX(*n0.m_children->children[7], *n1.m_children->children[3]);

	// todo edgeprocs
}

void Octree::ProcessEdge(const Octree&, const Octree&, const Octree&, const Octree&)
{
}