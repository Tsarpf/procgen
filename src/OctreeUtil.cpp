#include "OctreeUtil.h"

Direction expansionDirection(const Octree* root, const glm::vec3& newPos, int chunkSize) {
    glm::vec3 maxBound = root->m_min + glm::vec3(root->m_size);
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

Octree* FindNode(Octree* rootNode, const glm::vec3& newPosition, int chunkSize) {
    Octree* currentNode = rootNode;
    while (currentNode->m_size > chunkSize) {
        int x = newPosition.x >= (currentNode->m_min.x + currentNode->m_size / 2) ? 1 : 0;
        int y = newPosition.y >= (currentNode->m_min.y + currentNode->m_size / 2) ? 1 : 0;
        int z = newPosition.z >= (currentNode->m_min.z + currentNode->m_size / 2) ? 1 : 0;
        
        int childIndex = octreeIndex(x, y, z);

        if (currentNode->m_size / 2 < chunkSize) {
            break; // Stop if the next level is smaller than the required chunk size
        }

        if (!currentNode->children[childIndex]) {
            // Create new child node if it does not exist
            glm::vec3 childMin = currentNode->m_min + glm::vec3(x * currentNode->m_size / 2, y * currentNode->m_size / 2, z * currentNode->m_size / 2);
            currentNode->children[childIndex] = new Octree(chunkSize, childMin);
        }

        // Move to the child node
        currentNode = currentNode->children[childIndex];
    }

    if (currentNode->m_size != chunkSize) {
        throw std::runtime_error("Error: Chunk size mismatch or position out of bounds.");
    }

    return currentNode;
}
