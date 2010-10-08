/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/MemoryManager.h
/// \brief Base memory manager class
///
#ifndef __components_memory_MemoryManager_h__
#define __components_memory_MemoryManager_h__

#include <pami.h>
#include "util/common.h"
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "spi/include/kernel/location.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

#define MM_DEBUG

// need a "name" for each MemoryManager, in order to debug effectively.
// But HeapMemoryManager and SharedMemoryManager debug is still useful.

namespace PAMI
{
  namespace Memory
  {
    static const unsigned int PAMI_MM_PROCSCOPE = 1;
    static const unsigned int PAMI_MM_NODESCOPE = 2;
    static const unsigned int PAMI_MM_SCOPE = (PAMI_MM_PROCSCOPE|PAMI_MM_NODESCOPE);
    // how do platforms add more?
    static const unsigned int PAMI_MM_L2ATOMIC  = 4; // BGQ-specific
    static const unsigned int PAMI_MM_WACREGION = 8; // BGQ-specific

    static const unsigned int MMKEYSIZE = 128;
    typedef void MM_INIT_FN(void *mem, size_t bytes, const char *key, unsigned attrs, void *cookie);

    // internal class, templatized to allow varied use
    class MemoryManager
    {
      public:

	// is this neded? do we always ensure memory is zero?
	static void memzero(void *mem, size_t bytes, const char *key, unsigned attrs, void *cookie)
	{
		memset(mem, 0, bytes);
	}
      protected:
	/// \brief compute the padding needed to provide user with aligned addr
	///
	/// \param[in] base	Start address of raw chunk
	/// \param[in] off	Addition (meta) data needed
	/// \param[in] align	Alignment required (power of two - NOT zero!)
	///
	static inline size_t padding(void *base, size_t off, size_t align) {
		return (((size_t)base + off + (align - 1)) & ~(align - 1)) -
			(size_t)base;
	}

	class MemoryManagerSync {
	public:
		MemoryManagerSync() { }

		inline size_t addRef() {
			return __sync_fetch_and_add(&_ref_count, 1);
		}
		inline size_t rmRef() {
			return __sync_fetch_and_add(&_ref_count, -1);
		}
		inline size_t refCount() {
			return _ref_count;
		}
		inline void initDone() { _init_done = 1; }

		// must be called without lock!
		inline void waitDone() {
			while (_init_done == 0) { }
		}
	private:
		volatile size_t _ref_count;
		volatile size_t _init_done;
	}; // class MemoryManagerSync

	class MemoryManagerKey {
	public:
		MemoryManagerKey() { }

		inline void key(const char *key) {
			if (key) {
				strncpy((char *)_key, key, MMKEYSIZE);
			} else {
				// whatever this does, _key[0] must not be 0...
				snprintf((char *)_key, MMKEYSIZE, "/pami-%d-%p",
							getpid(), this);
			}
			// assert(!isFree());
		}
		inline char *key() { return (char *)_key; }

		inline bool isMatch(const char *key) {
			return (strncmp((char *)_key, key, MMKEYSIZE) == 0);
		}
		inline void freeKey() { _key[0] = '\0'; }
		inline bool isFree() { return (_key[0] == '\0'); }
	private:
		volatile char _key[MMKEYSIZE];
	}; // class MemoryManagerKey

	/// \brief Class to hold the meta-data portion of an allocation that is shared
	///
	/// Anonymous (private) chunks do not use this object
	///
	/// Can't use util/queue/*Queue.h since they all depend on MemoryManager.
	///
	class MemoryManagerAlloc :	public MemoryManagerSync,
					public MemoryManagerKey {
	public:
		static const size_t ALIGNMENT = sizeof(void *);
		MemoryManagerAlloc() { }
		virtual ~MemoryManagerAlloc() { }

		inline size_t offset() { return _offset; }
		inline void offset(size_t off) { _offset = off; }
		inline void rawOffset(size_t off) {
			_rawSize = (_offset - off) + _userSize;
		}

		inline void free() { memset(this, 0, sizeof(*this)); }
		static void static_free(MemoryManagerAlloc *m, void *v) {
			m->free();
		}

		inline void *userMem() { return (void *)-1; }
		inline size_t rawSize() { return _rawSize; }
		inline size_t userSize() { return _userSize; }
		inline void userSize(size_t size, size_t align) {
			_userSize = size;
		}
	private:
		volatile size_t _offset; // _base + _offset = memory chunk
		volatile size_t _userSize;
		volatile size_t _rawSize;
	}; // class MemoryManagerAlloc

