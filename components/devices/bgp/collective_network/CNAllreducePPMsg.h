/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreducePPMsg.h
 * \brief Default collective network allreduce interface.
 */
#ifndef __components_devices_bgp_collective_network_CNAllreducePPMsg_h__
#define __components_devices_bgp_collective_network_CNAllreducePPMsg_h__

#include "util/common.h"
#include "components/devices/bgp/collective_network/CNDevice.h" // externs for env vars
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "components/devices/bgp/collective_network/CNPacket.h"
#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "math/bgp/collective_network/xmi_optibgmath.h"
#include "components/devices/FactoryInterface.h"

namespace XMI {
namespace Device {
namespace BGP {

/**
 * \brief Collective Network Allreduce Send Message base class
 *
 * Collective Network Allreduce with source/dest as pipe WQ.
 */

class CNAllreducePPModel;
class CNAllreducePPMessage;
typedef XMI::Device::BGP::BaseGenericCNThread CNAllreducePPThread;
class CNAllreducePPDevice : public XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNAllreducePPThread,2> {
public:
	inline CNAllreducePPDevice(CNDevice *common) :
	XMI::Device::Generic::SharedQueueSubDevice<CNDevice,CNAllreducePPThread,2>(common) {
	}

	class Factory : public Interface::FactoryInterface<Factory,CNAllreducePPDevice,Generic::Device> {
	public:
		static inline CNAllreducePPDevice *generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager & mm);
		static inline xmi_result_t init_impl(CNAllreducePPDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices);
		static inline size_t advance_impl(CNAllreducePPDevice *devs, size_t client, size_t context);
		static inline CNAllreducePPDevice & getDevice_impl(CNAllreducePPDevice *devs, size_t client, size_t context);
	}; // class Factory
}; // class CNAllreducePPDevice

};	// BGP
};	// Device
};	// XMI

extern XMI::Device::BGP::CNAllreducePPDevice _g_cnallreducepp_dev;

namespace XMI {
namespace Device {
namespace BGP {

inline CNAllreducePPDevice *CNAllreducePPDevice::Factory::generate_impl(size_t client, size_t num_ctx, Memory::MemoryManager &mm) {
	return &_g_cnallreducepp_dev;
}

inline xmi_result_t CNAllreducePPDevice::Factory::init_impl(CNAllreducePPDevice *devs, size_t client, size_t contextId, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd, XMI::Device::Generic::Device *devices) {
	return _g_cnallreducepp_dev.__init(client, contextId, clt, ctx, sd, devices);
}

inline size_t CNAllreducePPDevice::Factory::advance_impl(CNAllreducePPDevice *devs, size_t client, size_t contextId) {
	return 0;
}

inline CNAllreducePPDevice & CNAllreducePPDevice::Factory::getDevice_impl(CNAllreducePPDevice *devs, size_t client, size_t contextId) {
	return _g_cnallreducepp_dev;
}

class CNAllreducePPMessage : public XMI::Device::BGP::BaseGenericCNPPMessage {
	enum roles {
		NO_ROLE = 0,
		INJECTION_ROLE = (1 << 0), // first role must be "injector"
		RECEPTION_ROLE = (1 << 1), // last role must be "receptor"
	};
public:
	CNAllreducePPMessage(GenericDeviceMessageQueue *qs,
			xmi_multicombine_t *mcomb,
			size_t bytes,
			bool doStore,
			unsigned dispatch_id,
			XMI::Device::BGP::CNAllreduceSetup tas) :
	BaseGenericCNPPMessage(qs, mcomb->client, mcomb->context,
				(XMI::PipeWorkQueue *)mcomb->data,
				(XMI::PipeWorkQueue *)mcomb->results,
				bytes, doStore, mcomb->roles, mcomb->cb_done,
				dispatch_id, tas),
	_roles(mcomb->roles)
	{
	}

	// virtual function
	xmi_context_t postNext(bool devQueued) {
		return _g_cnallreducepp_dev.common()->__postNext<CNAllreducePPMessage,CNAllreducePPThread>(this, devQueued);
	}

	// _bytesLeft == bytes on network!
	inline int setThreads(CNAllreducePPThread **th) {
		CNAllreducePPThread *t;
		int n;
		_g_cnallreducepp_dev.__getThreads(&t, &n);
		int nt = 0;
		_g_cnallreducepp_dev.common()->__resetThreads();
		_nThreads = ((_roles & INJECTION_ROLE) != 0) + ((_roles & RECEPTION_ROLE) != 0);
		if (_roles & INJECTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setAdv(advanceInj);
			t[nt].setStatus(XMI::Device::Ready);
			t[nt]._wq = _swq;
			t[nt]._bytesLeft = _bytes << _allreduceSetup._logbytemult;
			t[nt]._cycles = 1;
			__advanceInj(&t[nt]);
			++nt;
		}
		if (_roles & RECEPTION_ROLE) {
			t[nt].setMsg(this);
			t[nt].setAdv(advanceRcp);
			t[nt].setStatus(XMI::Device::Ready);
			t[nt]._wq = _rwq;
			t[nt]._bytesLeft = _bytes << _allreduceSetup._logbytemult;
			t[nt]._cycles = 3000; // DCMF_PERSISTENT_ADVANCE;
			__advanceRcp(&t[nt]);
			++nt;
		}
		// assert(nt > 0? && nt < n);
		*th = t;
		return nt;
	}

protected:
	inline void __completeThread(CNAllreducePPThread *thr);

