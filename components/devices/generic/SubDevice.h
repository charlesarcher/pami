/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_devices_generic_SubDevice_h__
#define __components_devices_generic_SubDevice_h__

#include "util/queue/Queue.h"
#include "components/devices/generic/BaseGenericDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/Device.h"
#include "sys/xmi.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/SubDevice.h
///  \brief Generic Device
///
///  The Generic classes implement a QueueSystem and a Message object
///  to post into the queueing system.  The GI device is currently
///  used to implement barriers, so the Generic device posts a message
///  and uses a interprocess communication sysdep to signal the Generic wire
///  This is used to implement
///  -
///  - Barriers
///
///  Definitions:
///  - GenericMessage:  An Generic message
///  - Device:      Queue System for messages
///
///  Namespace:  XMI, the messaging namespace.
///
////////////////////////////////////////////////////////////////////////
namespace XMI {
namespace Device {
namespace Generic {

class GenericSubDevSendq : public MultiQueue<2, 0> {
public:

	/// \brief Add a message to the (end of the) queue
	///
	/// Does not perform any actions on the message, the caller has
	/// already attempted early advance.
	///
	/// \param[in] msg	New message to be posted
	///
	inline void post(GenericMessage *msg) {
		pushTail(msg);
	}

	/// \brief peek at "next" message on queue
	///
	/// \return	Top message on queue
	///
	inline GenericMessage *getCurrent() {
		return (GenericMessage *)peekHead();
	}

	/// \brief pop "next" message off queue and return it
	///
	/// \return	Former top message on queue
	///
	inline GenericMessage *dequeue() {
		return (GenericMessage *)popHead();
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	///
	inline int queueSize() {
		return size();
	}
protected:
}; // class GenericSubDevSendq

// in this case, threads come from sub-device...
// (others might have threads in the message, or ???)
// postToGeneric() will only post threads that are not Done...
#define STD_POSTNEXT(T_Device,T_Thread)							\
	inline bool postNext(bool devPosted) {						\
		T_Thread *t;								\
		int N, n;								\
		setStatus(XMI::Device::Initialized);					\
		static_cast<T_Device &>(_QS).getThreads(&t, &N);			\
		n = __setThreads(t, N);							\
		if (!devPosted && getStatus() == XMI::Device::Done) {			\
			return true;							\
		}									\
		static_cast<T_Device &>(_QS).postToGeneric(this, t, sizeof(*t), n);	\
		return false;								\
	}

class GenericSubDevice : public BaseGenericDevice {
private:
public:
	GenericSubDevice() :
	BaseGenericDevice(),
	_hasBlockingAdvance(false),
	_nRoles(1),
	_repl(-1)
	{
	}

	GenericSubDevice(int nRoles, int repl) :
	BaseGenericDevice(),
	_hasBlockingAdvance(false),
	_nRoles(nRoles),
	_repl(repl),
	_sd(NULL)
	{
	}

	virtual ~GenericSubDevice() { }

	inline XMI::SysDep *getSysdep() { return _sd; }

	inline int advanceRecv(size_t context);

	/// \brief default blocking advance and prototype
	///
	/// may be overridden by parent class, along with calling setBlockingAdvance(true).
	///
	/// \param[in] thr	Thread to be advanced
	///
	virtual void advanceBlocking(GenericAdvanceThread *thr) { XMI_abort(); }

	/// \brief whether messages support blocking advance
	///
	/// \return	Boolean indicating support for blocking advance calls
	///
	inline bool hasBlockingAdvance() { return _hasBlockingAdvance; }

	/// \brief get information about message roles
	///
	/// Number of roles indicates how much distribution of work between
	/// processes may be done. Replication indicates which role (if any)
	/// is to be replicated for addition processes beyond nRoles.
	///
	/// \param[out] nRoles	Number of roles available
	/// \param[out] repl	Which role to replicate
	///
	inline void getRoles(int *nRoles, int *repl) {
		*nRoles = _nRoles;
		*repl = _repl;
	}

	// wrappers for GenericSubDevSendq...

	/// \brief Add a message to the (end of the) queue
	///
	/// Does not perform any actions on the message, the caller has
	/// already attempted early advance.
	///
	/// \param[in] msg	New message to be posted
	///
	inline void post(GenericMessage *msg) {
		_queue.pushTail(msg);
	}

	/// \brief peek at "next" message on queue
	///
	/// \return	Top message on queue
	///
	inline GenericMessage *getCurrent() {
		return (GenericMessage *)_queue.peekHead();
	}

	/// \brief pop "next" message off queue and return it
	///
	/// \return	Former top message on queue
	///
	inline GenericMessage *dequeue() {
		return (GenericMessage *)_queue.popHead();
	}

	/// \brief Remove a message from the middle of the queue
	///
	/// \param[in] msg	New message to be removed
	///
	inline void deleteElem(GenericMessage *msg) {
		_queue.deleteElem(msg);
	}

