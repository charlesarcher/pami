/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/shmem/SharedMemoryManager.h
/// \brief Shared memory manager class
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_memory_shmem_SharedMemoryManager_h__
#define __components_memory_shmem_SharedMemoryManager_h__

#include "components/memory/MemoryManager.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI
{
  namespace Memory
  {
    class SharedMemoryManager : public MemoryManager
    {
	private:
#ifdef MM_DEBUG // set/unset in MemoryManager.h
		inline void MM_RESET_STATS() {
			_num_allocs = 0;
			_num_frees = 0;
			_loc_bytes = 0;
			_rep_bytes = 0;
		}
		inline void MM_DUMP_STATS() {
			fprintf(stderr, "SharedMemoryManager: %zd allocs, %zd frees, "
					"local %zdb, repeat %zdb\n",
				_num_allocs, _num_frees,
				_loc_bytes, _rep_bytes);
		}
#endif // MM_DEBUG
      public:

	/// \brief This class is a wrapper for the shm_open/ftruncate/mmap sequence
	///
	/// Unlike general MemoryManagers, it does not actually contain any memory
	/// instead it provides an interface into the OS's shared memory pool.
	///
	/// Because shm segments persist after the job exits (on some platforms)
	/// this will need to keep track of all allocations so that the destructor
	/// can unlink them all. This only happens when the ref_count reaches zero.
	///
	/// Support for free() is not provided yet, but allocations must still be
	/// tracked for the afore mentioned reason.
	///
        inline SharedMemoryManager(size_t jobid, MemoryManager *mm) :
          MemoryManager (),
	  _meta(),
	  _jobid(jobid)
        {
		// This could be better decided based on number of processes
		// per node, but can't get that from __global because of
		// circular dependencies. Instead, Global could check and
		// simply not construct SharedMemoryManager in shared_mm
		// (construct HeapMemoryManager twice, in heap_mm and shared_mm).
		_attrs = PAMI_MM_NODESCOPE;
		_meta.init(mm, "/pami-shmemmgr");
		_enabled = true;
#ifdef MM_DEBUG
		_debug = (getenv("PAMI_MM_DEBUG") != NULL);
		MM_RESET_STATS();
#endif // MM_DEBUG
        }

        ~SharedMemoryManager ()
        {
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
		}
#endif // MM_DEBUG
		// if this only happens at program exit, just unlink all keys...

		// freeAll(); // needs to be done before/in _meta.~MemoryManagerMeta which
		// frees all the meta data buffers!

		// could free up all the meta data, but it is in heap and
		// about to be freed by _exit().
		// _meta.~MemoryManagerMeta<MemoryManagerOSShmAlloc>(); // already called!?
        }

	inline const char *getName() { return "SharedMemoryManager"; }

	inline pami_result_t init (MemoryManager *mm, size_t bytes,
			size_t alignment, size_t new_align,
			unsigned attrs = 0, const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
		PAMI_abortf("SharedMemoryManager cannot be init()");
		return PAMI_ERROR;
	}

	inline pami_result_t reset(bool force = false) {
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
			MM_RESET_STATS();
		}
#endif // MM_DEBUG
		pami_result_t rc = PAMI_SUCCESS;
		if (force) rc = _meta.reset(); // acquires lock...
		return rc;
	}

	/// \todo How to enforce alignment?
	inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes,
			const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{

		PAMI_assert_debugf(_attrs == PAMI_MM_NODESCOPE, "SharedMemoryManager not shared");
		// May need to enforce uniquness at a higher level than just this
		// PAMI job. May need to acquire a unique prefix from, say, Mapping
		// that ensures the underlying OS shmem segment will not conflict
		// with any other jobs that might be running.
		//
		char nkey[MMKEYSIZE];
		if (key && key[0]) {
			if (*key == '/') ++key; // or... allow "relative" vs. "absolute" keys?
			snprintf(nkey, sizeof(nkey), "/job%zd-%s", _jobid, key);
		} else {
			// this should be unique... or not? memptr might not be...
			snprintf(nkey, sizeof(nkey), "/job%zd-pid%d-%lx", _jobid,
						getpid(), (unsigned long)memptr);
			if (key) {
				// callers wants to know the unique key we chose...
				strcpy((char *)key, nkey);
			}
		}
		// ... use 'nkey' here-after...
		//
		if (alignment < _alignment) alignment = _alignment;
		void *ptr = NULL;
		bool first = false;
		_meta.acquire(); // only makes this thread-safe, not proc-safe.
		MemoryManagerOSShmAlloc *alloc = _meta.findFree();
		if (alloc == NULL) {
			_meta.release();
			return PAMI_ERROR;
		}
		alloc->key(nkey);
		alloc->userSize(bytes, alignment);
		// note, inital (worst-case) padding now set, when
		// actual pointer assigned below, padding is updated.

		// Note, Global does not construct this if the target conditions
		// would prevent it from succeeding - for example SMP-mode on BG.
		// So that simplifies the code, as any errors are just fatal.

		// should we keep track of each shm_open, so that we can
		// later shm_unlink?

		// first try to create the file exclusively, if that
		// succeeds then we know we should initialize it.
		// However, we still need to ensure others do not
		// start using the memory until we complete the init.
		// Use a "counter mutex" that is initialized to 0, all but
		// the first (based on O_EXCL) will wait on it.
		// 

		// use GCC atomics on the shared memory chunk, in order to
		// synchronize init, and in free will need to ensure memory gets zeroed.
		int lrc;

		lrc = shm_open(alloc->key(), O_CREAT | O_EXCL | O_RDWR, 0600);
		first = (lrc != -1); // must be the first...

		if (!first) lrc = shm_open(alloc->key(), O_RDWR, 0);
		if (lrc == -1)
		{
			alloc->free();
			_meta.release();
			return PAMI_ERROR;
		}
		alloc->fd(lrc); // mmap (et al.) requires this first.
		lrc = ftruncate(alloc->fd(), alloc->size());
		if (lrc == 0) {
			ptr = mmap(NULL, alloc->size(), PROT_READ | PROT_WRITE, MAP_SHARED,
									alloc->fd(), 0);
		}
		if (ptr == NULL || ptr == MAP_FAILED)
		{
			// segment is not mapped...
			alloc->free();
			_meta.release();
			return PAMI_ERROR;
		}
		// shared segment acquired and mapped, now sync and init.

		alloc->mem(ptr, alignment); // required for addRef(), userMem(), etc...
		alloc->addRef();
		_meta.release();
		if (init_fn) {
			if (first) {
				init_fn(alloc->userMem(), alloc->userSize(),
						alloc->key(), _attrs, cookie);
				alloc->initDone();
			} else {
				alloc->waitDone();
			}
		}
		*memptr = alloc->userMem();
#ifdef MM_DEBUG
		if (_debug) {
			++_num_allocs;
			size_t alloc_bytes = alloc->userSize();
			if (first) {
				_loc_bytes += alloc_bytes;
			} else {
				_rep_bytes += alloc_bytes;
			}
		}
#endif // MM_DEBUG
		return PAMI_SUCCESS;
	}

	inline void free(void *mem) {
		_meta.acquire();
		MemoryManagerOSShmAlloc *m = _meta.find(mem);
		if (m) {
#ifdef MM_DEBUG
			if (_debug) {
				++_num_frees;
				_rep_bytes -= m->userSize();
			}
#endif // MM_DEBUG
			m->free();
		}
		_meta.release();
	}

	inline size_t available(size_t alignment) {
		// how to tell how much is available???
		// BGQ: getenv(BG_SHAREDMEMSIZE), minus something...
		return (size_t)-1;
	}

	inline void dump(const char *str) {
		if (str) {
			fprintf(stderr, "%s: SharedMemoryManager %x\n", str,
					_attrs);
		} else {
			fprintf(stderr, "SharedMemoryManager %x\n",
					_attrs);
		}
	}

    protected:

	MemoryManagerMeta<MemoryManagerOSShmAlloc> _meta;
	size_t _jobid;
#ifdef MM_DEBUG
	bool _debug;
	size_t _num_allocs;
	size_t _num_frees;
	size_t _loc_bytes;
	size_t _rep_bytes;
#endif // MM_DEBUG

    }; // class SharedMemoryManager
  }; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
