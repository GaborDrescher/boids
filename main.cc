#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include "dmath.h"
#include "boid.h"
#include "gridhash.h"
#include "painter.h"

class SimSettings
{
	public:
	uintptr_t nBoids;
	real timeStep;

	real nearRadius;
	real nearRadiusSq;
	real separationFactor;
	real alignmentFactor;
	real cohesionFactor;

};

static uint64_t getms()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

    return ((uint64_t)tp.tv_sec) * 1000 + ((uint64_t)tp.tv_nsec) / 1000000;
}

static void initBoids(Boid *boids, SimSettings *settings)
{
	uintptr_t nBoids = settings->nBoids;
	real sep = settings->nearRadius * real(10.0);
	for(uintptr_t i = 0; i < nBoids; ++i) {
		boids[i].pos.x = (rand() / real(RAND_MAX)) * sep;
		boids[i].pos.y = (rand() / real(RAND_MAX)) * sep;
		boids[i].pos.z = (rand() / real(RAND_MAX)) * sep;
	}
}

static void calcForce(Boid *boid, GridHash *gridHash, SimSettings *settings)
{
	const real separationFactor = settings->separationFactor;
	const real nearRadiusSq = settings->nearRadiusSq;

	// get the neighbours
	const uintptr_t neighbourNodes = 3*3*3;
	HashNode *nearNodes[neighbourNodes];
	gridHash->getNodes(boid, nearNodes);


	// iterate over all neighbour nodes, get center position and velocity
	// calculate forces for separation
	Vec centerPos;
	Vec centerVel;
	uintptr_t numNeighbours = 0;
	for(uintptr_t i = 0; i < neighbourNodes; ++i) {
		HashNode *currentNode = nearNodes[i];
		if(currentNode == nullptr) {
			continue;
		}

		Boid *other = currentNode->boids;
		for(; other != nullptr; other = other->hashNext) {
			if(other == boid) {
				continue;
			}

			// check the actual distance
			Vec diff = boid->pos - other->pos;
			real distSq = diff.lengthSq();
			if(distSq > nearRadiusSq) {
				continue;
			}

			// update center pos and vel
			numNeighbours += 1;
			centerPos = centerPos + other->pos;
			centerVel = centerVel + other->vel;

			real dist = Math::sqrt(distSq);
			// if dist is (almost) zero we get crazy values here e.g. nan
			if(dist > EPSILON) {
				// calculate separation force
				// TODO is this correct?
				real distInv = real(1.0) / dist;
				Vec normal = diff * distInv;
				real forceMagnitude = dist * separationFactor;
				boid->force = boid->force + (normal * forceMagnitude);
			}
		}
	}

	// add force in direction 0,0,0, TODO this is just for entertainment
	Vec n = boid->pos * real(-1.0);
	n.normalize();
	n = n * real(10);
	boid->force = boid->force + n;


	// calculate forces for alignment an cohesion
	// TODO is this correct?
	if(numNeighbours == 0) {
		return;
	}

	centerPos = centerPos / numNeighbours;
	centerVel = centerVel / numNeighbours;
	//printf("center pos: %f %f %f\n", centerPos.x, centerPos.y, centerPos.z);
	//printf("center vel: %f %f %f\n", centerVel.x, centerVel.y, centerVel.z);

	{
		Vec normal = centerPos - boid->pos;
		real length = normal.normalize();
		if(length > EPSILON) {
			real forceMagnitude = length * settings->cohesionFactor;
			boid->force = boid->force + (normal * forceMagnitude);
		}
	}
	{
		Vec normal = centerVel - boid->vel;
		real length = normal.normalize();
		if(length > EPSILON) {
			real forceMagnitude = length * settings->alignmentFactor;
			boid->force = boid->force + (normal * forceMagnitude);
		}
	}

	//printf("force: %f %f %f\n", boid->force.x, boid->force.y, boid->force.z);
}

int main()
{
	uintptr_t nBoids = 20000;
	uintptr_t maxIter = 1000000;
	real timeStep = 0.01;

	SimSettings settings;
	settings.nBoids = nBoids;
	settings.timeStep = timeStep;
	settings.nearRadius = 10.0;
	settings.nearRadiusSq = settings.nearRadius * settings.nearRadius;
	settings.separationFactor = 0.1;
	settings.alignmentFactor = 1.0;
	settings.cohesionFactor = 10.0;

	Painter painter(1024, 768);

	Boid *boids = new Boid[nBoids];
	initBoids(boids, &settings);

	GridHash gridHash;

	for(uintptr_t iter = 0; iter < maxIter; ++iter) {

		uint64_t time = getms();
		// calc min and max positions
		Vec minPos = boids[0].pos;
		Vec maxPos = boids[0].pos;
		for(uintptr_t i = 1; i < nBoids; ++i) {
			minPos = Vec::min(minPos, boids[i].pos);
			maxPos = Vec::max(maxPos, boids[i].pos);
		}
		if(iter == 0) {
			painter.setScale(minPos, maxPos);
		}

		//printf("min pos: %f %f %f\n", minPos.x, minPos.y, minPos.z);
		//printf("max pos: %f %f %f\n\n", maxPos.x, maxPos.y, maxPos.z);


		// clear/init GridHash
		gridHash.setDims(minPos, maxPos, settings.nearRadius, nBoids * 2);

		// add boids to GridHash in parallel
		#pragma omp parallel for
		for(uintptr_t i = 0; i < nBoids; ++i) {
			gridHash.add(&boids[i]);
		}
		// implicit barrier

		// calculate forces in parallel
		#pragma omp parallel for schedule(dynamic, 1)
		for(uintptr_t i = 0; i < nBoids; ++i) {
			calcForce(&boids[i], &gridHash, &settings);
		}
		// implicit barrier
		
		// advance boids in parallel
		#pragma omp parallel for
		for(uintptr_t i = 0; i < nBoids; ++i) {
			boids[i].vel = boids[i].vel + (boids[i].force * timeStep);
			boids[i].pos = boids[i].pos + (boids[i].vel * timeStep);
			boids[i].force.zero();
		}
		// implicit barrier
		time = getms() - time;
		printf("calc time for %" PRIuPTR " boids: %" PRIu64 "ms\n", nBoids, time);

		painter.paint(boids, nBoids);
	}

	delete [] boids;

	return 0;
}
