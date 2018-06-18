//
// Created by Teemu Sarapisto on 20/07/2017.
//
#include <array>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <memory>

#include "Octree.h"

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

void PrintBinary(uint8_t field)
{
    std::cout << "printing field" << std::endl;
    std::string output = "";
    for(int i = 7; i >= 0; i--)
    {
        ((field & (1 << i)) > 0)
            ? output += "1"
            : output += "0";
    }
    std::cout << output << std::endl;
}


// Used to initialize with children
Octree::Octree(std::unique_ptr<OctreeChildren> children, int size, glm::vec3 min) : m_children(std::move(children))
{
    std::cout << "pre-initialized octree" << std::endl;
    PrintBinary(children->field);
}


// Call this to initialize a new node with children already made
// in this case no need to create anything except maybe construct what to draw.
//Octree::Octree(int size, glm::vec3 min, std::array<Octree*, 8> Children, int8_t ChildField)
//        : m_children(Children), m_childField(ChildField)
//{
//    std::cout << "pre-initialized octree" << std::endl;
//    std::string output = "";
//    for(int i = 7; i >= 0; i--)
//    {
//        ((m_childField & (1 << i)) > 0)
//            ? output += "1"
//            : output += "0";
//    }
//    std::cout << output << std::endl;
//}
//
//// Call this if creating a completely new octree
//// or a leaf node
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

//struct OctreeChildren {
//	uint8_t field;
//	std::array<Octree*, 8> children;  
//};

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

    PrintBinary(field);
    if (field > 0) 
    {
        std::cout << "field is over 0" << std::endl;
        //OctreeChildren* c = new OctreeChildren();
        auto c = std::unique_ptr<OctreeChildren>(new OctreeChildren());
        c->field = field;
        c->children = children;
        Octree* o = new Octree(std::move(c), resolution * 2, min);
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
                            //node = new Octree(cubeSize, pos, oldParents[childIdx]);

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
                    //newParents[parentIdx]->children[cornerIdx] = node;
                    //newParents[parentIdx]->field = newParents[parentIdx]->field & (1 << cornerIdx);
                }
            }
        }
        cubeSize *= 2;
        childCountPerAxis = size / cubeSize;
        parentCountPerAxis = childCountPerAxis / 2;
        childCount = pow(childCountPerAxis, 3);
        parentCount = childCount / 8;

        //Octree** parents = new Octree*[parentCountPerAxis * parentCountPerAxis * parentCountPerAxis];

        currentSizeNodes = std::move(parentSizeNodes);
        for(int i = 0; i < currentSizeNodes.size(); i++)
        {
            std::cout << "idx: " << i << std::endl;
            if(currentSizeNodes[i])
            {
                PrintBinary(currentSizeNodes[i]->field);
            }
            else 
            {
                std::cout << "no array" << std::endl;
            }
        }
        parentSizeNodes = std::vector<std::unique_ptr<OctreeChildren>>(parentCount);
        for(int i = 0; i < parentSizeNodes.size(); i++)
        {
            parentSizeNodes[i] = nullptr;
        }
    }
}


//void Octree::ConstructBottomUp(const int maxResolution, const int size, const glm::vec3 min)
//{
//    int cubeSize = maxResolution;
//
//    std::array<Octree*, 8> oldChildren;
//    uint8_t oldChildField = 0;
//
//    while (cubeSize < size)
//    {
//        // for each size, create the children on behalf of their owner
//
//        std::array<Octree*, 8> newChildren;
//        uint8_t newChildField = 0;
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

//void Octree::ConstructBottomUp(const int resolution, const int size, const glm::vec3 min)
//{
//    // Figure out the area encapsulated by this cube.
//    // First loop through the area in 1x1x1 cubes, then  2x2x2, etc.
//
//    int cubeSize = resolution * 2;
//    while (cubeSize != size)
//    {
//        std::vector<std::vector<Octree *[8]> > children;
//
//        // there will be (size / cubesize) cubes per axis
//        // and /2 that many cubes _per axis_ in the parent
//
//        const int childCountPerAxis = size / cubeSize;
//        const int parentCountPerAxis = childCountPerAxis / 2;
//        const int parentCount = pow(size / cubeSize, 3) / 8;
//
//        //Octree** parents = new Octree*[parentCountPerAxis * parentCountPerAxis * parentCountPerAxis];
//
//
//        // Should we initialize them to null pointers.
//        std::vector<OctreeChildren*> newParents(parentCount);
//        // TODO:
//        // oldParents should probably be a vector, since it's size will change dynamically.
//        // we cannot place it outside the loop because then it's size might be different from newParents,
//        // and we cannot place it inside the loop, since then its contents wouldn't survive to the next loop as it should.
//        std::array<OctreeChildren*, parentCount> oldParents;
//
//        for (int x = 0; x < size; x += cubeSize)
//        {
//            for (int y = 0; y < size; y += cubeSize)
//            {
//                for (int z = 0; z < size; z += cubeSize)
//                {
//                    // 'x / cubesize' is how manyth index we're in, in child sizes
//                    // for parents, for each 2 (per axis) per child, the parent idx should rise.
//                    // if we divie by two, first two are 0 / 2, 1/2. then 2/2, 3/2, 4/2... yup the floored integer
//                    // increases every other child idx. so it's correct.
//                    const int childIdxX = x / cubeSize;
//                    const int childIdxY = y / cubeSize;
//                    const int childIdxZ = z / cubeSize;
//                    const int parentIdxX = childIdxX / 2;
//                    const int parentIdxY = childIdxY / 2;
//                    const int parentIdxZ = childIdxZ / 2;
//                    const int parentIdx = index(parentIdxX, parentIdxY, parentIdxZ, parentCountPerAxis);
//                    const int childIdx = index(childIdxX, childIdxY, childIdxZ, size / cubeSize);
//                    const int cornerIdx = index(childIdxX % 2, childIdxY % 2, childIdxZ % 2, 2);
//                    const glm::vec3 childPos = min + CHILD_MIN_OFFSETS[cornerIdx] * cubeSize;
//
//                    Octree* child = nullptr;
//                    if (cubeSize == resolution*2)
//                    {
//                        // 1) is max res level aka has no children, just check it's draw info and add to it's parent array
//                        //      if there is something to draw for it
//                        child = new Octree(resolution, cubeSize, childPos);
//                    }
//                    else
//                    {
//                        // 2) is not max res level, check if has children, if yes, check if this one has something to draw
//                        //      if yes, add to parents array
//
//                        child = new Octree(cubeSize, childPos, oldParents[childIdx]);
//                    }
//
//                    if (!child->HasSomethingToRender())
//                    {
//                        delete child;
//                        child = nullptr;
//                        continue;
//                    }
//
//                    // do we really want to do the null checking dance here?
//                    // Should we initialize them to nullptr?
//                    if(!newParents[parentIdx])
//                    {
//                        // initialize OctreeChildren
//                    }
//                    newParents[parentIdx]->children[cornerIdx] = child;
//                    newParents[parentIdx]->field = newParents[parentIdx]->field & (1 << cornerIdx);
//                }
//            }
//        }
//        cubeSize *= 2;
//        oldParents = newParents;
//    }
//}

