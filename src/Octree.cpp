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

const glm::ivec3 CHILD_MIN_OFFSETS[] =
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
    /////// NYI, should f.ex delete the OctreeChildren->children
    //printf("destroying Octree\n"); // helluva lot of these. TODO: fix
}


// Used to initialize with ready built children
Octree::Octree(std::unique_ptr<OctreeChildren> children, int size, glm::vec3 min, int resolution)
: m_children(std::move(children)), m_resolution(resolution), m_size(size), m_min(min), m_leaf(false)
{
    //printf("new octree with already made children min (%i, %i, %i), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    //printBinary(m_children->field);
}


// Called when creating a completely new octree from scratch
Octree::Octree(const int resolution, const int size, const glm::vec3 min)
    : m_resolution(resolution), m_size(size), m_min(min), m_children(nullptr), m_leaf(false)
{
    printf("constructing new octree at min (%i, %i, %i), size %i\n", m_min.x, m_min.y, m_min.z, m_size);
    ConstructBottomUp(resolution, size, min);
}

// Leaf node constructor
Octree::Octree(const int resolution, glm::vec3 min)
    : m_resolution(resolution), m_size(resolution), m_min(min), m_children(nullptr), m_leaf(true)
{
}

int index(int x, int y, int z, int dimensionLength)
{
	return x + dimensionLength * (y + dimensionLength * z);
}

float Sphere(const glm::vec3& worldPosition, const glm::vec3& origin, float radius)
{
	return glm::length(worldPosition - origin) - radius;
}

float Box(const glm::vec3& p, const glm::vec3& b)
{
	glm::vec3 d = abs(p) - b;
	glm::vec3 maxed(0);
	for (int i = 0; i < 3; i++)
	{
		maxed[i] = std::max(d[i], 0.f);
	}
	return glm::length(maxed) + std::min(std::max(d.x, std::max(d.y, d.z)), 0.f);
}

#include <noise/noise.h>
using namespace noise;
float Noise(const glm::vec3& p)
{
	double epsilon = 0.1;
	static module::Perlin myModule;
	float divider = 100;
	double value = myModule.GetValue(p.x / divider  + epsilon, p.y / divider + epsilon, p.z / divider + epsilon);
	//std::cout << value << std::endl;
	return value;
}

float DensityFunction(const glm::vec3 pos)
{
	return Noise(pos);
	//return Sphere(pos, glm::vec3(20, 20, 20), 16.0);
	//return Box(pos - glm::vec3(8,8,8), glm::vec3(5, 5, 5));
}

bool Sample(const glm::vec3 pos)
{
	return DensityFunction(pos) > 0;
}

glm::vec3 CalculateSurfaceNormal(const glm::vec3& p)
{
	const float epsilon = 0.001f;
	const float dx = DensityFunction(p + glm::vec3(epsilon, 0.f, 0.f)) - DensityFunction(p - glm::vec3(epsilon, 0.f, 0.f));
	const float dy = DensityFunction(p + glm::vec3(0.f, epsilon, 0.f)) - DensityFunction(p - glm::vec3(0.f, epsilon, 0.f));
	const float dz = DensityFunction(p + glm::vec3(0.f, 0.f, epsilon)) - DensityFunction(p - glm::vec3(0.f, 0.f, epsilon));

	return glm::normalize(glm::vec3(dx, dy, dz));
}

