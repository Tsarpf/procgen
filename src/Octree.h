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
	Octree(int size, int min, std::array<Octree*, 8> Children, int8_t ChildField);
	Octree(const int maxResolution, const int size, const int min);
	~Octree();

    bool HasSomethingToRender();
private:
	void ConstructLeaf(const int resolution, const int min);
	void ConstructBottomUp(const int maxResolution, const int size, const int min);
	std::array<Octree*, 8> m_children;
    int8_t m_childField;
};

enum OldPosition {
	UPPERLEFT,
	LOWERRIGHT
};


#endif //PROJECT_OCTREE_H
