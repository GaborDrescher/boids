#include "lock.h"

Lock::Lock()
{
	#ifdef _OPENMP
		omp_init_lock(&ompLock);
	#endif
}

Lock::~Lock()
{
	#ifdef _OPENMP
		omp_destroy_lock(&ompLock);
	#endif
}

void Lock::lock()
{
	#ifdef _OPENMP
		omp_set_lock(&ompLock);
	#endif
}

void Lock::unlock()
{
	#ifdef _OPENMP
		omp_unset_lock(&ompLock);
	#endif
}
