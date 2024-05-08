#pragma once

#include <glm/glm.hpp>
#include "Octree.h"
#include "OctreeMesh.h"

Octree* CreateNewSubTree(Octree* rootNode, const glm::vec3& newPosition, int chunkSize);
Direction expansionDirection(const Octree* root, const glm::vec3& newPos, int chunkSize);
Octree* FindNode(Octree* rootNode, glm::ivec3 position, int chunkSize);
std::vector<Octree*> FindNeighbors(Octree* rootNode, glm::vec3 nodePos, int nodeSize);