	class MemoryManagerOSShmAlloc : public MemoryManagerKey {
	public:
		static const size_t ALIGNMENT = sizeof(void *);
		MemoryManagerOSShmAlloc() { }
		virtual ~MemoryManagerOSShmAlloc() { }

		// caller holds lock...
		inline void free() {
			close(_fd);
			if (sync() && rmRef() == 1) { // last one out...
				// zero memory so that next use is zeroed?
				// memset(rawAddress(), 0, size());
				shm_unlink(key());
			}
			freeKey();
		}

		// caller holds lock...
		static void static_free(MemoryManagerOSShmAlloc *m, void *v) {
			m->free();
		}

		inline void fd(int fd) { _fd = fd; }
		inline int fd() { return _fd; }
		inline void vkey(uint32_t key) { _vkey = key; }
		inline uint32_t vkey() { return _vkey; }
		inline void mem(void *mem, size_t align) {
			_mem = mem;
			_pad = padding(mem, sizeof(MemoryManagerSync), align);
		}
		inline void *mem() { return _mem; }
		inline size_t size() { return _size; }

		inline size_t offset() { return (size_t)-1; }
		inline void offset(size_t off) { }
		inline void rawOffset(size_t off) { }

		inline void *userMem() { return (char *)_mem + _pad; }
		inline size_t userSize() { return _userSize; }
		inline size_t rawSize() { return (size_t)-1; }
		inline void userSize(size_t size, size_t align) {
			_userSize = size;
			_pad = sizeof(MemoryManagerSync) + align; // worst-case
			_size = size + _pad;
		}

		inline void initDone() { sync()->initDone(); }
		inline void waitDone() { sync()->waitDone(); }
		inline size_t addRef() { return sync()->addRef(); }
		inline size_t rmRef() { return sync()->rmRef(); }
	private:
		inline MemoryManagerSync *sync() { return (MemoryManagerSync *)_mem; }
		void *_mem;
		size_t _size;
		size_t _userSize;
		size_t _pad;
		uint32_t _vkey; // SysV Shm only
		int _fd; // mmap fd or shmget id
	}; // class MemoryManagerOSShmAlloc

	/// \brief Class to contain the root header of a MemoryManager
	///
	/// Can't use util/queue/*Queue.h or components/atomic/*... since they all
	/// depend on MemoryManager.
	///
	class MemoryManagerHeader {
	public:
		MemoryManagerHeader() :
		_mutex(0),
		_offset(0),
		_nmetas(0)
		{
		}
		virtual ~MemoryManagerHeader() { }

		// caller holds lock!
		inline size_t offset() { return _offset; }
		inline size_t offset(size_t off) {
			size_t o = _offset;
			_offset = off;
			return o;
		}

		inline void acquire() {
			while (__sync_fetch_and_add(&_mutex, 1) != 0);
		}

		inline void release() {
			mem_sync();
			_mutex = 0;
		}

		inline size_t numMetas() { return _nmetas; }
		inline void setMeta(size_t x) { if (x >= _nmetas) _nmetas = x + 1; }
	private:
		volatile size_t _mutex;
        	volatile size_t _offset; // simple free-space handling
        	volatile size_t _nmetas; // number of _metas[] allocated globally
	}; // class MemoryManagerHeader

	// meta[0] has 8, meta[1] has 32, meta[2] has 128, etc...
	#define MM_META_NUM(idx)	((8UL << idx) << idx)
	#define MMMAX_N_META	4

