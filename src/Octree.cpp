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

//void Octree::ConstructBottomUp(const int maxResolution, const int size, const glm::vec3 min)
//{
//    int cubeSize = maxResolution;
//
//    std::array<Octree*, 8> oldChildren;
//    int8_t oldChildField = 0;
//
//    while (cubeSize < size)
//    {
//        // for each size, create the children on behalf of their owner
//
//        std::array<Octree*, 8> newChildren;
//        int8_t newChildField = 0;
//        for(int x = 0; x < 2; x++)
//        {
//            for(int y = 0; y < 2; y++)
//            {
//                for(int z = 0; z < 2; z++)
//                {
//                    const int idx = index(x, y, z, 2);
//
//                    const glm::vec3 childCornerPos = min + CHILD_MIN_OFFSETS[idx] * (float)cubeSize;
//
//                    Octree* tree = nullptr;
//                    if(cubeSize > maxResolution) {
//                        // look up children created on a previous level from array
//                        // if previous level children count for this node is 0, skip this node as well!
//
//                        // the children array should be populated now as this shouldn't be the first round.
//                        // (on the first round cubeSize == maxResolution)
//                        if(oldChildField > 0 && x == 0 && y == 0 && z == 0)
//                        {
//                            tree = new Octree(cubeSize, childCornerPos, oldChildren, oldChildField);
//                        }
//                        else if(x == 0 && y == 0 && z == 0)
//                        {
//                            // no children for this node, nothing to draw for this node either
//                            continue;
//                        }
//                        else
//                        {
//                            // we will have to check if this node has children
//                            tree = new Octree(maxResolution, cubeSize, childCornerPos);
//                        }
//                    }
//                    else
//                    {
//                        // if cube size is maxResolution, we're creating leaf nodes
//                        // this means no children
//
//                        tree = new Octree(maxResolution, cubeSize, childCornerPos);
//                    }
//
//                    if (!tree->HasSomethingToRender())
//                    {
//                        delete tree;
//                        tree = nullptr;
//
//                        continue;
//                    }
//
//                    newChildren[idx] = tree;
//                    newChildField = newChildField & (1 << idx);
//                }
//            }
//        }
//        cubeSize *= 2;
//        oldChildren = newChildren;
//        oldChildField = newChildField;
//    }
//}

void Octree::ConstructBottomUp(const int resolution, const int size, const glm::vec3 min)
{
    // Figure out the area encapsulated by this cube.
    // First loop through the area in 1x1x1 cubes, then  2x2x2, etc.

    int cubeSize = resolution;
    while (cubeSize != size)
    {
        std::vector<std::vector<Octree *[8]> > children;

        // there will be (size / cubesize) cubes per axis
        // and /2 that many cubes _per axis_ in the parent

        const int childCountPerAxis = size / cubeSize;
        const int parentCountPerAxis = childCountPerAxis / 2;
        const int parentCount = pow(size / cubeSize, 3) / 8;

        //Octree** parents = new Octree*[parentCountPerAxis * parentCountPerAxis * parentCountPerAxis];


        // Should we initialize them to null pointers.
        std::array<OctreeChildren*, parentCount> newParents;
        // TODO:
        // oldParents should probably be a vector, since it's size will change dynamically.
        // we cannot place it outside the loop because then it's size might be different from newParents,
        // and we cannot place it inside the loop, since then its contents wouldn't survive to the next loop as it should.
        std::array<OctreeChildren*, parentCount> oldParents;

        for (int x = 0; x < size; x += cubeSize)
        {
            for (int y = 0; y < size; y += cubeSize)
            {
                for (int z = 0; z < size; z += cubeSize)
                {
                    // 'x / cubesize' is how manyth index we're in, in child sizes
                    // for parents, for each 2 (per axis) per child, the parent idx should rise.
                    // if we divie by two, first two are 0 / 2, 1/2. then 2/2, 3/2, 4/2... yup the floored integer
                    // increases every other child idx. so it's correct.
                    int childIdxX = x / cubeSize;
                    int childIdxY = y / cubeSize;
                    int childIdxZ = z / cubeSize;
                    int parentIdxX = childIdxX / 2;
                    int parentIdxY = childIdxY / 2;
                    int parentIdxZ = childIdxZ / 2;
                    int parentIdx = index(parentIdxX, parentIdxY, parentIdxZ, parentCountPerAxis);
                    int childIdx = index(childIdxX, childIdxY, childIdxZ, size / cubeSize);
                    int cornerIdx = index(childIdxX % 2, childIdxY % 2, childIdxZ % 2, 2);
                    const glm::vec3 childPos = min + CHILD_MIN_OFFSETS[cornerIdx] * cubeSize;

                    Octree* child = nullptr;
                    if (cubeSize == resolution)
                    {
                        // 1) is max res level aka has no children, just check it's draw info and add to it's parent array
                        //      if there is something to draw for it
                        child = new Octree(resolution, cubeSize, childPos);
                    }
                    else
                    {
                        // 2) is not max res level, check if has children, if yes, check if this one has something to draw
                        //      if yes, add to parents array

                        child = new Octree(cubeSize, childPos, oldParents[childIdx]);
                    }

                    if (!child->HasSomethingToRender())
                    {
                        delete child;
                        child = nullptr;
                        continue;
                    }

                    // do we really want to do the null checking dance here?
                    // Should we initialize them to nullptr?
                    if(!newParents[parentIdx])
                    {
                        // initialize OctreeChildren
                    }
                    newParents[parentIdx]->children[cornerIdx] = child;
                    newParents[parentIdx]->field = newParents[parentIdx]->field & (1 << cornerIdx);
                }
            }
        }
        cubeSize *= 2;
        oldParents = newParents;
    }
}