	DECL_ADVANCE_ROUTINE(advanceInj,CNAllreducePPMessage,CNAllreducePPThread);
	DECL_ADVANCE_ROUTINE(advanceRcp,CNAllreducePPMessage,CNAllreducePPThread);
	inline xmi_result_t __advanceInj(CNAllreducePPThread *thr) {
		if (thr->_bytesLeft == 0) return XMI_SUCCESS;
		unsigned hcount = BGPCN_FIFO_SIZE, dcount = BGPCN_QUADS_PER_FIFO;
		size_t avail = thr->_wq->bytesAvailableToConsume();
		char *buf = thr->_wq->bufferToConsume();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (__wait_send_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI_EAGAIN;
		}
		__send_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__send_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->consumeBytes(did);
			if (thr->_bytesLeft == 0) {
				thr->setStatus(XMI::Device::Complete);
				__completeThread(thr);
				return XMI_SUCCESS;
			}
		}
		return XMI_EAGAIN;
	}


	inline xmi_result_t __advanceRcp(CNAllreducePPThread *thr) {
		if (thr->_bytesLeft == 0) return XMI_SUCCESS;
		unsigned hcount = 0, dcount = 0;
		size_t avail = thr->_wq->bytesAvailableToProduce();
		char *buf = thr->_wq->bufferToProduce();
		bool aligned = (((unsigned)buf & 0x0f) == 0);
		size_t did = 0;
		if (__wait_recv_fifo_to(thr, hcount, dcount, thr->_cycles)) {
			return XMI_EAGAIN;
		}
		__recv_whole_packets(thr, hcount, dcount, avail, did, buf, aligned);
		__recv_last_packet(thr, hcount, dcount, avail, did, buf, aligned);
		if (did) {
			thr->_wq->produceBytes(did);
			if (thr->_bytesLeft == 0) {
				thr->setStatus(XMI::Device::Complete);
				__completeThread(thr);
				return XMI_SUCCESS;
			}
		}
		return XMI_EAGAIN;
	}

	unsigned _roles;
	unsigned _nThreads;
}; // class CNAllreducePPMessage

class CNAllreducePPModel : public XMI::Device::Interface::MulticombineModel<CNAllreducePPModel,CNAllreducePPDevice,sizeof(CNAllreducePPMessage)> {
public:
	static const int NUM_ROLES = 2;
	static const int REPL_ROLE = -1;
	static const size_t sizeof_msg = sizeof(CNAllreducePPMessage);

	CNAllreducePPModel(CNAllreducePPDevice &device, xmi_result_t &status) :
	XMI::Device::Interface::MulticombineModel<CNAllreducePPModel,CNAllreducePPDevice,sizeof(CNAllreducePPMessage)>(device,status)
	{
		// assert(device == _g_cnallreducepp_dev);
		_dispatch_id = _g_cnallreducepp_dev.newDispID();
		_me = __global.mapping.task();
	}

	inline xmi_result_t postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb);

private:
	size_t _me;
	unsigned _dispatch_id;
}; // class CNAllreducePPModel

inline void CNAllreducePPMessage::__completeThread(CNAllreducePPThread *thr) {
	unsigned c = _g_cnallreducepp_dev.common()->__completeThread(thr);
	if (c >= _nThreads) {
		setStatus(XMI::Device::Done);
	}
}

// Permit a NULL results_topo to mean "everyone" (i.e. "root == -1")
inline xmi_result_t CNAllreducePPModel::postMulticombine_impl(uint8_t (&state)[sizeof_msg], xmi_multicombine_t *mcomb) {
	XMI::Device::BGP::CNAllreduceSetup &tas =
		XMI::Device::BGP::CNAllreduceSetup::getCNAS(mcomb->dtype, mcomb->optor);
	// assert(tas._pre != NULL);
	if (!tas._pre || !tas._post) {
XMI_abort();
		return XMI_ERROR;
	}
	XMI::Topology *result_topo = (XMI::Topology *)mcomb->results_participants;
	bool doStore = (!result_topo || result_topo->isRankMember(_me));
	size_t bytes = mcomb->count << xmi_dt_shift[mcomb->dtype];

	// could try to complete allreduce before construction, but for now the code
	// is too dependent on having message and thread structures to get/keep context.
	// __post() will still try early advance... (after construction)
	CNAllreducePPMessage *msg;
	msg = new (&state) CNAllreducePPMessage(_g_cnallreducepp_dev.common(),
			mcomb, bytes, doStore, _dispatch_id, tas);
	_g_cnallreducepp_dev.__post<CNAllreducePPMessage>(msg);
	return XMI_SUCCESS;
}

};	// BGP
};	// Device
};	// XMI

#endif // __component_devices_bgp_cnallreduceppmsg_h__
