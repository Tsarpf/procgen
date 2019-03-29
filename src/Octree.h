//
// Created by Teemu Sarapisto on 20/07/2017.
//

#ifndef PROJECT_OCTREE_H
#define PROJECT_OCTREE_H

#include <vector>
#include <array>
#include <memory>

#include <glm/glm.hpp>
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};
typedef std::vector<int> IndexBuffer;
typedef std::vector<Vertex> VertexBuffer;


struct OctreeChildren;

class Octree {
public:
	//Octree(int size, glm::vec3 min, OctreeChildren* children);
	Octree(const int maxResolution, const int size, const glm::vec3 min);

	Octree(const int resolution, glm::vec3 min); // Leaf node
	Octree(std::unique_ptr<OctreeChildren> children, int size, glm::vec3 min, int resolution);
	~Octree();

    bool HasSomethingToRender();

	OctreeChildren* GetChildren() const;

	const glm::vec3 m_min;
	const int m_size;
	int m_index;
	bool IsLeaf() const;

private:
	void ConstructBottomUp(const int maxResolution, const int size, const glm::vec3 min);
	
	static Octree* ConstructLeaf(const int resolution, const glm::vec3 min);
	static bool Sample(const glm::vec3 pos);

	void CellProc(IndexBuffer& indexBuffer);
	void FaceProcX(const Octree&, const Octree&, IndexBuffer& indexBuffer);
	void FaceProcY(const Octree&, const Octree&, IndexBuffer& indexBuffer);
	void FaceProcZ(const Octree&, const Octree&, IndexBuffer& indexBuffer);
	void EdgeProcXY(const Octree&, const Octree&, const Octree&, const Octree&, IndexBuffer& indexBuffer);
	void EdgeProcXZ(const Octree&, const Octree&, const Octree&, const Octree&, IndexBuffer& indexBuffer);
	void EdgeProcYZ(const Octree&, const Octree&, const Octree&, const Octree&, IndexBuffer& indexBuffer);
	void ProcessEdge(const Octree node[4] , int dir, IndexBuffer& indexBuffer);

	void MeshFromOctree(IndexBuffer& indexBuffer);
	void GenerateVertexIndices(VertexBuffer& vertexBuffer);

	std::unique_ptr<OctreeChildren> m_children;

	Octree(const Octree&);
	Octree& operator=(const Octree&);

	const int m_resolution;
	bool m_leaf;
};

struct OctreeChildren {
	uint8_t field;
	std::array<Octree*, 8> children;
};


#endif //PROJECT_OCTREE_H