	template <class T_MMAlloc = MemoryManagerAlloc>
	class MemoryManagerMeta {
	private:
		inline pami_result_t _metaAlloc(void **pptr, size_t len, char tag) {
			pami_result_t rc;
			if (_meta_key_len) {
				_meta_key_fmt[_meta_key_len] = tag; // replaced on each use
				rc = _meta_mm->memalign(pptr, 0, len, _meta_key_fmt);
			} else {
				rc = _meta_mm->memalign(pptr, 0, len, NULL);
			}
			// PAMI_assertf(rc == PAMI_SUCCESS, "Failed to get memory for meta \"%s\" from \"%s\"", _meta_key_fmt, _meta_mm->getName());
			return rc;
		}

		inline pami_result_t _getMeta(size_t x) {
			pami_result_t rc = PAMI_SUCCESS;
			if (_metas[x] == NULL) {
				rc = _metaAlloc((void **)&_metas[x],
						MM_META_NUM(x) * sizeof(*_metas[x]),
						"0123456789"[x]);
				if (rc == PAMI_SUCCESS) _metahdr->setMeta(x);
			}
			return rc;
		}
		inline pami_result_t _freeAll() {
			if (!_metahdr) return PAMI_SUCCESS;
			size_t x, y;
			for (x = 0; x < _metahdr->numMetas(); ++x) {
				if (_metas[x] == NULL) break;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (!_metas[x][y].isFree()) {
						_metas[x][y].free();
					}
				}
			}
			return PAMI_SUCCESS;
		}
	public:
		MemoryManagerMeta() :
		_meta_mm(NULL),
		_metahdr(NULL),
		_meta_key_fmt(),
		_meta_key_len(0)
		{
			memset(_metas, 0, sizeof(_metas));
		}

		virtual ~MemoryManagerMeta() {
			if (!_metahdr) return;
			acquire(); // no one else should be here...
			_freeAll();
			release(); // no one else should be here...
			if (_meta_mm->base()) { // still there?
				if (_metahdr) {
					_meta_mm->free(_metahdr); // destroys mutex
					_metahdr = NULL;
				}
				int x;
				for (x = 0; x < MMMAX_N_META; ++x) {
					if (_metas[x]) {
						if (!_pre_alloc) _meta_mm->free(_metas[x]);
						_metas[x] = NULL;
					}
				}
			}
		}

		inline pami_result_t reset() {
			acquire();
			pami_result_t rc = _freeAll();
			release();
			return rc;
		}

		inline void dump(const char *str) {
			size_t x, y;
			if (!_metahdr) return;
			for (x = 0; x < _metahdr->numMetas(); ++x) {
				if (_getMeta(x) != PAMI_SUCCESS) return;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (_metas[x][y].isFree()) { continue; }
					fprintf(stderr, "%s: _metas[%zd][%zd]: "
						"\"%s\" %p %zd (%zd/%zd)\n",
						str, x, y,
						_metas[x][y].key(),
						_metas[x][y].userMem(),
						_metas[x][y].offset(),
						_metas[x][y].userSize(),
						_metas[x][y].rawSize()
						);
				}
			}
		}

		inline void init(MemoryManager *mm, const char *key) {
			_meta_mm = mm;
			_pre_alloc = (mm == shared_mm); // minimize small allocs
			if (key) {
				_meta_key_len = strlen(key);
				strncpy(_meta_key_fmt, key, MMKEYSIZE);
				if (_meta_key_len + 3 >= MMKEYSIZE) {
					_meta_key_len = MMKEYSIZE - 3;
					// hope for the best...
				}
				_meta_key_fmt[_meta_key_len++] = '-';
				_meta_key_fmt[_meta_key_len + 1] = '\0';
			}
			pami_result_t rc;
			if (_pre_alloc) {
				rc = _metaAlloc((void **)&_metahdr, sizeof(*_metahdr), 'a');
				PAMI_assertf(rc == PAMI_SUCCESS,
					"Failed to get memory for meta data");
				int x;
				T_MMAlloc *mp = (T_MMAlloc *)(_metahdr + 1);
				for (x = 0; x < MMMAX_N_META; ++x) {
					_metas[x] = mp;
					mp += MM_META_NUM(x);
				}
			} else {
				rc = _metaAlloc((void **)&_metahdr, sizeof(*_metahdr), 'h');
				PAMI_assertf(rc == PAMI_SUCCESS,
					"Failed to get memory for _metahdr");
			}
			//new (_metahdr) MemoryManagerHeader(); // can? should?
		}

