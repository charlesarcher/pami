/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/workqueue/LocalReduceWQMessage.h
 * \brief ???
 */

#ifndef __components_devices_workqueue_localreducewqmsg_h__
#define __components_devices_workqueue_localreducewqmsg_h__

#include "components/devices/workqueue/SharedWorkQueue.h"
#include "components/devices/workqueue/MemoryWorkQueue.h"
#include "math/math_coremath.h"
#include "components/sysdep/SysDep.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/SubDevice.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/MulticombineModel.h"

extern XMI::Topology *_g_topology_local;

namespace XMI {
namespace Device {

class LocalReduceWQModel;
class LocalReduceWQMessage;
typedef XMI::Device::Generic::GenericAdvanceThread LocalReduceWQThread;
typedef XMI::Device::Generic::SimpleSubDevice<LocalReduceWQThread> LocalReduceWQDevice;

}; // namespace Device
}; // namespace XMI

extern XMI::Device::LocalReduceWQDevice _g_l_reducewq_dev;

namespace XMI {
namespace Device {

class LocalReduceWQMessage : public XMI::Device::Generic::GenericMessage {
public:

          ///
          /// \brief Local (intranode) reduce collective message.
          ///
          /// The rank designated as peer \c 0 is \b always the reduction root. Care must be
          /// taken when assigning peer identifications.
          ///
          /// \param[in] device       Shared Memory device
          /// \param[in] cb           Callback to invoke when the broadcast is complete
          /// \param[in] sharedmemory Location of the shared memory
          /// \param[in] peer         This core's peer id
          /// \param[in] peers        Number of ranks active on this node
          /// \param[in] sbuffer      Location of the source reduce buffer
          /// \param[in] rbuffer      Location of the result reduce buffer
          /// \param[in] count        Number of elements to reduce
          /// \param[in] func         Math function to invoke to perform the reduction
          /// \param[in] dtshift      Shift in byts of the elements for the reduction
          ///
          inline LocalReduceWQMessage (BaseGenericDevice &device,
                                       xmi_callback_t   cb,
                                       XMI::Device::WorkQueue::SharedWorkQueue &workqueue,
                                       unsigned          peer,
                                       unsigned          peers,
                                       unsigned          rootpeer,
                                       XMI::PipeWorkQueue *sbuffer,
                                       XMI::PipeWorkQueue *rbuffer,
                                       unsigned          count,
                                       coremath          func,
                                       int               dtshift) :
            XMI::Device::Generic::GenericMessage (device, cb),
            _isrootpeer (peer == rootpeer),
            //_iscopypeer (peer == ((rootpeer+1)%peers)),
            _iscopypeer (peer == ((rootpeer+1) >= peers ? (rootpeer+1) - peers : (rootpeer+1))),
            _func (func),
            _dtshift (dtshift),
            _source (*sbuffer),
            _result (*rbuffer),
            _shared (workqueue)
          {
            // Producer 0 will always be the "copy peer"
            //unsigned copypeer = (rootpeer+1)%peers;
            //unsigned producer = (peer + peers - rootpeer - 1) % peers;
            unsigned producer = (peer + peers - rootpeer - 1); if (producer >= peers) producer -= peers;
            if (_isrootpeer) producer = 0; // hack!


if (!(producer < peers-1)) fprintf(stderr, "LocalReduceWQMessage %d %d %d %d\n", peer, peers, rootpeer, producer);
            _shared.setProducers (peers-1, producer);
            _shared.setConsumers (1, 0);
          }

	///
	/// \brief Advance the reduce shared memory message
	///
	inline XMI::Device::MessageStatus advanceThread(XMI::Device::Generic::GenericAdvanceThread *t);

	inline void complete();

private:
	// friend class LocalReduceWQDevice;
	friend class XMI::Device::Generic::SimpleSubDevice<LocalReduceWQThread>;

