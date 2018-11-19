//
// Created by Teemu Sarapisto on 20/07/2017.
//

#ifndef PROJECT_OCTREE_H
#define PROJECT_OCTREE_H

#include <vector>
#include <array>
#include <memory>

#include <glm/glm.hpp>

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

private:
	void ConstructBottomUp(const int maxResolution, const int size, const glm::vec3 min);
	
	static Octree* ConstructLeaf(const int resolution, const glm::vec3 min);
	static bool Sample(const glm::vec3 pos);

	void CellProc();
	void FaceProc(const Octree&, const Octree&);
	void EdgeProc(const Octree&, const Octree&, const Octree&, const Octree&);
	void ProcessEdge(const Octree&, const Octree&, const Octree&, const Octree&);

	void MeshFromOctree();
	void GenerateVertexIndices();

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
