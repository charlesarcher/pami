/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file ll_pipeworkqueue.cc
 * \brief PipeWorkQueue "C" interface.
 */

#include "ll_pipeworkqueue.h"
#include "PipeWorkQueue.h"
#include "Messager.h"

extern DCMF::Messager *_g_messager;

///
/// \brief Configure for Shared Circular Buffer variety.
///
/// Only one consumer and producer are allowed.
/// Creates a circular buffer of specified size in shared memory.
/// Buffer size must be power-of-two.
///
/// \param[out] wq	Opaque memory for PipeWorkQueue
/// \param[in] bufsize	Size of buffer to allocate
///
extern "C" void XMI_PipeWorkQueue_config_circ(XMI_PipeWorkQueue_t *wq, size_t bufsize) {
	LL::PipeWorkQueue *pwq =
		new (wq) LL::PipeWorkQueue();
	pwq->configure(_g_messager->sysdep(), bufsize);
}

///
/// \brief Configure for User-supplied Circular Buffer variety.
///
/// Only one consumer and producer are allowed.
/// Uses the provided buffer as a circular buffer of specified size.
/// Buffer size must be power-of-two.
/// Assumes the caller has placed buffer and (this) in appropriate memory
/// for desired use - i.e. all in shared memory if to be used beyond this process.
///
/// \param[out] wq	Opaque memory for PipeWorkQueue
/// \param[in] buffer	Buffer to use
/// \param[in] bufsize	Size of buffer
///
extern "C" void XMI_PipeWorkQueue_config_circ_usr(XMI_PipeWorkQueue_t *wq, char *buffer, size_t bufsize) {
	LL::PipeWorkQueue *pwq =
		new (wq) LL::PipeWorkQueue();
	pwq->configure(_g_messager->sysdep(), buffer, bufsize);
}

///
/// \brief Configure for Memeory (flat buffer) variety.
///
/// Only one consumer and producer are allowed. Still supports pipelining.
/// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
/// Buffer size must be power-of-two.
/// Assumes the caller has placed buffer and (this) in appropriate memory
/// for desired use - i.e. all in shared memory if to be used beyond this process.
///
/// \param[out] wq	Opaque memory for PipeWorkQueue
/// \param[in] buffer	Buffer to use
/// \param[in] bufsize	Size of buffer
/// \param[in] bufinit	Amount of data initially in buffer
///
extern "C" void XMI_PipeWorkQueue_config_flat(XMI_PipeWorkQueue_t *wq, char *buffer, size_t bufsize, size_t bufinit) {
	LL::PipeWorkQueue *pwq =
		new (wq) LL::PipeWorkQueue();
	pwq->configure(_g_messager->sysdep(), buffer, bufsize, bufinit);
}

///
/// \brief PROPOSAL: Configure for Non-Contig Memory (flat buffer) variety.
///
/// Only one consumer and producer are allowed. Still supports pipelining.
/// Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
/// Assumes the caller has placed buffer and (this) in appropriate memory
/// for desired use - i.e. all in shared memory if to be used beyond this process.
///
/// This is typically only used for the application buffer, either input or output, 
/// and so would not normally have both producer and consumer (only one or the other). 
/// The interface is the same as for contiguous data except that "bytesAvailable" will
/// only return the number of *contiguous* bytes available. The user must consume those
/// bytes before it can see the next contiguous chunk.
///
/// \param[out] wq            Opaque memory for PipeWorkQueue
/// \param[in] buffer         Buffer to use
/// \param[in] dgsp           Memory layout of a buffer unit
/// \param[in] dgspcount      Number of repetitions of buffer units
/// \param[in] dgspinit       Number of units initially in buffer
///
extern "C" void XMI_PipeWorkQueue_config_noncontig(XMI_PipeWorkQueue_t *wq, char *buffer, XMI_dgsp_t *dgsp, size_t dgspcount, size_t dgspinit) {
	LL::PipeWorkQueue *pwq =
		new (wq) LL::PipeWorkQueue();
	pwq->configure(_g_messager->sysdep(), buffer, dgsp, dgspcount, dgspinit);
}

///
/// \brief Export
///
/// Produces information about the PipeWorkQueue into the opaque buffer "export".
/// This info is suitable for sharing with other processes such that those processes
/// can then construct a PipeWorkQueue which accesses the same data stream. 
///
/// \param[in] wq             Opaque memory for PipeWorkQueue
/// \param[out] export        Opaque memory to export into
/// \return   success of the export operation
///
extern "C" XMI_Result XMI_PipeWorkQueue_export(XMI_PipeWorkQueue_t *wq, XMI_PipeWorkQueue_ext *exp) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->exp(exp);
}

