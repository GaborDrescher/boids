#ifndef BOID_HEADER
#define BOID_HEADER

#include "dmath.h"

class Boid
{
	public:
	Boid *hashNext;

	Vec pos;
	Vec vel;
	Vec force;

	Boid() : hashNext(nullptr)
	{
	}
};

#endif /* BOID_HEADER */
