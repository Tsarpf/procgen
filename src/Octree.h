//
// Created by Teemu Sarapisto on 20/07/2017.
//

#ifndef PROJECT_OCTREE_H
#define PROJECT_OCTREE_H

#include <vector>
#include <array>

#include <glm/glm.hpp>

class Octree {
public:
	Octree(int size, glm::vec3 min, OctreeChildren* children);
	Octree(const int maxResolution, const int size, const glm::vec3 min);
	~Octree();

    bool HasSomethingToRender();
private:
	void ConstructLeaf(const int resolution, const glm::vec3 min);
	void ConstructBottomUp(const int maxResolution, const int size, const glm::vec3 min);

    OctreeChildren* m_children;
	//std::array<Octree*, 8> m_children;
    //int8_t m_childField;
};

enum OldPosition {
	UPPERLEFT,
	LOWERRIGHT
};

struct OctreeChildren {
	int8_t field;
	std::array<Octree*, 8> children;
};


#endif //PROJECT_OCTREE_H
