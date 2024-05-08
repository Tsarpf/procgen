#include "OctreeUtil.h"

Direction expansionDirection(const Octree* root, const glm::vec3& newPos, int chunkSize) {
    glm::vec3 maxBound = root->m_min + root->m_size;
    bool expandNeeded = false;
    if (newPos.x < root->m_min.x || newPos.x >= maxBound.x) {
        return (newPos.x < root->m_min.x) ? Direction::xminus : Direction::xplus;
    }
    if (newPos.y < root->m_min.y || newPos.y >= maxBound.y) {
        return (newPos.y < root->m_min.y) ? Direction::yminus : Direction::yplus;
    }
    if (newPos.z < root->m_min.z || newPos.z >= maxBound.z) {
        return (newPos.z < root->m_min.z) ? Direction::zminus : Direction::zplus;
    }

    return Direction::nodir;

}
// bool needsExpansion(const glm::vec3& position) {
//     glm::vec3 maxBound = m_tree->m_min + glm::vec3(m_tree->m_size);
//     return position.x < m_tree->m_min.x || position.x >= maxBound.x ||
//            position.y < m_tree->m_min.y || position.y >= maxBound.y ||
//            position.z < m_tree->m_min.z || position.z >= maxBound.z;
// }
int calculateChildIndex(const Octree* node, const glm::vec3& position) {
    int x = position.x >= (node->m_min.x + node->m_size / 2) ? 1 : 0;
    int y = position.y >= (node->m_min.y + node->m_size / 2) ? 1 : 0;
    int z = position.z >= (node->m_min.z + node->m_size / 2) ? 1 : 0;
    return octreeIndex(x, y, z);
}

Octree* CreateNewSubTree(Octree* rootNode, const glm::vec3& newPosition, int chunkSize) {
    Octree* currentNode = rootNode;
    while (currentNode->m_size > chunkSize) {
        OctreeChildren* children = currentNode->GetChildren();

        int x = newPosition.x >= (currentNode->m_min.x + currentNode->m_size / 2) ? 1 : 0;
        int y = newPosition.y >= (currentNode->m_min.y + currentNode->m_size / 2) ? 1 : 0;
        int z = newPosition.z >= (currentNode->m_min.z + currentNode->m_size / 2) ? 1 : 0;
        
        int childIndex = octreeIndex(x, y, z);


        if (!children->children[childIndex]) {
            // Create new child node if it does not exist
            glm::ivec3 childMin = currentNode->m_min + glm::ivec3(x * currentNode->m_size / 2, y * currentNode->m_size / 2, z * currentNode->m_size / 2);
            int newNodeSize = currentNode->m_size / 2;
            children->children[childIndex] = new Octree(chunkSize, childMin);
        }

        // Move to the child node
        currentNode = children->children[childIndex];
    }

    if (currentNode->m_size != chunkSize) {
        throw std::runtime_error("Error: Chunk size mismatch or position out of bounds.");
    }

    return currentNode;
}


Octree* FindNode(Octree* rootNode, glm::ivec3 position, int chunkSize)
{
    Octree* currentNode = rootNode;
    while (currentNode)
    {
        if (currentNode->IsLeaf())
        {
            return currentNode;
        }

        OctreeChildren* children = currentNode->GetChildren();
        if (!children->field)
        {
            return nullptr; // No children or content in this node, there will never be anything interesting here so just early exit.
        }

        if (currentNode->m_size == chunkSize)
        {
            assert(currentNode->m_min == position && "Found node, but position does not match the target position. Probably descended into a wrong child?");
            return currentNode;
        }

        assert(currentNode->m_size > chunkSize && "Node size is smaller than the target chunk size. Something failed");

        int childIndex = calculateChildIndex(currentNode, position);

        currentNode = children->children[childIndex];
    }

    return nullptr;
}

std::vector<Octree *> FindNeighbors(Octree *rootNode, glm::vec3 nodePos, int nodeSize)
{
    std::vector<Octree *> neighbors;
    std::vector<glm::vec3> neighborPositions = {
        nodePos + glm::vec3(nodeSize, 0, 0), // xplus
        nodePos - glm::vec3(nodeSize, 0, 0), // xminus
        nodePos + glm::vec3(0, nodeSize, 0), // yplus
        nodePos - glm::vec3(0, nodeSize, 0), // yminus
        nodePos + glm::vec3(0, 0, nodeSize), // zplus
        nodePos - glm::vec3(0, 0, nodeSize)  // zminus
    };

    for (const auto &pos : neighborPositions)
    {
        Octree *neighbor = FindNode(rootNode, pos, nodeSize);
        if (neighbor)
        {
            neighbors.push_back(neighbor);
        }
    }

    return neighbors;
}