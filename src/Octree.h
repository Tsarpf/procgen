//
// Created by Teemu Sarapisto on 20/07/2017.
//

#ifndef PROJECT_OCTREE_H
#define PROJECT_OCTREE_H

#include <vector>

class Octree {
public:
	Octree(int size, int min, std::vector<Octree*> children, int8_t childField);
	Octree(const int maxResolution, const int size, const int min);
    Octree(int min);
	~Octree();

    bool HasSomethingToRender();
private:
	void ConstructLeaf(int min);
	void ConstructBottomUp(const int maxResolution, const int size, const int min);
};

enum OldPosition {
	UPPERLEFT,
	LOWERRIGHT
};


#endif //PROJECT_OCTREE_H
