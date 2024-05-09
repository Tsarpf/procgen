#pragma once

#include <glm/glm.hpp>
#include "Octree.h"
#include "OctreeMesh.h"

Direction expansionDirection(const Octree* root, const glm::ivec3& newPos, int chunkSize);
Octree* FindNode(Octree* rootNode, glm::ivec3 position, int chunkSize);
std::vector<std::pair<Octree*, Direction>> FindNeighbors(Octree *rootNode, glm::ivec3 nodePos, int nodeSize);