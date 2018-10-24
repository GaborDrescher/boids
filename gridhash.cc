#include "gridhash.h"
#include "lock.h"
#include <algorithm> // for std::fill


GridHash::GridHash()
{
	hashArray = nullptr;
	hashLocks = nullptr;
	hashSize = 0;
}

GridHash::~GridHash()
{
	if(hashArray != nullptr) {
		for(uintptr_t i = 0; i < hashSize; ++i) {
			HashNode *node = hashArray[i];

			while(node != nullptr) {
				HashNode *next = node->next;
				delete node;
				node = next;
			}
		}
	}

	delete [] hashLocks;
	delete [] hashArray;
}

HashNode* GridHash::getNewNode(const UIntVec &newPos)
{
	HashNode *out = new HashNode();
	out->init(newPos);
	return out;
}

void GridHash::freeNode(HashNode *node)
{
	delete node;
}

void GridHash::setDims(const Vec &min, const Vec &max, real searchDist,
	uintptr_t hashSize)
{
	// clear old state
	if(hashArray != nullptr) {
		for(uintptr_t i = 0; i < hashSize; ++i) {
			HashNode *node = hashArray[i];

			while(node != nullptr) {
				HashNode *next = node->next;
				freeNode(node);
				node = next;
			}
		}
	}

	this->cellSize = searchDist * real(0.5);
	this->cellSizeInv = real(1.0) / this->cellSize;

	Vec cellSizeVec(this->cellSize, this->cellSize, this->cellSize);

	// increase by another cell size to avoid corner cases later
	this->min = min - cellSizeVec;
	this->min = max + cellSizeVec;

	this->size = max - min;

	if(this->hashSize != hashSize || hashArray == nullptr) {
		delete [] hashLocks;
		delete [] hashArray;

		this->hashSize = hashSize;
		hashArray = new HashNode*[hashSize];

		hashLocks = new Lock[hashSize];
	}

	// clear state
	std::fill(hashArray, hashArray + hashSize, (HashNode*)nullptr);
}

void GridHash::add(Boid *boid)
{
	// calc cell position of boid
	const UIntVec intCoords((boid->pos - min) * cellSizeInv);

	// calc hash to look for HashNode
	uintptr_t hashIdx = intCoords.hash() % hashSize;

	// get the lock for the hash bucket
	hashLocks[hashIdx].lock();
		// search for an existing HashNode
		HashNode *current = hashArray[hashIdx];
		for(; current != nullptr; current = current->next) {
			if(current->pos.equals(intCoords)) {
				break;
			}
		}

		// current is now nullptr or the right HashNode
		if(current == nullptr) {
			// create a new HashNode and add it to the hash array
			current = getNewNode(intCoords);
			current->next = hashArray[hashIdx];
			hashArray[hashIdx] = current;
		}

		// add the boid to the boid list
		boid->hashNext = current->boids;
		current->boids = boid;

	hashLocks[hashIdx].unlock();
}

void GridHash::getNodes(const Boid *boid, HashNode *outNodes[]) const
{
	// calc cell position of boid
	UIntVec intCoords((boid->pos - min) * cellSizeInv);

	intCoords.x -= 1;
	intCoords.y -= 1;
	intCoords.z -= 1;

	UIntVec tmp(intCoords);

	uintptr_t outIdx = 0;
	for(uintptr_t z = 0; z < 3; ++z) {
		for(uintptr_t y = 0; y < 3; ++y) {
			tmp.y += y;
			for(uintptr_t x = 0; x < 3; ++x) {

				uintptr_t hashIdx = tmp.hash() % hashSize;
				HashNode *current = hashArray[hashIdx];
				for(; current != nullptr; current = current->next) {
					if(current->pos.equals(tmp)) {
						break;
					}
				}
				outNodes[outIdx] = current;
				outIdx += 1;
				

				tmp.x += 1;
			}
			tmp.x = intCoords.x;
			tmp.y += 1;
		}
		tmp.y = intCoords.y;
		tmp.z += 1;
	}
}