	inline XMI::Device::MessageStatus __advanceThread(LocalReduceWQThread *thr) {
		// workaround for GNU compiler -fPIC -O3 bug
		volatile coremath1 shmcpy = (coremath1) XMI::Device::WorkQueue::SharedWorkQueue::shmemcpy;
		if (_iscopypeer) {
			_shared.Q2Q (_source, shmcpy, 0);

			// If all bytes have been copied from the local source buffer into
			// the shared queue then this peer is done.
			if (_source.bytesAvailableToConsume () == 0) setStatus(XMI::Device::Done);
		} else if (_isrootpeer) {
			_shared.reduce2Q (_source, _result, _func, _dtshift);

			// If all bytes have been copied from the shared queue into the
			// local result buffer then the root is done.
			if (_result.bytesAvailableToProduce () == 0) setStatus(XMI::Device::Done);
		} else {
			_shared.reduceInPlace (_source, _func, _dtshift);

			// If all bytes have been copied from the local source buffer into
			// the shared queue then this peer is done.
			if (_source.bytesAvailableToConsume () == 0) setStatus(XMI::Device::Done);
		}
		return getStatus();
	}

	inline int __setThreads(LocalReduceWQThread *t, int n) {
		t[0].setMsg(this);
		t[0].setDone(false);
		return 1;
	}

protected:

          bool              _isrootpeer;
          bool              _iscopypeer;
          coremath          _func;
          int               _dtshift;
          XMI::PipeWorkQueue   &_source;
          XMI::PipeWorkQueue   &_result;
          XMI::Device::WorkQueue::SharedWorkQueue & _shared;
}; // class LocalReduceWQMessage

class LocalReduceWQModel : public Reduce::Model<LocalReduceWQModel,LocalReduceWQDevice,LocalReduceWQMessage> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = 1;

	LocalReduceWQModel(XMI::SysDep *sysdep, xmi_result_t &status) :
	Reduce::Model<LocalReduceWQModel,LocalReduceWQDevice,LocalReduceWQMessage>(_g_l_reducewq_dev, status),
	_shared(sysdep),
	_peer(_g_topology_local->rank2Index(sysdep->mapping().rank())),
	_npeers(_g_topology_local->size())
	{
		if (!_shared.available()) {
			status = XMI_ERROR;
			return;
		}
		_shared.setProducers(_npeers, _peer);
		_shared.setConsumers(_npeers, _peer);
		reset_impl();
	}

	inline void reset_impl() {
		if (_peer == 0) {
			_shared.reset();
		}
	}

	inline bool generateMessage_impl(xmi_multicombine_t *mcomb);

private:
	XMI::Device::WorkQueue::SharedWorkQueue _shared;
	unsigned _peer;
	unsigned _npeers;
}; // class LocalReduceWQModel

void LocalReduceWQMessage::complete() {
	((LocalReduceWQDevice &)_QS).__complete<LocalReduceWQMessage>(this);
	executeCallback();
}
inline XMI::Device::MessageStatus LocalReduceWQMessage::advanceThread(XMI::Device::Generic::GenericAdvanceThread *t) {
	return __advanceThread((LocalReduceWQThread *)t);
}

inline bool LocalReduceWQModel::generateMessage_impl(xmi_multicombine_t *mcomb) {
	if (mcomb->req_size < sizeof(LocalReduceWQMessage)) {
		return false;
	}
	XMI::Topology *results_topo = (XMI::Topology *)mcomb->results_participants;
	// assert((data_topo .U. results_topo).size() == _npeers);
	// This is a LOCAL reduce, results_topo must be a valid local rank!
	// assert(_g_topology_local->rank2Index(results_topo->index2Rank(0)) != -1);
	int dtshift = dcmf_dt_shift[mcomb->dtype];
	coremath func = MATH_OP_FUNCS(mcomb->dtype, mcomb->optor, 2);
	unsigned rootpeer = _g_topology_local->rank2Index(results_topo->index2Rank(0));
	LocalReduceWQMessage *msg =
		new (mcomb->request) LocalReduceWQMessage(_g_l_reducewq_dev,
				mcomb->cb_done, _shared, _peer, _npeers, rootpeer,
				(XMI::PipeWorkQueue *)mcomb->data,
				(XMI::PipeWorkQueue *)mcomb->results,
				mcomb->count, func, dtshift);
	_g_l_reducewq_dev.__post<LocalReduceWQMessage>(msg);
	return true;
}

}; // namespace Device
}; // namespace XMI

#endif // __components_devices_workqueue_localreducewqmsg_h__
