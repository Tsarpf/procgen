//
// Created by Teemu Sarapisto on 20/07/2017.
//
#include <array>
#include <cmath>
#include <cstdint>

#include "Octree.h"

// Call this to initialize a new node with children already made
Octree::Octree(int size, int min, std::array<Octree*, 8> children, int8_t childField)
{
}

// Call this if creating a completely new octree
// or a leaf node
Octree::Octree(const int maxResolution, const int size, const int min)
{
	if (size == 1) {
		ConstructLeaf(min);
	}
	else
	{
		ConstructBottomUp(maxResolution, size, min);
	}
}

Octree::Octree()
{

}

void Octree::ConstructLeaf(int min)
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

// 'min' is the x=y=z coordinate of the minimum corner of the octree.
void Octree::ConstructBottomUp(const int maxResolution, const int size, const int min)
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
                    // having a function that creates children on behalf of a node doesn't make any sense
                    // because the children's children should already exist. or something. i don't remember

                    Octree* tree = nullptr;
                    if(cubeSize > maxResolution) {
                        // in this case we've ran at least one round,
                        // so there are children we can pass along.

                        // look up children created on a previous level from array
                        // if previous level children count for this node is 0, skip this node as well!

                        // the children array should be populated now as this shouldn't be the first round.
                        if(oldChildField > 0 && x == 0 && y == 0 && z == 0)
                        {
                            tree = new Octree(cubeSize, min, oldChildren, oldChildField);
                        }
                        else if(x == 0 && y == 0 && z == 0)
                        {
                            // no children for this node, nothing to draw for this node either
                            continue;
                        }
                        else
                        {
                            // we will have to check if this node has children
                            tree = new Octree(maxResolution, cubeSize, min);
                        }
                    }
                    else
                    {
                        // if cube size is maxResolution, we're creating leaf nodes
                        // this means no children

                        tree = new Octree(maxResolution, cubeSize, min);
                    }

                    if (!tree->HasSomethingToRender())
                    {
                        delete tree;
                        tree = nullptr;

                        continue;
                    }

                    int idx = index(x, y, z, 2);
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
}

