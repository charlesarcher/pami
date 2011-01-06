/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/native/YieldingNativeMutex.h
 * \brief "native" compiler builtin atomics implementation of the mutex interface that yields
 */
#ifndef __components_atomic_native_YieldingNativeMutex_h__
#define __components_atomic_native_YieldingNativeMutex_h__

#include <stdint.h>

#include "Compiler.h"
#include "components/atomic/MutexInterface.h"
#include "components/memory/MemoryManager.h"
#include "NativeAtomics.h"

namespace PAMI
{
  namespace Mutex
  {
    ///
    /// \brief PAMI::Mutex::Interface implementation using "native" compiler builtin atomics
    ///
    /// The PAMI::Mutex::Native class is considered an "in place" implementation
    /// because the storage for the actual atomic resource is embedded within
    /// the class instance.
    ///
    /// Any "in place" mutex implementation may be converted to an "indirect"
    /// mutex implementation, where the atomic resource is located outside
    /// of the class instance, by using the PAMI::Mutex::Indirect<T> class
    /// instead of the native "in place" implementation.
    ///
    class YieldingNative : public PAMI::Mutex::Interface<YieldingNative>
    {
      public:

        friend class PAMI::Mutex::Interface<YieldingNative>;

        inline YieldingNative() {};

        inline ~YieldingNative() {};

	static bool checkCtorMm(PAMI::Memory::MemoryManager *mm)
	{
		return true;
	}

	static bool checkDataMm(PAMI::Memory::MemoryManager *mm)
	{
		return true;
	}
      
      protected:

        // -------------------------------------------------------------------
        // PAMI::Mutex::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void acquire_impl()
        {
          while (_atom.lock_test_and_set(1) != 0)
	  {
		yield();
	  }
        }

        inline bool tryAcquire_impl()
        {
          return (_atom.lock_test_and_set(1) == 0);
        }

        inline void release_impl()
        {
          _atom.lock_release();
        }

        inline bool isLocked_impl()
        {
          return (_atom.fetch() != 0);
        }

        PAMI::Atomic::NativeAtomic _atom;

    }; // PAMI::Mutex::YieldingNative class
  }; //   PAMI::Mutex namespace
}; //     PAMI namespace

#endif // __components_atomic_native_YieldingNativeMutex_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