void Octree::ConstructBottomUp(const int resolution, const int size, const glm::vec3 min)
{
	std::cout << "Constructing new octree from bottom up" << std::endl;

    // First loop through the area in 1x1x1 cubes, then 2x2x2, etc.
    int cubeSize = resolution * 2;
    int childCountPerAxis = size / cubeSize;
    int parentCountPerAxis = childCountPerAxis / 2;
    int childCount = pow(childCountPerAxis, 3);
    int parentCount = childCount / 8;
    std::vector<std::unique_ptr<OctreeChildren>> currentSizeNodes(childCount);
    std::vector<std::unique_ptr<OctreeChildren>> parentSizeNodes(parentCount);
    for(uint32_t i = 0; i < currentSizeNodes.size(); i++)
    {
        currentSizeNodes[i] = nullptr;
    }
    for(uint32_t i = 0; i < parentSizeNodes.size(); i++)
    {
        parentSizeNodes[i] = nullptr;
    }
    while (cubeSize != size)
    {
        for (int x = 0; x < size; x += cubeSize)
        {
            for (int y = 0; y < size; y += cubeSize)
            {
                for (int z = 0; z < size; z += cubeSize)
                {
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
                        node = Octree::ConstructLeafParent(resolution, pos); // null if nothing to draw
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
                            //std::cout << "no stuff in node" << std::endl;
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
							int field = currentSizeNodes[childIdx]->field;
							if (field == 0)
							{
								// just skip adding this anywhere
								printf("skipping (%i, %i, %i) with cubesize %i\n", x, y, z, cubeSize);
								continue;
							}

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

        currentSizeNodes = std::vector<std::unique_ptr<OctreeChildren>>();
        for(auto & node : parentSizeNodes)
        {
            currentSizeNodes.push_back(std::move(node));
        }
        parentSizeNodes = std::vector<std::unique_ptr<OctreeChildren>>(parentCount);
        for(uint32_t i = 0; i < parentSizeNodes.size(); i++)
        {
            parentSizeNodes[i] = nullptr;
        }
    }
    
    // currentSizeNodes length should be 1 now.
    m_children = std::move(currentSizeNodes[0]);
}

OctreeChildren* Octree::GetChildren() const
{
    if(!m_children)
    {
        return nullptr;
    }
    //printBinary(m_children.get()->field);
    return m_children.get() ? m_children.get() : nullptr;
}

void Octree::MeshFromOctree(IndexBuffer& indexBuffer, VertexBuffer& vertexBuffer)
{
	GenerateVertexIndices(vertexBuffer);
	CellProc(indexBuffer);
}

void Octree::GenerateVertexIndices(VertexBuffer& vertexBuffer)
{
	if (m_leaf)
	{
		m_index = vertexBuffer.size();
		glm::vec3 color = { 255.0, 0.0, 100.0};
		Vertex v = {
			m_drawPos,
			color,
			m_averageNormal
			// plus there's a single float of padding so that it's 3*3*4 + 4 = 40 bytes and aligned
		};
		vertexBuffer.push_back(v);
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_children->children[i]) {
				m_children->children[i]->GenerateVertexIndices(vertexBuffer);
			}
		}
	}
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

// XY/XZ/YZ means eg for XZ the cubes are on the bottom (0 1 4 5) and top four (2 3 6 7)

Octree* LeafOrChild(Octree* node, size_t idx)
{
	if (node->IsLeaf())
		return node;

	return node->GetChildren()->children[idx];
}

bool Octree::IsLeaf() const
{
	return m_leaf;
}

void Octree::CellProc(IndexBuffer& indexBuffer)
{
	if (!m_leaf)
	{
		const auto& children = m_children->children;

		// Cells
		for (int i = 0; i < 8; i++)
		{
			if (children[i])
			{
				children[i]->CellProc(indexBuffer);
			}
		}

		// Faces
		// X is dir 2 in example
		const std::tuple<int, int> xPairs[] = {
			{ 0, 1 },
			{ 2, 3 },
			{ 4, 5 },
			{ 6, 7 },
		};
		// Y is dir 1 in example
		const std::tuple<int, int> yPairs[] = {
			{ 0, 2 },
			{ 1, 3 },
			{ 4, 6 },
			{ 5, 7 },
		};
		// Z is dir 0 in example
		const std::tuple<int, int> zPairs[] = {
			{ 0, 4 },
			{ 1, 5 },
			{ 2, 6 },
			{ 3, 7 },
		};
		// dir 0 in edgeProcEdgeMask
		const std::tuple<int, int, int, int> xyPairs[] = {
			{ 0, 1, 2, 3},
			{ 4, 5, 6, 7},
		};
		// dir 1 in edgeProcEdgeMask
		const std::tuple<int, int, int, int> xzPairs[] = {
			{ 0, 1, 4, 5},
			{ 2, 3, 6, 7},
		};
		// dir 2 in edgeProcEdgeMask
		const std::tuple<int, int, int, int> yzPairs[] = {
			{ 0, 2, 4, 6},
			{ 1, 3, 5, 7},
		};
		for (const auto[a, b] : xPairs) {
			FaceProcX(children[a], children[b], indexBuffer);
		}
		for (const auto[a, b] : yPairs) {
			FaceProcY(children[a], children[b], indexBuffer);
		}
		for (const auto[a, b] : zPairs) {
			FaceProcZ(children[a], children[b], indexBuffer);
		}
		for (const auto[a, b, c, d] : xyPairs) {
			EdgeProcXY(children[a], children[b], children[c], children[d], indexBuffer);
		}
		for (const auto[a, b, c, d] : xzPairs) {
			EdgeProcXZ(children[a], children[b], children[c], children[d], indexBuffer);
		}
		for (const auto[a, b, c, d] : yzPairs) {
			EdgeProcYZ(children[a], children[b], children[c], children[d], indexBuffer);
		}
	}
	
	// TODO else do nothing?
}

void Octree::EdgeProcXY(Octree* n0, Octree* n1, Octree* n2, Octree* n3, IndexBuffer& indexBuffer)
{
	if (!n0 || !n1 || !n2 || !n3)
		return;

	const int dir = 2;
	if (n0->m_leaf && n1->m_leaf && n2->m_leaf && n3->m_leaf)
	{
		const Octree* nodes[4] = {
			n0, n2, n1, n3
		};
		return ProcessEdge(nodes, dir, indexBuffer);
	}
	
	EdgeProcXY(LeafOrChild(n0, 3), LeafOrChild(n1, 2), LeafOrChild(n2, 1), LeafOrChild(n3, 0), indexBuffer);
	EdgeProcXY(LeafOrChild(n0, 7), LeafOrChild(n1, 6), LeafOrChild(n2, 5), LeafOrChild(n3, 4), indexBuffer);
}
void Octree::EdgeProcXZ(Octree* n0, Octree* n1, Octree* n2, Octree* n3, IndexBuffer& indexBuffer)
{
	if (!n0 || !n1 || !n2 || !n3)
		return;

	const int dir = 1;
	if (n0->m_leaf && n1->m_leaf && n2->m_leaf && n3->m_leaf)
	{
		const Octree* nodes[4] = {
			n0, n1, n2, n3
		};
		return ProcessEdge(nodes, dir, indexBuffer);
	}
	EdgeProcXZ(LeafOrChild(n0, 5), LeafOrChild(n1, 4), LeafOrChild(n2, 1), LeafOrChild(n3, 0), indexBuffer);
	EdgeProcXZ(LeafOrChild(n0, 7), LeafOrChild(n1, 6), LeafOrChild(n2, 3), LeafOrChild(n3, 2), indexBuffer);
}
void Octree::EdgeProcYZ(Octree* n0, Octree* n1, Octree* n2, Octree* n3, IndexBuffer& indexBuffer)
{
	if (!n0 || !n1 || !n2 || !n3)
		return;

	const int dir = 0;
	if (n0->m_leaf && n1->m_leaf && n2->m_leaf && n3->m_leaf)
	{
		const Octree* nodes[4] = {
			n0, n2, n1, n3
		};
		return ProcessEdge(nodes, dir, indexBuffer);
	}
	EdgeProcYZ(LeafOrChild(n0, 6), LeafOrChild(n1, 4), LeafOrChild(n2, 2), LeafOrChild(n3, 0), indexBuffer);
	EdgeProcYZ(LeafOrChild(n0, 7), LeafOrChild(n1, 5), LeafOrChild(n2, 3), LeafOrChild(n3, 1), indexBuffer);
}

void Octree::FaceProcX(Octree* n0, Octree* n1, IndexBuffer& indexBuffer)
{
	if (!n0 || !n1)
		return;

	if (n0->m_leaf && n1->m_leaf)
		return;

	const std::tuple<int, int> facePairs[] = {
		{ 1, 0 },
		{ 3, 2 },
		{ 5, 4 },
		{ 7, 6 },
	};

	const std::tuple<int, int, int, int> edgeXYPairs[] = {
		{ 1, 0, 3, 2 },
		{ 5, 4, 7, 6 },
	};

	const std::tuple<int, int, int, int> edgeXZPairs[] = {
		{ 3, 2, 7, 6 },
		{ 1, 0, 5, 4 },
	};

	for (const auto [a, b] : facePairs) {
		FaceProcX(LeafOrChild(n0, a), LeafOrChild(n1, b), indexBuffer);
	}

	for (const auto [a, b, c, d] : edgeXYPairs) {
		EdgeProcXY(LeafOrChild(n0, a), LeafOrChild(n1, b), LeafOrChild(n0, c), LeafOrChild(n1, d), indexBuffer);
	}
	for (const auto [a, b, c, d] : edgeXZPairs) {
		EdgeProcXZ(LeafOrChild(n0, a), LeafOrChild(n1, b), LeafOrChild(n0, c), LeafOrChild(n1, d), indexBuffer);
	}
}
void Octree::FaceProcY(Octree* n0, Octree* n1, IndexBuffer& indexBuffer)
{
	if (!n0 || !n1)
		return;

	if (n0->m_leaf && n1->m_leaf)
		return;

	const std::tuple<int, int> facePairs[] = {
		{ 2, 0 },
		{ 6, 4 },
		{ 3, 1 },
		{ 7, 5 },
	};
	const std::tuple<int, int, int, int> edgeXYPairs[] = {
		{ 2, 3, 0, 1 },
		{ 6, 7, 4, 5 },
	};
	const std::tuple<int, int, int, int> edgeYZPairs[] = {
		{ 2, 0, 6, 4 },
		{ 3, 1, 7, 5 },
	};

	for (const auto [a, b] : facePairs) {
		FaceProcY(LeafOrChild(n0, a), LeafOrChild(n1, b), indexBuffer);
	}

	for (const auto [a, b, c, d] : edgeXYPairs) {
		EdgeProcXY(LeafOrChild(n0, a), LeafOrChild(n0, b), LeafOrChild(n1, c), LeafOrChild(n1, d), indexBuffer);
	}

	for (const auto [a, b, c, d] : edgeYZPairs) {
		EdgeProcYZ(LeafOrChild(n0, a), LeafOrChild(n1, b), LeafOrChild(n0, c), LeafOrChild(n1, d), indexBuffer);
	}
}
void Octree::FaceProcZ(Octree* n0, Octree* n1, IndexBuffer& indexBuffer)
{
	if (!n0 || !n1)
		return;

	if (n0->m_leaf && n1->m_leaf)
		return;

	const std::tuple<int, int> facePairs[] = {
		{ 4, 0 },
		{ 5, 1 },
		{ 6, 2 },
		{ 7, 3 },
	};
	const std::tuple<int, int, int, int> edgeYZPairs[] = {
		{ 4, 6, 0, 2 },
		{ 5, 7, 1, 3 },
	};
	const std::tuple<int, int, int, int> edgeXZPairs[] = {
		{ 4, 5, 0, 1 },
		{ 6, 7, 2, 3 },
	};

	for (const auto [a, b] : facePairs) {
		FaceProcZ(LeafOrChild(n0, a), LeafOrChild(n1, b), indexBuffer);
	}

	for (const auto [a, b, c, d] : edgeYZPairs) {
		EdgeProcYZ(LeafOrChild(n0, a), LeafOrChild(n0, b), LeafOrChild(n1, c), LeafOrChild(n1, d), indexBuffer);
	}

	for (const auto [a, b, c, d] : edgeXZPairs) {
		EdgeProcXZ(LeafOrChild(n0, a), LeafOrChild(n0, b), LeafOrChild(n1, c), LeafOrChild(n1, d), indexBuffer);
	}
}

const int edgevmap[12][2] =
{
	{0,4},{1,5},{2,6},{3,7},	// x-axis 
	{0,2},{1,3},{4,6},{5,7},	// y-axis
	{0,1},{2,3},{4,5},{6,7}		// z-axis
};
void Octree::ProcessEdge(const Octree* node[4], int dir, IndexBuffer& indexBuffer)
 {
	/*
	This function copied from https://github.com/nickgildea/DualContouringSample/blob/master/DualContouringSample/octree.cpp
	Implementations of Octree member functions.
	Copyright (C) 2011  Tao Ju
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public License
	(LGPL) as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/
	const int MATERIAL_AIR = 0;
	const int MATERIAL_SOLID = 1;

	const int processEdgeMask[3][4] = { {3,2,1,0},{7,5,6,4},{11,10,9,8} };
	int minSize = 1000000;		// arbitrary big number
	int minIndex = 0;
	int indices[4] = { -1, -1, -1, -1 };
	bool flip = false;
	bool signChange[4] = { false, false, false, false };

	for (int i = 0; i < 4; i++)
	{
		const int edge = processEdgeMask[dir][i];
		const int c1 = edgevmap[edge][0];
		const int c2 = edgevmap[edge][1];

		const int m1 = (node[i]->m_corners >> c1) & 1;
		const int m2 = (node[i]->m_corners >> c2) & 1;

		if (node[i]->m_size < minSize)
		{
			minSize = node[i]->m_size;
			minIndex = i;
			flip = m1 != MATERIAL_AIR;
		}

		indices[i] = node[i]->m_index;

		signChange[i] =
			(m1 == MATERIAL_AIR && m2 != MATERIAL_AIR) ||
			(m1 != MATERIAL_AIR && m2 == MATERIAL_AIR);
	}

	if (signChange[minIndex])
	{
		if (!flip)
		{
			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[1]);
			indexBuffer.push_back(indices[3]);

			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[3]);
			indexBuffer.push_back(indices[2]);
		}
		else
		{
			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[3]);
			indexBuffer.push_back(indices[1]);

			indexBuffer.push_back(indices[0]);
			indexBuffer.push_back(indices[2]);
			indexBuffer.push_back(indices[3]);
		}
	}
}
glm::vec3 ApproximateZeroCrossingPosition(const glm::vec3& p0, const glm::vec3& p1)
{
	/*
	This function copied from https://github.com/nickgildea/DualContouringSample/blob/master/DualContouringSample/octree.cpp
	Implementations of Octree member functions.
	Copyright (C) 2011  Tao Ju
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public License
	(LGPL) as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/
	// approximate the zero crossing by finding the min value along the edge
	float minValue = 100000.f;
	float t = 0.f;
	float currentT = 0.f;
	const int steps = 8;
	const float increment = 1.f / (float)steps;
	while (currentT <= 1.f)
	{
		const glm::vec3 p = p0 + ((p1 - p0) * currentT);
		const float density = glm::abs(DensityFunction(p));
		if (density < minValue)
		{
			minValue = density;
			t = currentT;
		}

		currentT += increment;
	}

	return p0 + ((p1 - p0) * t);
}
Octree* ConstructLeaf(const int resolution, glm::ivec3 min)
{
	/*
	This function (mostly) copied from https://github.com/nickgildea/DualContouringSample/blob/master/DualContouringSample/octree.cpp, ConstructLeaf
	Implementations of Octree member functions.
	Copyright (C) 2011  Tao Ju
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public License
	(LGPL) as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/
	const float QEF_ERROR = 1e-6f;
	const int QEF_SWEEPS = 4;
	const int MATERIAL_AIR = 0;
	const int MATERIAL_SOLID = 1;
	using namespace glm;


	Octree* leaf = new Octree(resolution, min);

	int corners = 0;
	for (int i = 0; i < 8; i++)
	{
		const ivec3 cornerPos = leaf->m_min + CHILD_MIN_OFFSETS[i];
		const bool inside = !Sample(vec3(cornerPos));
		if (inside)
		{
			corners |= (1 << i);
		}
	}

	if (corners == 0 || corners == 255)
	{
		// voxel is full inside or outside the volume
		delete leaf;
		return nullptr;
	}

	// otherwise the voxel contains the surface, so find the edge intersections
	const int MAX_CROSSINGS = 6;
	int edgeCount = 0;
	vec3 averageNormal(0.f);
	svd::QefSolver qef;

	for (int i = 0; i < 12 && edgeCount < MAX_CROSSINGS; i++)
	{
		const int c1 = edgevmap[i][0];
		const int c2 = edgevmap[i][1];

		const int m1 = (corners >> c1) & 1;
		const int m2 = (corners >> c2) & 1;

		if ((m1 == MATERIAL_AIR && m2 == MATERIAL_AIR) ||
			(m1 == MATERIAL_SOLID && m2 == MATERIAL_SOLID))
		{
			// no zero crossing on this edge
			continue;
		}

		const vec3 p1 = vec3(min + CHILD_MIN_OFFSETS[c1]);
		const vec3 p2 = vec3(min + CHILD_MIN_OFFSETS[c2]);
		const vec3 p = ApproximateZeroCrossingPosition(p1, p2);
		const vec3 n = CalculateSurfaceNormal(p);
		qef.add(p.x, p.y, p.z, n.x, n.y, n.z);

		averageNormal += n;

		edgeCount++;
	}

	svd::Vec3 qefPosition;
	qef.solve(qefPosition, QEF_ERROR, QEF_SWEEPS, QEF_ERROR);

	leaf->m_drawPos = vec3(qefPosition.x, qefPosition.y, qefPosition.z);
	leaf->m_qef = qef.getData();

	const vec3 max = vec3(leaf->m_min + ivec3(leaf->m_size)); // why do we have to add this one's specific size? Shouldn't it always be 1?

	if (leaf->m_drawPos.x < min.x || leaf->m_drawPos.x > max.x ||
		leaf->m_drawPos.y < min.y || leaf->m_drawPos.y > max.y ||
		leaf->m_drawPos.z < min.z || leaf->m_drawPos.z > max.z)
	{
		const auto& mp = qef.getMassPoint();
		leaf->m_drawPos = vec3(mp.x, mp.y, mp.z);
	}

	leaf->m_averageNormal = glm::normalize(averageNormal / (float)edgeCount);
	leaf->m_corners = corners;

	return leaf;
}

Octree* Octree::ConstructLeafParent(const int resolution, const glm::vec3 min)
{
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

				glm::vec3 childPos = glm::vec3(x + min.x, y + min.y, z + min.z) * (float)resolution;
				Octree* child = ConstructLeaf(resolution, childPos);
				if (child)
				{
                    field |= (1 << cornerIdx);
				}

				// Set in any case to children array, so elements are not left with default values, but nullptr instead.
				children[cornerIdx] = child;
            }
        }
    }

    if (field != 0 && field != 255)  // todo check if this is actually a good idea
    {
		// TODO: maybe check here field = 0xFF aka all children exist so we can possibly
		// approximate all children with a single vertex. Check QEFs. Set m_leaf'ness.
		// ^ wot?

        auto c = std::unique_ptr<OctreeChildren>(new OctreeChildren());
        c->field = field;
        //printBinary(c->field);
        c->children = children;
        Octree* o = new Octree(std::move(c), resolution * 2, min, resolution);
        return o;
    }
    return nullptr;
}
