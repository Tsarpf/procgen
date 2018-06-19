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
}


// Used to initialize with children
Octree::Octree(std::unique_ptr<OctreeChildren> children, int size, glm::vec3 min) : m_children(std::move(children))
{
    std::cout << "pre-initialized octree" << std::endl;
    printBinary(m_children->field);
}


// Called when creating a new octree
Octree::Octree(const int resolution, const int size, const glm::vec3 min)
{
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
    return pos.x == 1 ? true : false;
}

Octree::Octree(const int resolution, glm::vec3 min)
{
    // NYI
    // Create leaf node draw data here.
    //std::cout << "creating leaf node at resolution: " << resolution << std::endl;
    //std::cout << "and min: " <<  min.x << min.y << min.z << std::endl;
}

Octree* Octree::ConstructLeaf(const int resolution, const glm::vec3 min)
{
    uint8_t field = 0;
    std::array<Octree*, 8> children;
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
                    children[cornerIdx] = new Octree(resolution, min);
                }

            }
        }
    }

    //printBinary(field);
    if (field > 0) 
    {
        std::cout << "field is over 0" << std::endl;
        auto c = new OctreeChildren();
        c->field = field;
        //printBinary(c->field);
        c->children = children;
        Octree* o = new Octree(std::unique_ptr<OctreeChildren>(c), resolution * 2, min);
        return o;
    }
    else
    {
        std::cout << "not over 0 " << field << std::endl;
        //std::cout << "field is not over 0" << std::endl;
    }

    return nullptr;
}

void Octree::ConstructBottomUp(const int resolution, const int size, const glm::vec3 min)
{
    // First loop through the area in 1x1x1 cubes, then  2x2x2, etc.

    int cubeSize = resolution * 2;
    int childCountPerAxis = size / cubeSize;
    int parentCountPerAxis = childCountPerAxis / 2;
    int childCount = pow(childCountPerAxis, 3);
    int parentCount = childCount / 8;
    //TODO unique_ptr hommat
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
                    std::cout << "x, y, z: " << x << y << z << std::endl;

                    const int childIdxX = x / cubeSize;
                    const int childIdxY = y / cubeSize;
                    const int childIdxZ = z / cubeSize;
                    const int parentIdxX = childIdxX / 2;
                    const int parentIdxY = childIdxY / 2;
                    const int parentIdxZ = childIdxZ / 2;
                    const int parentIdx = index(parentIdxX, parentIdxY, parentIdxZ, parentCountPerAxis);
                    const int childIdx = index(childIdxX, childIdxY, childIdxZ, size / cubeSize);
                    const int cornerIdx = index(childIdxX % 2, childIdxY % 2, childIdxZ % 2, 2);
                    const glm::vec3 pos = min + CHILD_MIN_OFFSETS[cornerIdx] * (float)cubeSize;
                    Octree* node = nullptr;
                    if (cubeSize == resolution * 2)
                    {
                        node = Octree::ConstructLeaf(resolution, pos); // null if nothing to draw
                        if (node)
                        {
                            std::cout << "node had stuff" << std::endl;
                            if (!parentSizeNodes[parentIdx])
                            {
                                std::array<Octree*, 8> children = {};
                                children[cornerIdx] = node;
                                parentSizeNodes[parentIdx] = std::unique_ptr<OctreeChildren>(new OctreeChildren
                                {
                                    (1 << cornerIdx),
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
                            // do nothing?
                        }
                    }
                    else
                    {
                        std::cout << "creating non-leafs" << std::endl;
                        // is a container element, should just contain previously created children, if any
                        if(currentSizeNodes[childIdx]) // <- todo check if sane
                        {
                            std::cout << "has children" << std::endl;
                            // Create a new octree node whose children are the ones created in previous
                            // loop iterations parents that are now currentSized
                            node = new Octree(std::move(currentSizeNodes[childIdx]), resolution * 2, min);

                            //////////////////////////////////////////////////////////// This is the same as lines 180 ->
                            if (!parentSizeNodes[parentIdx])
                            {
                                std::cout << "parent array not created" << std::endl;
                                std::array<Octree*, 8> children = {};
                                children[cornerIdx] = node;
                                parentSizeNodes[parentIdx] = std::unique_ptr<OctreeChildren>(new OctreeChildren
                                {
                                    (1 << cornerIdx),
                                    children
                                });
                            }
                            else // parent list already created
                            {
                                std::cout << "parent array already created" << std::endl;
                                parentSizeNodes[parentIdx]->field |= (1 << cornerIdx);
                                parentSizeNodes[parentIdx]->children[cornerIdx] = node;
                            }
                            //////////////////////////////////////////////////////////// end copypasta from 180
                        }
                        else
                        {
                            std::cout << "no children" << std::endl;
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

        currentSizeNodes = std::move(parentSizeNodes);
        parentSizeNodes = std::vector<std::unique_ptr<OctreeChildren>>(parentCount);
        for(int i = 0; i < parentSizeNodes.size(); i++)
        {
            parentSizeNodes[i] = nullptr;
        }
    }
    
    // currentSizeNodes length should be 1 now.
    m_children = std::move(currentSizeNodes[0]);
}

OctreeChildren* Octree::GetChildren()
{
    return m_children.get();
}