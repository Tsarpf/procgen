#pragma once

#include "Octree.h"

bool needsExpansion(const Octree* root, const glm::vec3& newPos, int chunkSize);