//
// Created by Teemu Sarapisto on 20/07/2017.
//

#ifndef PROJECT_OCTREE_H
#define PROJECT_OCTREE_H

#include "svd.h"
#include "qef.h"

#include <vector>
#include <array>
#include <memory>

#include <glm/glm.hpp>

#include <vector_types.h> // cuda vector types

#include "Mesh.h"

struct OctreeChildren;

class Octree {
public:
	Octree(const int maxResolution, const int size, const glm::vec3 min);
	Octree(const int resolution, glm::vec3 min); // Leaf node
	Octree(std::unique_ptr<OctreeChildren> children, int size, glm::vec3 min, int resolution);
	~Octree();

	// Disallow constructors
	Octree(const Octree&) = delete;
	Octree& operator=(const Octree&) = delete;

	void Construct();

	void ConstructBottomUp();
	void MeshFromOctree(IndexBuffer& indexBuffer, VertexBuffer& vertexBuffer);
	OctreeChildren* GetChildren() const;

	const glm::ivec3 m_min;
	const int m_size;
	bool IsLeaf() const;

	// Draw info. TODO: Moving these behind a pointer should save space.
	int m_index;
	svd::QefData m_qef;
	glm::vec3 m_drawPos;
	glm::vec3 m_averageNormal;
	int m_corners;

	static void CellChildProc(const std::array<Octree*, 8>& children, IndexBuffer& indexBuffer);
	static void GenerateVertexIndices(Octree* node, VertexBuffer& vertexBuffer);
	static void FaceProcX(Octree*, Octree*, IndexBuffer& indexBuffer);
	static void FaceProcY(Octree*, Octree*, IndexBuffer& indexBuffer);
	static void FaceProcZ(Octree*, Octree*, IndexBuffer& indexBuffer);
private:
	void CellProc(IndexBuffer& indexBuffer);
	static void EdgeProcXY(Octree*, Octree*, Octree*, Octree*, IndexBuffer& indexBuffer);
	static void EdgeProcXZ(Octree*, Octree*, Octree*, Octree*, IndexBuffer& indexBuffer);
	static void EdgeProcYZ(Octree*, Octree*, Octree*, Octree*, IndexBuffer& indexBuffer);
	static void ProcessEdge(const Octree* node[4] , int dir, IndexBuffer& indexBuffer);

	std::unique_ptr<OctreeChildren> m_children;

	std::vector<std::vector<float>> m_sampleCache;
	std::vector<float4>* m_sampleCacheCuda;

	glm::vec3 GetSurfaceNormal(const glm::vec3& p);
	glm::vec3 ApproximateZeroCrossingPosition(const glm::vec3& p0, const glm::vec3& p1);

	Octree* ConstructLeafParent(const int resolution, const glm::vec3 min);
	Octree* ConstructLeaf(const int resolution, glm::vec3 min);


	const int m_resolution;
	bool m_leaf;
};

struct OctreeChildren {
	uint8_t field;
	std::array<Octree*, 8> children;
};

int index(int x, int y, int z, int dimensionLength);


#endif //PROJECT_OCTREE_H
