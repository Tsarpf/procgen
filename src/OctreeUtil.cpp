#include "Octree.h"
#include "OctreeMesh.h"

Direction expansionDirection(const Octree* root, const glm::ivec3& newPos, int chunkSize) {
    glm::vec3 maxBound = root->m_min + root->m_size;
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
int calculateChildIndex(const Octree* node, const glm::vec3& position) {
    int x = position.x >= (node->m_min.x + node->m_size / 2) ? 1 : 0;
    int y = position.y >= (node->m_min.y + node->m_size / 2) ? 1 : 0;
    int z = position.z >= (node->m_min.z + node->m_size / 2) ? 1 : 0;
    return octreeIndex(x, y, z);
}

Octree* Octree::CreateNewSubTree(Octree* rootNode, const glm::vec3& newPosition, int chunkSize) {
    Octree* currentNode = rootNode;
    while (currentNode->m_size > chunkSize) {

        int x = newPosition.x >= (currentNode->m_min.x + currentNode->m_size / 2) ? 1 : 0;
        int y = newPosition.y >= (currentNode->m_min.y + currentNode->m_size / 2) ? 1 : 0;
        int z = newPosition.z >= (currentNode->m_min.z + currentNode->m_size / 2) ? 1 : 0;
        
        int childIndex = octreeIndex(x, y, z);

        auto children = currentNode->GetChildren();
        if (!children)
        {

            std::array<Octree *, 8> empty = {}; // new child added in the next if

            children = std::make_shared<OctreeChildren>(OctreeChildren
            {
                (uint8_t)(1 << childIndex),
                empty 
            });

            currentNode->m_children = children;
        }

        if (!children->children[childIndex]) {
            glm::ivec3 childMin = currentNode->m_min + glm::ivec3(x * currentNode->m_size / 2, y * currentNode->m_size / 2, z * currentNode->m_size / 2);
            int newNodeSize = currentNode->m_size / 2;
            children->children[childIndex] = new Octree(1, newNodeSize, childMin);
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

    // Check if the node is outside the root node
    if (position.x < currentNode->m_min.x || position.y < currentNode->m_min.y || position.z < currentNode->m_min.z || position.x >= currentNode->m_min.x + currentNode->m_size || position.y >= currentNode->m_min.y + currentNode->m_size || position.z >= currentNode->m_min.z + currentNode->m_size)
    {
        return nullptr;
    }

    // Extra check for case where there is just one node and its not the one we were looking for
    if (currentNode->m_size == chunkSize)
    {
        if (currentNode->m_min != position)
        {
            return nullptr;
        }
        return currentNode;
    }

    while (currentNode)
    {
        if (currentNode->IsLeaf())
        {
            return currentNode;
        }

        std::shared_ptr<OctreeChildren> children = currentNode->GetChildren();

        if (!children || !children->field)
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


std::vector<std::pair<Octree*, Direction>> FindNeighbors(Octree *rootNode, glm::ivec3 nodePos, int nodeSize)
{
    std::vector<std::pair<Octree*, Direction>> neighbors;
    std::vector<std::pair<glm::ivec3, Direction>> neighborPositions = {
        {nodePos - glm::ivec3(nodeSize, 0, 0), Direction::xplus}, // xplus
        {nodePos + glm::ivec3(nodeSize, 0, 0), Direction::xminus}, // xminus
        {nodePos - glm::ivec3(0, nodeSize, 0), Direction::yplus},  // yplus
        {nodePos + glm::ivec3(0, nodeSize, 0), Direction::yminus}, // yminus
        {nodePos - glm::ivec3(0, 0, nodeSize), Direction::zplus},  // zplus
        {nodePos + glm::ivec3(0, 0, nodeSize), Direction::zminus}  // zminus
    };

    for (const auto &posDir : neighborPositions)
    {
        Octree *neighbor = FindNode(rootNode, posDir.first, nodeSize);
        if (neighbor)
        {
            neighbors.push_back(std::make_pair(neighbor, posDir.second));
        }
    }

    return neighbors;
}