		inline void acquire() { _metahdr->acquire(); }
		inline void release() { _metahdr->release(); }

		inline size_t spaceUsed() {
			return _metahdr->offset();
		}
		inline size_t getSpace(void *base, size_t alignment, size_t bytes) {
			// fancier freespace handling TBD...
			return padding(base, _metahdr->offset(), alignment);
		}
	  	inline size_t allocSpace(size_t offset, size_t bytes) {
			// fancier freespace handling TBD...
			return _metahdr->offset(offset + bytes);
		}

		// caller holds lock
		inline T_MMAlloc *find(const char *key) {
			size_t x, y;
			for (x = 0; x < _metahdr->numMetas(); ++x) {
				if (_getMeta(x) != PAMI_SUCCESS) return NULL;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (_metas[x][y].isFree()) continue;
					if (_metas[x][y].isMatch(key)) {
						return &_metas[x][y];
					}
				}
			}
			return NULL;
		}

		// only for MemoryManager, T_MMAlloc == MemoryManagerAlloc
		// caller holds lock
		inline T_MMAlloc *find(size_t off) {
			size_t x, y;
			for (x = 0; x < _metahdr->numMetas(); ++x) {
				if (_metas[x] == NULL) break;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (_metas[x][y].isFree()) continue;
					if (_metas[x][y].offset() == off) {
						return &_metas[x][y];
					}
				}
			}
			return NULL;
		}

		// only for SharedMemoryManager, T_MMAlloc == MemoryManagerOSShmAlloc
		// caller holds lock
		inline T_MMAlloc *find(void *mem) {
			size_t x, y;
			for (x = 0; x < _metahdr->numMetas(); ++x) {
				if (_metas[x] == NULL) break;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (_metas[x][y].isFree()) continue;
					if (_metas[x][y].userMem() == mem) {
						return &_metas[x][y];
					}
				}
			}
			return NULL;
		}

		// allocates more space if needed/possible.
		// caller holds lock
		inline T_MMAlloc *findFree() {
			size_t x, y;
			COMPILE_TIME_ASSERT(MMMAX_N_META <= 10); // for "0123456789"[x]...
			for (x = 0; x < MMMAX_N_META; ++x) {
				if (_getMeta(x) != PAMI_SUCCESS) return NULL;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (_metas[x][y].isFree()) {
						return &_metas[x][y];
					}
				}
			}
			return NULL;
		}

		// caller holds lock
		inline void forAllActive(void (*func)(T_MMAlloc *m, void *cookie),
						void *cookie = NULL) {
			size_t x, y;
			for (x = 0; x < _metahdr->numMetas(); ++x) {
				if (_getMeta(x) != PAMI_SUCCESS) return;
				for (y = 0; y < MM_META_NUM(x); ++y) {
					if (!_metas[x][y].isFree()) {
						func(&_metas[x][y], cookie);
					}
				}
			}
		}
	private:
		MemoryManager *_meta_mm; // mm for meta data (same scope as parent)
		bool _pre_alloc;
		MemoryManagerHeader *_metahdr;
		char _meta_key_fmt[MMKEYSIZE];
		size_t _meta_key_len;
		T_MMAlloc *_metas[MMMAX_N_META];
	}; // class MemoryManagerMeta

      public:

	static MemoryManager *heap_mm; // reference to __global.heap_mm without circ deps.
	static MemoryManager *shared_mm; // reference to __global.shared_mm without circ deps.
	static MemoryManager *shm_mm; // reference to __global.mm without circ deps.

        ///
        /// \brief Empty base memory manager constructor
        ///
        inline MemoryManager () :
	_base(NULL),
	_size(0),
	_enabled(false),
	_attrs(0),
	_alignment(sizeof(void *)),
	_pmm(NULL)
        {
          TRACE_ERR((stderr, "%s, this = %p\n", __PRETTY_FUNCTION__, this));
        }

	virtual ~MemoryManager()
	{
		//_pmm = NULL;
		_enabled = false;
		_base = NULL;
	}

        inline void enable () { _enabled = true; }
        inline void disable () { _enabled = false; }
	inline unsigned attrs () { return _attrs; }
	inline void attrs (unsigned attrs) { _attrs |= attrs; }
        inline MemoryManager *getParent() { return _pmm; }

        ///
        /// \brief Allocate an aligned buffer of the memory.
        ///
        /// The initializer function is called only once on a given chunk of memory,
	/// by the first caller to allocate with a given key.
        ///
        /// This does NOT support the freeing of memory
        ///
        /// \param[out] memptr    Pointer to the allocated memory.
        /// \param[in]  alignment Requested buffer alignment - must be a power of 2.
        /// \param[in]  bytes     Number of bytes to allocate.
        /// \param[in]  key       (opt) Shared identifier for allocation
        /// \param[in]  init_fn   (opt) Initializer
        /// \param[in]  cookie    (opt) Opaque data for initializer
        ///
        virtual pami_result_t memalign(void **memptr, size_t alignment, size_t bytes,
			const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL) = 0;
	virtual void free(void *mem) = 0;
        virtual size_t available (size_t alignment = 1) = 0;

        virtual const char *getName() = 0;
        ///
        /// \brief Intialize a memory manager from another memory manager
        ///
        /// \param[in] mm        MemoryManager providing buffer
        /// \param[in] bytes     Number of bytes of memory to manage
        /// \param[in] alignment (opt) alignment of memory chunk
        /// \param[in] new_align (opt) Default/minimum alignment for mm
        /// \param[in] attrs     (opt) Attributes added to new mm
        /// \param[in] key	 (opt) Unique identifier for memory
        /// \param[in] init_fn	 (opt) Initializer function
        /// \param[in] cookie	 (opt) Opaque data for init_fn
        ///
        virtual pami_result_t init(MemoryManager *mm,
					size_t bytes, size_t alignment = 1,
					size_t new_align = 1,
					unsigned attrs = 0, const char *key = NULL,
					MM_INIT_FN *init_fn = NULL, void *cookie = NULL) = 0;
        ///
        /// \brief Reset a memory manager
        ///
        /// Forceably release all user memory. (caution!)
	/// If PAMI_MM_DEBUG is set, will print then reset statistics.
        ///
        virtual pami_result_t reset(bool force = false) = 0;

	virtual void dump(const char *str = NULL) = 0;
        ///
        /// \brief Return the size of the managed memory buffer
        ///
        /// \return    Number of bytes in the memory buffer
        ///
        inline size_t size ()
        {
          TRACE_ERR((stderr, "%s %zu\n", __PRETTY_FUNCTION__,_size));
          PAMI_assert_debug(_base != NULL);
          return _size;
        }

        ///
        /// \brief Return the base address of the managed memory buffer
        ///
        /// \return    base address of the memory buffer
        ///
        inline void *base ()
        {
          return _base;
        }

      protected:
        void * _base;
        size_t _size;
        bool   _enabled;
	unsigned _attrs;
	size_t _alignment;
	MemoryManager *_pmm; // parent mm
    }; // class MemoryManager

    class GenMemoryManager : public MemoryManager {
    private:
	inline void MM_RESET_STATS() {
		_num_allocs = 0;
		_num_frees = 0;
		_loc_bytes = 0;
		_rep_bytes = 0;
	}
	inline void MM_DUMP_STATS() {
		fprintf(stderr, "%s(%p, %zd): %zd allocs, %zd frees, "
				"local %zdb, repeat %zdb\n",
			_name, _base, _size,
			_num_allocs, _num_frees,
			_loc_bytes, _rep_bytes);
	}
    public:
	static size_t MAX_META_SIZE() {
		size_t x;
		size_t y = 0;
		for (x = 0; x < MMMAX_N_META; ++x) {
			y += MM_META_NUM(x);
		}
		return y * sizeof(MemoryManagerAlloc) + sizeof(MemoryManagerHeader);
	}

	GenMemoryManager() : MemoryManager(),
	_meta()
	{
#ifdef MM_DEBUG // set/unset in MemoryManager.h
		_debug = (getenv("PAMI_MM_DEBUG") != NULL);
		MM_RESET_STATS();
#endif // MM_DEBUG
	}
	virtual ~GenMemoryManager()
	{
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
		}
#endif // MM_DEBUG
		if (_base) {
			if (_pmm->base()) {
				_pmm->free(_base);
			}
		}
	}

	inline const char *getName() { return _name; }

	inline pami_result_t reset(bool force = false) {
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
			MM_RESET_STATS();
		}
