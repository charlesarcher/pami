/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __xmi_bgp_lockboxbarrier_h__
#define __xmi_bgp_lockboxbarrier_h__

/**
 * \file sysdep/prod/BGP/BarrierImpl.h
 * \brief LockBox implementation of a Node-scoped Barrier
 * This implementation is different from, and incompatible with,
 * the BGP SPI LockBox_Barrier implementation. Support for core
 * granularity barriers is added here, in a way that makes the
 * barrier code common for all granularities.
 */
#include "SysDep.h"
#include "components/sysdep/bgp/LockBoxFactory.h"
#include "components/atomic/Barrier.h"
#include <spi/bgp_SPI.h>

namespace XMI {
namespace Barrier {
namespace BGP {
/*
 * This class cannot be used directly. The super class must accocate the
 * particular type of lockbox based on desired scope.
 */
class _LockBoxBarrier {
private:
	/**
	 * \brief Base structure for lockbox barrier
	 *
	 * This houses 5 lockboxes which are used to implement
	 * a barrier.
	 */
	struct LockBox_Barrier_s {
		union {
			uint32_t *lkboxes[5]; /**< access all 5 lockboxes */
			struct {
				LockBox_Counter_t ctrl_lock; /**< the control lockbox (phase) */
				LockBox_Counter_t lock[2]; /**< lock lockboxes for all phases */
				LockBox_Counter_t status[2]; /**< status lockboxes for all phases */
			} lbx_s;
		} lbx_u;
		uint8_t master;    /**< master participant */
		uint8_t coreshift; /**< convert core to process for comparing to master */
		uint8_t nparties;  /**< number of participants */
		uint8_t _pad;      /**< pad to int */
	};
#define lbx_lkboxes	lbx_u.lkboxes		/**< shortcut for lkboxes */
#define lbx_ctrl_lock	lbx_u.lbx_s.ctrl_lock	/**< shortcut for ctrl_lock */
#define lbx_lock	lbx_u.lbx_s.lock	/**< shortcut for lock */
#define lbx_status	lbx_u.lbx_s.status	/**< shortcut for status */
public:
	_LockBoxBarrier() { }
	~_LockBoxBarrier() { }

	inline void init_impl();

	inline void enter_impl() {
		pollInit_impl();
		while (poll_impl() != Done);
	}

	inline void enterPoll_impl(pollFcn fcn, void *arg) {
		pollInit_impl();
		while (poll_impl() != Done) {
			fcn(arg);
		}
	}

	inline void pollInit_impl() {
		uint32_t lockup;
		_bgp_msync();
		lockup = LockBox_Query(_barrier.lbx_ctrl_lock);
		LockBox_FetchAndInc(_barrier.lbx_lock[lockup]);
		_data = (void*)lockup;
		_status = Entered;
	}

	inline lockPollStatus poll_impl() {
		DCMF_assert(_status == Entered);
		uint32_t lockup, value;
		lockup = (unsigned)_data;
		if (LockBox_Query(_barrier.lbx_lock[lockup]) < _barrier.nparties) {
			return Entered;
		}

		// All cores have participated in the barrier
		// We need all cores to block until checkin
		// to clear the lock atomically
		LockBox_FetchAndInc(_barrier.lbx_lock[lockup]);
		do {
			value = LockBox_Query(_barrier.lbx_lock[lockup]);
		} while (value > 0 && value < (unsigned)(2 * _barrier.nparties));

		if ((Kernel_PhysicalProcessorID() >> _barrier.coreshift) == _barrier.master) {
			if (lockup) {
				LockBox_FetchAndDec(_barrier.lbx_ctrl_lock);
			} else {
				LockBox_FetchAndInc(_barrier.lbx_ctrl_lock);
			}
			LockBox_FetchAndClear(_barrier.lbx_status[lockup]);
			LockBox_FetchAndClear(_barrier.lbx_lock[lockup]);
		} else {
			// wait until master releases the barrier by clearing the lock
			while (LockBox_Query(_barrier.lbx_lock[lockup]) > 0);
		}
		_status = Initialized;
		return Done;
	}
	// With 5 lockboxes used... which one should be returned?
	inline void *returnBarrier_impl() { return _barrier.lbx_ctrl_lock; }

	inline void init_impl() {
		XMI_abortf("_LockBoxBarrier class must be subclass");
	}
private:
	LockBox_Barrier_s _barrier;
	void *_data;
	lockPollStatus _status;
}; // class _LockBoxBarrier

class LockBoxNodeCoreBarrier : public _LockBoxBarrier {
public:
	LockBoxNodeCoreBarrier() : _LockBoxBarrier() {}
	~LockBoxNodeCoreBarrier() {}
	inline void init_impl() {
		// For core-granularity, everything is
		// a core number. Assume the master core
		// is the lowest-numbered core in the
		// process.
		_barrier.master = lm->masterProc << lm->coreShift;
		_barrier.coreshift = 0;
		_barrier.nparties = lm->numCore;
		XMI::Atomic::BGP::LockBoxFactory::lbx_alloc(
						(void **)_barrier.lbx_lkboxes, 5,
						XMI::Atomic::BGP::LBX_NODE_SCOPE);
		_status = Initialized;
	}
}; // class LockBoxNodeCoreBarrier

class LockBoxNodeProcBarrier : public _LockBoxBarrier
public:
	LockBoxNodeProcBarrier() : _LockBoxBarrier() {}
	~LockBoxNodeProcBarrier() {}
	inline void init_impl() {
		// For proc-granularity, must convert
		// between core id and process id,
		// and only one core per process will
		// participate.
		_barrier.master = lm->coreXlat[lm->masterProc] >> lm->coreShift;
		_barrier.coreshift = lm->coreShift;
		_barrier.nparties = lm->numProc;
		XMI::Atomic::BGP::LockBoxFactory::lbx_alloc(
						(void **)_barrier.lbx_lkboxes, 5,
						XMI::Atomic::BGP::LBX_PROC_SCOPE);
		_status = Initialized;
	}
}; // class LockBoxNodeProcBarrier

}; // BGP namespace
}; // Barrier namespace
}; // XMI namespace

#endif // __xmi_bgp_lockboxbarrier_h__