	/// \brief number of messages on the queue
	///
	/// \return	number of messages on the queue
	///
	inline int queueSize() {
		return _queue.size();
	}

	inline void __complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		XMI::Device::Generic::GenericMessage *nxt = getCurrent();
		if (nxt) {
			// skips posting to sub-device... that was already done.
			// must setup threads and post threads+message to generic device.
			(void)nxt->postNext(true);
			// Note: message might have completed here, but handling
			// that is too complicated so we just let the generic
			// device do the completion when it finds it on the queue.
			// (recursion possibility is one complication)
		}
	}

	inline void postToGeneric(GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		_generics->post(msg, t, l, n);
	}

	inline XMI::Device::Generic::Device *getGeneric(size_t contextId) { return &_generics[contextId]; }

protected:
	inline void ___init(XMI::SysDep &sd, XMI::Device::Generic::Device *generics) {
		_sd = &sd;
		_generics = generics;
	}

	/// \brief tell whether messages support blocking advance
	///
	/// \param[in] f	Boolean indicating support for blocking advance calls
	///
	inline void setBlockingAdvance(bool f) { _hasBlockingAdvance = f; }


	/// \brief set information about message roles
	///
	/// Number of roles indicates how much distribution of work between
	/// processes may be done. Replication indicates which role (if any)
	/// is to be replicated for addition processes beyond nRoles.
	///
	/// \param[out] nRoles	Number of roles available
	/// \param[out] repl	Which role to replicate
	///
	inline void setRoles(int nRoles, int repl) {
		_nRoles = nRoles;
		_repl = repl;
	}

	GenericSubDevSendq _queue;
	bool _hasBlockingAdvance;
	int _nRoles;
	int _repl;
	XMI::SysDep *_sd;
	XMI::Device::Generic::Device *_generics;
}; /* class GenericSubDevice */

/// \brief Simple Sub-Device where no threading is used.
///
/// A single-thread basic sub-device - standard boilerplate
/// One thread, no roles, no "receive" polling.
/// Thread object is 'empty', used only to queue work to Generic::Device.
///
/// Supports only one active message at a time.
///
template <class T_Thread>
class SimpleSubDevice : public GenericSubDevice {
	static const int NUM_THREADS = 1;
public:
	SimpleSubDevice() :
	GenericSubDevice()
	{
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
	}

	inline void getThreads(T_Thread **t, int *n) {
		*t = _threads;
		*n = NUM_THREADS;
	}

protected:
	friend class XMI::Device::Generic::Device;

	inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) {
		___init(sd, devices);
	}

	inline int advanceRecv(size_t context) { return 0; }

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Model;
	// So, we need to make this public until we figure it out.
public: // temporary

	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		// assert(isLocked(msg->getContext()));
		bool first = (getCurrent() == NULL);
		if (first) {
			if (static_cast<T_Message*>(msg)->postNext(false)) {
				msg->executeCallback(getGeneric(msg->getContextId())->getContext());
				return;
			}
		}
		XMI::Device::Generic::GenericSubDevice::post(msg);
	}

protected:
	T_Thread _threads[NUM_THREADS];
}; // class SimpleSubDevice

///
/// Implements a shared-queue for use by multiple different Thr/Msg/Dev/Mdl sets
/// which all share the same hardware (system) resource. Such a family of sets
/// would each refer to a common instance of this class object when doing their
/// init. The Device class of each set would inherit from SharedQueueSubDevice.
///
/// Supports only one active message at a time.
///
class CommonQueueSubDevice : public GenericSubDevice {
	#define ATOMIC_BUF_SIZE	16

public:

	CommonQueueSubDevice() :
	GenericSubDevice(),
	_init(0),
	_dispatch_id(0)
	{
	}

	inline unsigned newDispID() {
		// caller must ensure number os valid for their hardware,
		// for example a network device might only have 8 bits in
		// the header for "dispatch ID".

		// might need to be atomic, in some situations?
		return ++_dispatch_id;
	}

	// If we find that multiple devices (sharing this queue) are all in init()
	// at the same time, this init flag will have to become some sort of atomic op.
	// Right now, it should only be the case that a single thread is sequentially
	// calling each device's init() routine so there is no problem.
	int isInit() { return _init; }

	/// \brief init virtual function definition
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	///
	virtual void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) = 0;