#endif // MM_DEBUG
		// "reset" - discard arena
		pami_result_t rc = PAMI_SUCCESS;
		if (force) rc = _meta.reset();
		return rc;
	}

        inline pami_result_t init(MemoryManager *mm,
					size_t bytes, size_t alignment = 1,
					size_t new_align = 1,
					unsigned attrs = 0, const char *key = NULL,
					MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
        {
		// assert 'key' to be non-NULL?
		PAMI_assert_debugf(!(alignment & (alignment - 1)), "%zd: alignment must be power of two", alignment);
		PAMI_assert_debugf(!(new_align & (new_align - 1)), "%zd: new_align must be power of two", new_align);
		PAMI_assert_debugf(mm && _enabled == false, "Trying to re-init MemoryManager");
	  	if (new_align < _alignment) new_align = _alignment;
		if (attrs & PAMI_MM_SCOPE) {
			// can't change scope
			return PAMI_INVAL;
		}
#if 0
		// minimal alignment for MemoryManagerAlloc
		if (alignment < T_Alloc::ALIGNMENT) {
			alignment = T_Alloc::ALIGNMENT;
		}
#endif
		_pmm = mm;
		// make new allocation...
		if (key) {
			strncpy(_name, key, sizeof(_name));
		} else {
			_name[0] = '\0';
		}
		pami_result_t rc = mm->memalign((void **)&_base, alignment, bytes,
								_name, init_fn, cookie);
		if (rc != PAMI_SUCCESS) {
			return rc;
		}
		_attrs = mm->attrs() | attrs;
          	_size = bytes;
		_alignment = new_align;
		// have a problem when creating shm_mm,
		// can't use itself for meta data allocs.
		MemoryManager *mmm = (mm->attrs() & PAMI_MM_NODESCOPE) ?
					(this == shm_mm ? shared_mm : shm_mm) : heap_mm;
		_meta.init(mmm, _name);
          	_enabled = true;
		return PAMI_SUCCESS;
	}

        inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes,
			const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
	  if (alignment < _alignment) alignment = _alignment;
	  if (key && strlen(key) >= MMKEYSIZE) {
		return PAMI_INVAL;
	  }
	  _meta.acquire();
	  MemoryManagerAlloc *m;
	  if (key && key[0]) {
		// "public" (shared) allocation
		m = _meta.find(key);
		if (m) {
			m->addRef();
			_meta.release();
			m->waitDone();
			*memptr = (uint8_t *)_base + m->offset();
#ifdef MM_DEBUG
			if (_debug) {
				++_num_allocs;
				size_t alloc_bytes = m->rawSize(); // m->userSize();
				_rep_bytes += alloc_bytes;
			}
#endif // MM_DEBUG
			return PAMI_SUCCESS;
		}
		// lock still held...
	  	m = _meta.findFree();
	  } else {
		if (key) {
			// callers wants to know the unique key we chose...
			snprintf((char *)key, MMKEYSIZE, "/%d-%lx",
				getpid(), (unsigned long)memptr);
		}
	  	m = _meta.findFree();
	  }
	  if (!m) {
		_meta.release();
#ifdef MM_DEBUG
		if (_debug) {
			dump("ENOMEM(meta)");
	  	}
#endif // MM_DEBUG
		return PAMI_ERROR;
	  }
	  //PAMI_assertf(m, "Ran out of metadata space");
	  // pre-existing, shared, chunks were handled above,
	  // no init required by them. We have the lock, and are the
	  // first, so just do init if needed.

	  size_t offset = _meta.getSpace(_base, alignment, bytes);
	  m->offset(offset);
	  m->userSize(bytes, 0);
	  m->rawOffset(offset); // must be after m->offset(...) and m->userSize(...)
	  if (m->offset() + bytes > _size) {
		_meta.release();
#ifdef MM_DEBUG
		if (_debug) {
			dump("ENOMEM");
		}
#endif // MM_DEBUG
		return PAMI_ERROR;
	  }
	  m->key(key);
	  m->addRef();
	  offset = _meta.allocSpace(m->offset(), bytes);
	  _meta.release();
	  *memptr = (uint8_t *)_base + m->offset();
	  if (init_fn) {
		init_fn(*memptr, bytes, key, _attrs, cookie);
	  }
	  m->initDone();
#ifdef MM_DEBUG
	if (_debug) {
		++_num_allocs;
		size_t alloc_bytes = m->rawSize(); // m->userSize();
		_loc_bytes += alloc_bytes;
	}
#endif // MM_DEBUG
	  return PAMI_SUCCESS;
	}

	inline void free(void *mem)
	{
		// for now, only top-level mm's actually free...
		// i.e. SharedMemoryManager and HeapMemoryManager.
		// and then only during job exit (dtor).
#ifdef MM_DEBUG
		size_t off = (size_t)mem - (size_t)_base;
		MemoryManagerAlloc *m = _meta.find(off);
		if (m) {
			if (_debug) {
				++_num_frees;
				// _rep_bytes -= m->rawSize(); // m->userSize();
			}
		}
#endif // MM_DEBUG
	}

        ///
        /// \brief Return the current maximum number of bytes that may be allocated
        ///
        /// \param[in] alignment Optional alignment parameter
        ///
        /// \return    Number of bytes available
        ///
        inline size_t available(size_t alignment = 1)
        {
          TRACE_ERR((stderr, "%s(%zu) _size %zu, _offset %zu, this = %p\n", __PRETTY_FUNCTION__,alignment, _size, _offset, this));
          PAMI_assert(_enabled==true);
          PAMI_assert_debug((alignment & (alignment - 1)) == 0);
	  if (alignment < _alignment) alignment = _alignment;

          return _size - _meta.spaceUsed();
        }

	inline void dump(const char *str) {
		if (str) {
			fprintf(stderr, "%s: ", str);
		}
		fprintf(stderr, "%s::GenMemoryManager %p %zd (%zd) %x\n", _name,
					_base, _size, _meta.spaceUsed(), _attrs);
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
		}
#endif // MM_DEBUG
		_meta.dump(_name);
		_pmm->dump(str);
	}
    private:
	MemoryManagerMeta<MemoryManagerAlloc> _meta;
	char _name[MMKEYSIZE];
#ifdef MM_DEBUG
	bool _debug;
	size_t _num_allocs;
	size_t _num_frees;
	size_t _loc_bytes;
	size_t _rep_bytes;
#endif // MM_DEBUG
    }; // class GenMemoryManager

  }; // namespace Memory
}; // namespace PAMI

#endif // __components_memory_MemoryManager_h__
