//
// Created by Teemu Sarapisto on 20/07/2017.
//
#include <array>
#include <cmath>
#include <iostream>
#include <cstdint>

#include "Octree.h"

using namespace glm;

const vec3 CHILD_MIN_OFFSETS[] =
{
        vec3(0,0,0),
        vec3(0,0,1),
        vec3(0,1,0),
        vec3(0,1,1),
        vec3(1,0,0),
        vec3(1,0,1),
        vec3(1,1,0),
        vec3(1,1,1),
};

Octree::~Octree()
{
    /////// NYI
}


// Call this to initialize a new node with children already made
// in this case no need to create anything except maybe construct what to draw.
Octree::Octree(int size, glm::vec3 min, std::array<Octree*, 8> Children, int8_t ChildField)
        : m_children(Children), m_childField(ChildField)
{
    std::cout << "pre-initialized octree" << std::endl;
    std::string output = "";
    for(int i = 7; i >= 0; i--)
    {
        ((m_childField & (1 << i)) > 0)
            ? output += "1"
            : output += "0";
    }
    std::cout << output << std::endl;
}

// Call this if creating a completely new octree
// or a leaf node
Octree::Octree(const int resolution, const int size, const glm::vec3 min)
{
	if (resolution == size) {
        std::cout << "creating a leaf" << std::endl;
		ConstructLeaf(resolution, min);
	}
	else
	{
        std::cout << "constructing from bottom up" << std::endl;
		ConstructBottomUp(resolution, size, min);
	}
}

void Octree::ConstructLeaf(const int resolution, const glm::vec3 min)
{
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

void Octree::ConstructBottomUp(const int maxResolution, const int size, const glm::vec3 min)
{
    int cubeSize = maxResolution;

    std::array<Octree*, 8> oldChildren;
    int8_t oldChildField = 0;

    while (cubeSize < size)
    {
        // for each size, create the children on behalf of their owner

        std::array<Octree*, 8> newChildren;
        int8_t newChildField = 0;
        for(int x = 0; x < 2; x++)
        {
            for(int y = 0; y < 2; y++)
            {
                for(int z = 0; z < 2; z++)
                {
                    const int idx = index(x, y, z, 2);

                    const glm::vec3 childCornerPos = min + CHILD_MIN_OFFSETS[idx] * (float)cubeSize;

                    Octree* tree = nullptr;
                    if(cubeSize > maxResolution) {
                        // look up children created on a previous level from array
                        // if previous level children count for this node is 0, skip this node as well!

                        // the children array should be populated now as this shouldn't be the first round.
                        // (on the first round cubeSize == maxResolution)
                        if(oldChildField > 0 && x == 0 && y == 0 && z == 0)
                        {
                            tree = new Octree(cubeSize, childCornerPos, oldChildren, oldChildField);
                        }
                        else if(x == 0 && y == 0 && z == 0)
                        {
                            // no children for this node, nothing to draw for this node either
                            continue;
                        }
                        else
                        {
                            // we will have to check if this node has children
                            tree = new Octree(maxResolution, cubeSize, childCornerPos);
                        }
                    }
                    else
                    {
                        // if cube size is maxResolution, we're creating leaf nodes
                        // this means no children

                        tree = new Octree(maxResolution, cubeSize, childCornerPos);
                    }

                    if (!tree->HasSomethingToRender())
                    {
                        delete tree;
                        tree = nullptr;

                        continue;
                    }

                    newChildren[idx] = tree;
                    newChildField = newChildField & (1 << idx);
                }
            }
        }
        cubeSize *= 2;
        oldChildren = newChildren;
        oldChildField = newChildField;
    }
}

//void Octree::ConstructBottomUp(int size, int min)
//{
//	// Figure out the area encapsulated by this cube.
//	// First loop through the area in 1x1x1 cubes, then  2x2x2, etc.
//
//	int cubeSize = 1;
//	while (cubeSize != size) {
//		std::vector<std::vector<Octree*[8]> > children;
//
//		// there will be (size / cubesize) cubes per axis
//		// and /2 that many cubes in the parent   <-- wat? /9 for quadtrees and /27 for octrees, right?
//
//		const int parentCountPerAxis = (size / cubeSize) / 2;
//		const int parentCount = pow(size / cubeSize, 3) / 8;
//
//		//Octree parents[parentCount][parentCount][parentCount];
//
//		Octree** parents = new Octree*[parentCountPerAxis * parentCountPerAxis * parentCountPerAxis];
//		for (int x = 0; x < size; x += cubeSize)
//		{
//			for (int y = 0; y < size; y += cubeSize)
//			{
//				for (int z = 0; z < size; z += cubeSize)
//				{
//					int parentX = x / (cubeSize * 2);
//					int parentY = y / (cubeSize * 2);
//					int parentZ = z / (cubeSize * 2);
//					int idx = index(parentX, parentY, parentZ, parentCount);
//				}
//			}
//		}
//		cubeSize *= 2;
//	}
//}

