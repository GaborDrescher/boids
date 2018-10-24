#ifndef GRID_HASH_HEADER
#define GRID_HASH_HEADER

#include <inttypes.h>
#include "boid.h"
#include "dmath.h"

class Lock;
class GridHash;

class HashNode
{
	friend class GridHash;
	private:
	HashNode *next;
	UIntVec pos;

	public:
	HashNode()
	{
	}

	void init(const UIntVec &newPos)
	{
		next = nullptr;
		pos = newPos;
		boids = nullptr;
	}

	Boid *boids;
};

class GridHash
{
	private:
	Vec min;
	Vec max;
	Vec size;
	real cellSize;
	real cellSizeInv;

	HashNode **hashArray;
	Lock *hashLocks;
	uintptr_t hashSize;

	HashNode* getNewNode(const UIntVec &newPos);
	void freeNode(HashNode *node);

	public:
	GridHash();
	~GridHash();

	void setDims(const Vec &min, const Vec &max, real searchDist,
		uintptr_t hashSize);

	void add(Boid *boid);

	void getNodes(const Boid *boid, HashNode *outNodes[]) const;
};


#endif /* GRID_HASH_HEADER */
