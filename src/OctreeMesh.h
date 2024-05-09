#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mutex>
#include <future>
#include <queue>

#include "Octree.h"
#include "Mesh.h"
#include "utils.h"
#include "OctreeVisualization.h"

enum Direction {
	xplus, xminus,
	yplus, yminus,
	zplus, zminus,
	nodir
};

class OctreeMesh : public Mesh
{
public:
	//OctreeMesh();
	OctreeMesh(GLuint program, const int size, const glm::vec3 position, const uint16_t chunkSize);
	OctreeMesh(GLuint program, const int size, const glm::vec3 position, Octree* tree, VertexBuffer vertices, IndexBuffer indices);
	~OctreeMesh();
	void LoadMesh();

	void Draw(const float time, uint32_t mode) override;
	void BuildOctree();

	void Enlarge(Direction dir, uint16_t chunkSize);
	glm::ivec3 AddNewChunk(glm::ivec3 chunkCursor, Direction dir, uint16_t chunkSize);
	glm::ivec3 AddNewChunk(glm::ivec3 newPosition, uint16_t chunkSize);

	void CheckResults();

	Octree* GetOctree();

	// Chunk processing
	void SpiralGenerate(int width, int height, bool generateColumns);
private:
	// Chunk processing
	void ProcessQueue();
	void WaitForMeshCompletion();
	std::queue<glm::ivec3> chunkQueue;
    std::mutex queueMutex;
	uint16_t m_chunkSize;

	std::tuple<int, int, glm::vec3> EnlargeCorners(Direction dir);

	std::vector<std::future<OctreeMesh*>> m_futureMeshes;
	std::mutex m_childMeshMutex;
	Octree* m_tree;
	glm::vec3 m_position;
	int m_size;
	OctreeVisualization m_visualization;
	std::vector<OctreeMesh*> m_childMeshes;
};
