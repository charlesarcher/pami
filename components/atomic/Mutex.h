/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_atomic_Mutex_h__
#define __components_atomic_Mutex_h__

#include "SysDep.h"

///
///  \file components/atomic/Mutex.h
///  \brief Mutex Objects for Hardware and Software Mutexes
///
///  This object is a portability layer that abstracts atomic locking
///  - Access to the hardware locks
///  - Access to the software locks
///  - Lock/Unlock methods are provided
///  - Allocation/Deallocation handled by constructor/destructor
///
///  Definitions:
///  - Node Mutex:        A mutex where the scope is across all cores on the node.
///  - Process Mutex:     A mutex where the scope is only across the cores/threads that participate in a SMP process.
///
///  Namespace:  DCMF, the messaging namespace
///  Notes:  This is currently indended for use only by the lock manager
///
///
namespace PAMI
{
namespace Atomic
{
namespace Interface
{
  ///
  ///  \brief Base Class for Mutex
  ///
  template <class T_Object>
  class Mutex
    {
    public:
      ///
      /// \brief  Acquire a lock atomically
      ///
      inline void acquire();

      ///
      /// \brief  Release a lock atomically
      ///
      inline void release();

      ///
      /// \brief  Try to acquire a lock atomically
      ///
      inline bool tryAcquire();

      ///
      /// \brief  Test if mutex is locked
      ///
      inline bool isLocked();

      ///
      /// \brief  Alloc and Init
      ///
      inline void init(PAMI::Memory::MemoryManager *mm);

      ///
      /// \brief  Provide access to the raw lock var/data
      ///
      inline void * returnLock();
    protected:
      ///
      /// \brief  Construct a lock
      ///
      Mutex() {};
      ~Mutex() {};

    private:
    }; // class Mutex

template <class T_Object>
inline void Mutex<T_Object>::acquire()
{
	static_cast<T_Object*>(this)->acquire_impl();
}

template <class T_Object>
inline void Mutex<T_Object>::release()
{
	static_cast<T_Object*>(this)->release_impl();
}

template <class T_Object>
inline bool Mutex<T_Object>::tryAcquire()
{
	return static_cast<T_Object*>(this)->tryAcquire_impl();
}

template <class T_Object>
inline bool Mutex<T_Object>::isLocked()
{
	return static_cast<T_Object*>(this)->isLocked_impl();
}

template <class T_Object>
inline void Mutex<T_Object>::init(PAMI::Memory::MemoryManager *mm)
{
	static_cast<T_Object*>(this)->init_impl(mm);
}

template <class T_Object>
inline void *Mutex<T_Object>::returnLock()
{
	return static_cast<T_Object*>(this)->returnLock_impl();
}

}; // namespace Interface
}; // namespace Atomic
}; // namespace PAMI

#endif // __pami_mutex_object_h__