	/// \brief CommonQueueSubDevice portion of init function
	///
	/// All classes that inherit from this must implement init(), and that
	/// must callback to __init().
	///
	/// \param[in] sd	SysDep object
	/// \param[in] device	Generic::Device to be used.
	///
	inline void __init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) {
		_doneThreads.init(&sd);
		_doneThreads.fetch_and_clear();
		_init = 1;
		___init(sd, devices);
	}

	inline void post_msg(XMI::Device::Generic::GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		// doesn't matter which generic device "slice" we use to post...
		// the routine selects actual slice(s) based on msg.
		_generics->post(msg, t, l, n);
	}

	inline void __resetThreads() {
		_doneThreads.fetch_and_clear();
	}

	inline unsigned __completeThread(GenericAdvanceThread *t) {
		// fetchIncr() returns value *before* increment,
		// and we need to return total number of threads completed,
		// so we return "+1".
		return _doneThreads.fetch_and_inc() + 1;
	}

	///
	/// \brief Complete a message on the device and return next message
	///
	/// assumes only one message active at a time. otherwise, instead of dequeue()
	/// use remove(msg) and instead of getCurrent() must search for
	/// the next "unactivated" message to be started.
	///
	/// \param[in] msg	Message being completed.
	/// \return	Next message to start, or NULL if none.
	///
	inline XMI::Device::Generic::GenericMessage *__complete(XMI::Device::Generic::GenericMessage *msg) {
		/* assert msg == dequeue(); */
		dequeue();
		return getCurrent();
	}

private:
	int _init;
	GenericDeviceCounter _doneThreads;
	unsigned _dispatch_id;
}; // class CommonQueueSubDevice

/// \brief class for a Model/Device/Message/Thread tuple that shares hardware with others
///
/// Each Model/Device/Message/Thread tuple will inherit from SharedQueueSubDevice and
/// reference the same instance of CommonQueueSubDevice in the constructor.
/// One instance of CommonQueueSubDevice represents the hardware, which is shared, that
/// can only accomodate one message (of any type) active at a time.
///
/// We would like to have the _threads[] storage kept in the CommonQueueSubDevice,
/// but each Model/Device/Message/Thread tuple could have a different sized Thread class
/// and so we must have the _threads[] array here, where we know the exact Thread type.
///
template <class T_CommonDevice, class T_Thread, int N_Threads>
class SharedQueueSubDevice : public BaseGenericDevice {
	static const int NUM_THREADS = N_Threads;
public:
	// Note, 'common' must have been constructed but otherwised untouched.
	// The first SharedQueueSubDevice to encounter it will initialize it.
	SharedQueueSubDevice(T_CommonDevice *common) :
	BaseGenericDevice(),
	_common(common)
	{
	}

	inline unsigned newDispID() {
		return _common->newDispID();
	}

	inline T_CommonDevice *common() { return _common; }

	inline void getThreads(T_Thread **t, int *n) {
		*t = _threads;
		*n = NUM_THREADS;
	}

	inline void postToGeneric(GenericMessage *msg, GenericAdvanceThread *t, size_t l, int n) {
		_common->postToGeneric(msg, t, l, n);
	}
private:
	template <class T_Message>
	inline void __start_msg(XMI::Device::Generic::GenericMessage *msg) {
		int n;
		_common->__resetThreads();
		msg->setStatus(XMI::Device::Initialized);
		n = static_cast<T_Message*>(msg)->__setThreads(&_threads[0], NUM_THREADS);
		_nActiveThreads = n;
	}

	inline void __post_msg(XMI::Device::Generic::GenericMessage *msg) {
		_common->post_msg(msg, &_threads[0], sizeof(_threads[0]), _nActiveThreads);
	}

protected:
	friend class XMI::Device::Generic::Device;

	inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t contextId) {
		// do this now so we don't have to every time we post
//		for (int x = 0; x < NUM_THREADS; ++x) {
//			//_threads[x].setPolled(true);
//		}
		if (!_common->isInit()) {
			_common->init(sd, devices, contextId);
		}
	}

	inline int advanceRecv(size_t context) { return 0; }

private:
	// For some reason, we can't declare friends like this.
	//friend class T_Message;
	//friend class T_Model;
	// So, need to make it public for now...
public:	// temporary?

	template <class T_Message>
	inline void __post(XMI::Device::Generic::GenericMessage *msg) {
		bool first = (_common->getCurrent() == NULL);
		if (first) {
			__start_msg<T_Message>(msg); // may try advance...
			if (msg->getStatus() == XMI::Device::Done) {
				msg->executeCallback(_common->getGeneric(msg->getContextId())->getContext());
				return;
			}
			__post_msg(msg);
		}
		_common->post(msg);
	}

	inline unsigned __completeThread(T_Thread *thr) {
		return _common->__completeThread(thr);
	}

	template <class T_Message>
	inline void __complete(T_Message *msg) {
		_nActiveThreads = 0;
		T_Message *nxt = (T_Message *)_common->__complete(msg);
		if (nxt) {
			__start_msg<T_Message>(nxt); // may try advance...
			// don't complete here - too complicated recursion potential
			__post_msg(nxt);
		}
	}

private:
	T_CommonDevice *_common;
	T_Thread _threads[NUM_THREADS];
	int _nActiveThreads;
}; // class SharedQueueSubDevice

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace Device */

#endif /* __components_devices_generic_subdevice_h__ */
