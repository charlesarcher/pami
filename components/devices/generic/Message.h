/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_devices_generic_Message_h__
#define __components_devices_generic_Message_h__

#include "sys/xmi.h"
#include "GenericDevicePlatform.h"

////////////////////////////////////////////////////////////////////////
///  \file components/devices/generic/Message.h
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

////////////////////////////////////////////////////////////////////////
///  \brief Message Class for insertion into queues
///
///  These classes implement a message class for insertion into queues
///
///  Definitions:
///  - Message:    A communication object that can be inserted into a q
///
////////////////////////////////////////////////////////////////////////

enum MessageStatus {
	Uninitialized = 0,	///< status for uninitialized message
	Initialized,		///< status for initialized message
	Active,			///< status for active message
	Done			///< status for completed message
};

namespace Generic {

/// \brief Base Class for Messages
///
/// Messages must be able to exist on two queues at the same time.
/// This requires two actual QueueElem structs, handled by MultiQueueElem.
/// In fact, this class is templatized by number of queue elements and thus
/// is general for any number of queues.
///
class GenericMessage : public GenericDeviceMessageQueueElem {
public:
	//////////////////////////////////////////////////////////////////////
	///  \brief Constructor
	//////////////////////////////////////////////////////////////////////
	GenericMessage(GenericDeviceMessageQueue *QS, xmi_callback_t cb,
						size_t client, size_t context) :
	GenericDeviceMessageQueueElem(),
	_status(Uninitialized),
	_QS(QS),
	_client(client),
	_context(context),
	_cb(cb)
	{
	}

	virtual ~GenericMessage() {}

	/// \brief get client associated with message
	/// \return	client for message posting/completion
	/// \ingroup gendev_subdev_api
	size_t getClientId() { return _client; }

	/// \brief get context ID associated with message
	/// \return	Context ID for message posting/completion
	/// \ingroup gendev_subdev_api
	size_t getContextId() { return _context; }

	///  \brief Query function to determine message state
	///  \return	message status
	///
	inline MessageStatus getStatus() {return _status;}

	/// \brief Set message status
	///
	/// \param[in] status	Message status to set
	///
	inline void setStatus(MessageStatus status) {_status = status;}

	/// \brief     Returns the done status of the message
	///
	/// \return	true is message is Done
	/// \ingroup gendev_subdev_api
	///
	inline bool isDone() {return (getStatus() == Done);}

	///  \brief Sets the message completion callback
	///
	/// \param[in] cb	Callback to use for message completion
	///
	void setCallback(xmi_callback_t cb) {_cb = cb;}

	///  \brief Executes the message completion callback
	///
	/// \param[in] ctx	The context object on which completion is called
	/// \param[in] err	Optional error status (default is success)
	/// \ingroup gendev_subdev_api
	///
	void executeCallback(xmi_context_t ctx, xmi_result_t err = XMI_SUCCESS) {
		if(_cb.function) _cb.function(ctx, _cb.clientdata, err);
	}

	/// \brief accessor for sub-device linked to message
	///
	/// Returns reference to device object which contains "send queues".
	/// This may not be the actual sub-device paired with the message.
	///
	/// \return	Reference to sub-device
	/// \ingroup gendev_subdev_api
	///
	inline GenericDeviceMessageQueue *getQS() { return _QS; }

	/// \brief virtual wrapper for __postNext() method
	///
	/// Used during message complete to post the next message.
	///
	/// \param[in] devQueued	was msg was previously posted to sub-device?
	/// \return	bool whether message is complete
	/// \ingroup gendev_subdev_api
	///
	virtual xmi_context_t postNext(bool devQueued) = 0;

protected:
	MessageStatus _status;
	GenericDeviceMessageQueue *_QS;
	size_t _client;
	size_t _context;
	xmi_callback_t _cb;
}; /* class GenericMessage */

}; /* namespace Generic */
}; /* namespace Device */
}; /* namespace XMI */

#endif /* __components_devices_generic_message_h__ */
