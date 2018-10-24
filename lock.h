#ifndef LOCK_HEADER
#define LOCK_HEADER

#ifdef _OPENMP
#	include <omp.h>
#endif

class Lock
{
	private:
	#ifdef _OPENMP
		omp_lock_t ompLock;
	#endif

	public:
	Lock();
	~Lock();
	void lock();
	void unlock();
};

#endif /* LOCK_HEADER */