///
/// \brief Import
///
/// Takes the results of an export of a PipeWorkQueue on a different process and
/// constructs a new PipeWorkQueue which the local process may use to access the
/// data stream. 
///
/// The resulting PipeWorkQueue may consume data, but that is a local-only operation.
/// The producer has no knowledge of data consumed. There can be only one producer.
/// There may be multiple consumers, but the producer does not know about them.
///
/// TODO: can this work for circular buffers? does it need to, since those are
/// normally shared memory and thus already permit inter-process communication.
///
/// \param[in] import        Opaque memory into which an export was done.
/// \param[out] wq           Opaque memory for new PipeWorkQueue
/// \return   success of the import operation
///
extern "C" XMI_Result XMI_PipeWorkQueue_import(XMI_PipeWorkQueue_ext *import, XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq =
		new (wq) LL::PipeWorkQueue();
	return pwq->import(import);
}

///
/// \brief Clone constructor.
///
/// Used to create a second local memory wrapper object of the same
/// shared memory resource.
///
/// \see WorkQueue(WorkQueue &)
///
/// \param[out] wq	Opaque memory for new PipeWorkQueue
/// \param[in] obj	old object, to be cloned
///
extern "C" void XMI_PipeWorkQueue_clone(XMI_PipeWorkQueue_t *wq, XMI_PipeWorkQueue_t *obj) {
	LL::PipeWorkQueue *_obj = (LL::PipeWorkQueue *)obj;
	new (wq) LL::PipeWorkQueue(*_obj);
}

///
/// \brief Destructor
///
/// \param[out] wq	Opaque memory for PipeWorkQueue
///
extern "C" void XMI_PipeWorkQueue_destroy(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->~PipeWorkQueue();
}

///
/// \brief Reset this shared memory work queue.
///
/// Sets the number of bytes produced and the number of bytes
/// consumed by each consumer to zero.
///
/// \param[out] wq	Opaque memory for PipeWorkQueue
///
extern "C" void XMI_PipeWorkQueue_reset(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->reset();
}

///
/// \brief Dump shared memory work queue statistics to stderr.
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \param[in] prefix Optional character string to prefix.
///
extern "C" void XMI_PipeWorkQueue_dump(XMI_PipeWorkQueue_t *wq, const char *prefix) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->dump(prefix);
}

/// \brief register a wakeup for the consumer side of the PipeWorkQueue
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \param[in] vec	Opaque wakeup vector parameter
///
extern "C" void XMI_PipeWorkQueue_setConsumerWakeup(XMI_PipeWorkQueue_t *wq, void *vec) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->setConsumerWakeup(vec);
}

/// \brief register a wakeup for the producer side of the PipeWorkQueue
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \param[in] vec	Opaque wakeup vector parameter
///
extern "C" void XMI_PipeWorkQueue_setProducerWakeup(XMI_PipeWorkQueue_t *wq, void *vec) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->setProducerWakeup(vec);
}

///
/// \brief Return the number of contiguous bytes that can be produced into this work queue.
///
/// Bytes must be produced into the memory location returned by bufferToProduce() and then
/// this work queue \b must be updated with produceBytes().
///
/// \see bufferToProduce
/// \see produceBytes
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return Number of bytes that may be produced.
///
extern "C" size_t XMI_PipeWorkQueue_bytesAvailableToProduce(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->bytesAvailableToProduce();
}

///
/// \brief Return the number of contiguous bytes that can be consumed from this work queue.
///
/// Bytes must be consumed into the memory location returned by bufferToConsume() and then
/// this work queue \b must be updated with consumeBytes().
///
/// \see bufferToConsume
/// \see consumeBytes
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \param[in] consumer Consumer id for work queues with multiple consumers
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return Number of bytes that may be consumed.
///
extern "C" size_t XMI_PipeWorkQueue_bytesAvailableToConsume(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->bytesAvailableToConsume();
}

/// \brief raw accessor for total number of bytes produced since reset()
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	number of bytes produced
///
extern "C" size_t XMI_PipeWorkQueue_getBytesProduced(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->getBytesProduced();
}

/// \brief raw accessor for total number of bytes consumed since reset()
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	number of bytes consumed
///
extern "C" size_t XMI_PipeWorkQueue_getBytesConsumed(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->getBytesConsumed();
}

/// \brief current position for producing into buffer
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	location in buffer to produce into
///
char *XMI_PipeWorkQueue_bufferToProduce(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->bufferToProduce();
}

/// \brief notify workqueue that bytes have been produced
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	number of bytes that were produced
///
extern "C" void XMI_PipeWorkQueue_produceBytes(XMI_PipeWorkQueue_t *wq, size_t bytes) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->produceBytes(bytes);
}

/// \brief current position for consuming from buffer
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	location in buffer to consume from
///
char *XMI_PipeWorkQueue_bufferToConsume(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->bufferToConsume();
}

/// \brief notify workqueue that bytes have been consumed
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	number of bytes that were consumed
///
extern "C" void XMI_PipeWorkQueue_consumeBytes(XMI_PipeWorkQueue_t *wq, size_t bytes) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	pwq->consumeBytes(bytes);
}

/// \brief is workqueue ready for action
///
/// \param[in] wq	Opaque memory for PipeWorkQueue
/// \return	boolean indicate workqueue readiness
///
extern "C" bool XMI_PipeWorkQueue_available(XMI_PipeWorkQueue_t *wq) {
	LL::PipeWorkQueue *pwq = (LL::PipeWorkQueue *)wq;
	return pwq->available();
}
