/**
 * \file hl_collectives.h
 * \brief Common external collective layer interface.
 */

#ifndef __hl_collective_h__
#define __hl_collective_h__

#include "ll_collectives_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ************************************************************************* */
    /* **************     ALGORITHMS  ****************************************** */
    /* ************************************************************************* */
    /**
     * \brief Supported Algorithm Types:  Not Complete, add as many algorithms as you want!
     */
    typedef enum
    {
	HL_DEFAULT_BARRIER_PROTOCOL,
        HL_CAU_BARRIER_PROTOCOL,             /**< Global Interrupt barrier. */
        HL_GI_BARRIER_PROTOCOL,             /**< Global Interrupt barrier. */
        HL_TREE_BARRIER_PROTOCOL,           /**< Tree barrier. */
        HL_RECTANGLE_BARRIER_PROTOCOL,      /**< Rectangular shape barrier. */
        HL_BINOMIAL_BARRIER_PROTOCOL,       /**< Rectangular shape barrier. */
        HL_LOCKBOX_BARRIER_PROTOCOL,        /**< Lockbox Barrier protocol. */
        HL_IPC_BARRIER_PROTOCOL,            /**< IPC barrier. */
        HL_NUM_BARRIER_PROTOCOLS
    }HL_Barrier_Protocol_t;

    typedef enum
    {
	HL_DEFAULT_BROADCAST_PROTOCOL,
        HL_TREE_BROADCAST_PROTOCOL,            /**< Tree broadcast.  */
        HL_RECTANGLE_BROADCAST_PROTOCOL,       /**< rect broadcast. */
        HL_RECTANGLE_BROADCAST_PROTOCOL_DPUT,  /**< rect broadcast with direct put broadcasts. */
        HL_BINOMIAL_BROADCAST_PROTOCOL,        /**< binom broadcast. */
        HL_IPC_BROADCAST_PROTOCOL,             /**< IPC broadcast.   */
        HL_NUM_BROADCAST_PROTOCOLS
    }HL_Broadcast_Protocol_t;

    typedef enum
    {
	HL_DEFAULT_ALLGATHER_PROTOCOL,
        HL_TREE_ALLGATHER_PROTOCOL,            /**< Tree allgather.  */
        HL_RECTANGLE_ALLGATHER_PROTOCOL,       /**< rect allgather. */
        HL_RECTANGLE_ALLGATHER_PROTOCOL_DPUT,  /**< rect allgather with direct put allgathers. */
        HL_BINOMIAL_ALLGATHER_PROTOCOL,        /**< binom allgather. */
        HL_IPC_ALLGATHER_PROTOCOL,             /**< IPC allgather.   */
        HL_NUM_ALLGATHER_PROTOCOLS
    }HL_Allgather_Protocol_t;

    typedef enum
    {
	HL_DEFAULT_ALLGATHERV_PROTOCOL,
        HL_TREE_ALLGATHERV_PROTOCOL,            /**< Tree allgatherv.  */
        HL_RECTANGLE_ALLGATHERV_PROTOCOL,       /**< rect allgatherv. */
        HL_RECTANGLE_ALLGATHERV_PROTOCOL_DPUT,  /**< rect allgatherv with direct put allgathervs. */
        HL_BINOMIAL_ALLGATHERV_PROTOCOL,        /**< binom allgatherv. */
        HL_IPC_ALLGATHERV_PROTOCOL,             /**< IPC allgatherv.   */
        HL_NUM_ALLGATHERV_PROTOCOLS
    }HL_Allgatherv_Protocol_t;


    typedef enum
    {
	HL_DEFAULT_SCATTER_PROTOCOL,
        HL_TREE_SCATTER_PROTOCOL,            /**< Tree scatter.  */
        HL_RECTANGLE_SCATTER_PROTOCOL,       /**< rect scatter. */
        HL_RECTANGLE_SCATTER_PROTOCOL_DPUT,  /**< rect scatter with direct put scatters. */
        HL_BINOMIAL_SCATTER_PROTOCOL,        /**< binom scatter. */
        HL_IPC_SCATTER_PROTOCOL,             /**< IPC scatter.   */
        HL_NUM_SCATTER_PROTOCOLS
    }HL_Scatter_Protocol_t;

    typedef enum
    {
	HL_DEFAULT_SCATTERV_PROTOCOL,
        HL_TREE_SCATTERV_PROTOCOL,            /**< Tree scatterv.  */
        HL_RECTANGLE_SCATTERV_PROTOCOL,       /**< rect scatterv. */
        HL_RECTANGLE_SCATTERV_PROTOCOL_DPUT,  /**< rect scatterv with direct put scattervs. */
        HL_BINOMIAL_SCATTERV_PROTOCOL,        /**< binom scatterv. */
        HL_IPC_SCATTERV_PROTOCOL,             /**< IPC scatterv.   */
        HL_NUM_SCATTERV_PROTOCOLS
    }HL_Scatterv_Protocol_t;

    typedef enum
    {
        HL_ASYNCBROADCAST_BINOMIAL_PROTOCOL,
        HL_ASYNCBROADCAST_RECTANGLE_PROTOCOL,
        HL_NUM_ASYNCBROADCAST_PROTOCOLS
    }HL_AsyncBroadcast_Protocol_t;

    typedef enum
    {
	HL_DEFAULT_ALLREDUCE_PROTOCOL,
        HL_TREE_ALLREDUCE_PROTOCOL,                 /**< Tree allreduce. */
        HL_BINOMIAL_ALLREDUCE_PROTOCOL,             /**< binomial allreduce. */
        HL_RECTANGLE_ALLREDUCE_PROTOCOL,            /**< rectangle/binomial allreduce. */
        HL_RECTANGLE_RING_ALLREDUCE_PROTOCOL,       /**< rectangle/ring allreduce. */
        HL_TREE_PIPELINED_ALLREDUCE_PROTOCOL,       /**< Tree allreduce. */
        HL_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL,       /**< binomial short allreduce. */
        HL_TREE_DPUT_PIPELINED_ALLREDUCE_PROTOCOL,  /**< Tree allreduce. */
        HL_ASYNC_BINOMIAL_ALLREDUCE_PROTOCOL,       /**< binomial allreduce. */
        HL_ASYNC_RECTANGLE_ALLREDUCE_PROTOCOL,      /**< rectangle/binomial async allreduce. */
        HL_ASYNC_RECTANGLE_RING_ALLREDUCE_PROTOCOL, /**< rectangle/ring async allreduce. */
        HL_ASYNC_SHORT_BINOMIAL_ALLREDUCE_PROTOCOL, /**< binomial short async allreduce. */
        HL_ASYNC_SHORT_RECTANGLE_ALLREDUCE_PROTOCOL,/**< rectangle short async allreduce. */
        HL_RRING_DPUT_ALLREDUCE_PROTOCOL_SINGLETH,  /**< rectangle allreduce with direct puts (needs 16B alignmened buffers) */
        HL_IPC_ALLREDUCE_PROTOCOL,                  /**< IPC allreduce. */
        HL_NUM_ALLREDUCE_PROTOCOLS
    }HL_Allreduce_Protocol_t;

    typedef enum
    {
        HL_TREE_REDUCE_PROTOCOL,           /**< Tree reduce. */
        HL_BINOMIAL_REDUCE_PROTOCOL,       /**< binomial reduce. */
        HL_RECTANGLE_REDUCE_PROTOCOL,      /**< rectangle/binomial reduce. */
        HL_RECTANGLE_RING_REDUCE_PROTOCOL, /**< rectangle/ring reduce. */
        HL_SHORT_BINOMIAL_REDUCE_PROTOCOL, /**< binomial reduce. */
        HL_NUM_REDUCE_PROTOCOLS
    }HL_Reduce_Protocol_t;

    /*  add alltoall */

    typedef enum
    {
	HL_DEFAULT_ALLTOALLV_PROTOCOL,
        HL_ALLTOALLV_PROTOCOL,       /**< Alltoallv. */
        HL_IPC_ALLTOALLV_PROTOCOL,   /**< IPC Alltoallv. */
        HL_NUM_ALLTOALLV_PROTOCOLS
    }HL_Alltoallv_Protocol_t;


    typedef enum
	{
	    HL_CFG_BROADCAST = 0,
	    HL_CFG_ALLREDUCE,
	    HL_CFG_REDUCE,
	    HL_CFG_ALLGATHER,
	    HL_CFG_ALLGATHERV,
	    HL_CFG_SCATTER,
	    HL_CFG_SCATTERV,
	    HL_CFG_BARRIER,
	    HL_CFG_ALLTOALLV,
	    HL_CFG_COUNT
	}hl_config_t;

    /**
     * \brief Opaque data type that holds geometry information.
     *
     * The external API does not reveal the internal structure of the object.
     *
     * \internal Implementations should verify that geometry objects
     *           need less space than \c sizeof(HL_Geometry_t).
     *
     * \todo     Need to adjust size to optimal level (currently 2048 bytes).
     */
    typedef Quad  HL_Geometry_t [32];
    typedef struct 
    {
	size_t lo;
	size_t hi;
    }HL_Geometry_range_t;

    extern HL_Geometry_t  HL_World_Geometry;
    unsigned              HL_World_Geometry_id=0;

    /* *********************************************************************************** */
    /* **************    "Special Callback Types Objects  ******************************** */
    /* *********************************************************************************** */
    /**
     *  \brief A callback to map the geometry id to the geometry
     *  structure. The runtime has to be able to freely translate
     *  between geometry id's and geometry structures.
     */
    typedef HL_Geometry_t * (*HL_mapIdToGeometry) (int comm);


    /**
     * \brief Asynchronous broadcast receive callback.
     *
     * \todo doxygen
     */
    typedef void * (*HL_RecvAsyncBroadcast) (unsigned           root,
                                             unsigned           comm,
                                             const unsigned     sndlen,
                                             unsigned         * rcvlen,
                                             char            ** rcvbuf,
                                             HL_Callback_t    * const cb_info);



    /* ************************************************************************* */
    /* **************    Configuration Objects  ******************************** */
    /* ************************************************************************* */
    /**
     * \brief Configuration Objects for collectives
     *        Use these to set the algorithm types
     *        Register the protocol implementation
     *        specified by the asynchronous broadcast configuration.
     *
     * \warning After registering the protocol information it is illegal to
     *          deallocate the registration object.
     *
     * \param[out] registration  Opaque memory to maintain registration information.
     * \param[in]  configuration Asynchronous broadcast configuration information.
     *
     * \retval     0            Success
     *
     * \see HL_AsyncBroadcast
     */
    /*  Clear up mapId to geometry!!!! */

    typedef struct
    {
        hl_config_t             cfg_type;
        HL_mapIdToGeometry      cb_geometry;       /**< Callback to get the geometry when the async packet arrives. **/
        HL_Broadcast_Protocol_t protocol;          /**< The broadcast protocol implementation to register. */
    }HL_Broadcast_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        HL_mapIdToGeometry      cb_geometry;       /**< Callback to get the geometry when the async packet arrives. **/
        HL_Allgather_Protocol_t protocol;          /**< The allgather protocol implementation to register. */
    }HL_Allgather_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        HL_mapIdToGeometry      cb_geometry;       /**< Callback to get the geometry when the async packet arrives. **/
        HL_Allgatherv_Protocol_t protocol;          /**< The allgather protocol implementation to register. */
    }HL_Allgatherv_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        HL_mapIdToGeometry      cb_geometry;       /**< Callback to get the geometry when the async packet arrives. **/
        HL_Scatter_Protocol_t protocol;          /**< The scatter protocol implementation to register. */
    }HL_Scatter_Configuration_t;

    typedef struct
    {
        hl_config_t             cfg_type;
        HL_mapIdToGeometry      cb_geometry;       /**< Callback to get the geometry when the async packet arrives. **/
        HL_Scatterv_Protocol_t protocol;          /**< The scatter protocol implementation to register. */
    }HL_Scatterv_Configuration_t;

    typedef struct
    {
        hl_config_t                cfg_type;
        HL_AsyncBroadcast_Protocol_t protocol;     /**< The asynchronous broadcast protocol implementation to register. */
        HL_RecvAsyncBroadcast      cb_recv;      /**< Callback to invoke to receive an asynchronous broadcast message. */
        HL_mapIdToGeometry         cb_geometry;  /**< Callback to get the geometry when the async bcast packet arrives **/
        unsigned                   isBuffered;   /**< Should the broadcast buffer unexpected incoming broadcast messages
                                                    as opposed to calling cb_recv to allocate a buffer for the message*/
    }HL_AsyncBroadcast_Configuration_t;

    typedef struct
    {
        hl_config_t           cfg_type;
        HL_Allreduce_Protocol_t protocol;       /**< The allreduce protocol implementation to register. */
        HL_mapIdToGeometry    cb_geometry;    /**< Callback to get the geometry when the async packet arrives **/
        unsigned              reuse_storage:1;/**< Reuse malloc'd storage across calls if set. Otherwise, free it. */
        unsigned              reserved:31;    /**< Currently unused */
    }HL_Allreduce_Configuration_t;

    typedef struct
    {
        hl_config_t        cfg_type;
        HL_Reduce_Protocol_t protocol;       /**< The reduce protocol implementation to register. */
        HL_mapIdToGeometry cb_geometry;    /**< Callback to get the geometry when the async packet arrives. **/
        unsigned           reuse_storage:1;/**< Reuse malloc'd storage across calls if set. Otherwise, free it. */
        unsigned           reserved:31;    /**< Reserved for future use. */
    }HL_Reduce_Configuration_t;

    typedef struct
    {
        hl_config_t           cfg_type;
        HL_Barrier_Protocol_t protocol;    /**< The barrier protocol implementation to register. */
        HL_mapIdToGeometry    cb_geometry; /**< This function is used to look-up the associated
                                              geometry when doing a barrier */
    }HL_Barrier_Configuration_t;

    typedef struct
    {
        hl_config_t           cfg_type;
        HL_Barrier_Protocol_t protocol;    /**< The barrier protocol implementation to register. */
        HL_mapIdToGeometry    cb_geometry; /**< This function is used to look-up the associated
                                              geometry when doing a barrier */
    }HL_Alltoallv_Configuration_t;

    typedef union
    {
        hl_config_t                       cfg_type;
        HL_Reduce_Configuration_t         cfg_reduce;
        HL_Allreduce_Configuration_t      cfg_allreduce;
        HL_AsyncBroadcast_Configuration_t cfg_asyncbroadcast;
        HL_Allgather_Configuration_t      cfg_allgather;
        HL_Allgatherv_Configuration_t     cfg_allgatherv;
        HL_Scatter_Configuration_t        cfg_scatter;
        HL_Scatterv_Configuration_t       cfg_scatterv;
        HL_Broadcast_Configuration_t      cfg_broadcast;
        HL_Alltoallv_Configuration_t      cfg_alltoallv;
        HL_Barrier_Configuration_t        cfg_barrier;
    }   HL_CollectiveConfiguration_t;


    int HL_Collectives_initialize(int argc, char*argv[]);
    int HL_Collectives_finalize();
    /* Rename  to CollectiveRegistration_T , doxygen with Storage variable storage, others*/
    int HL_register(HL_CollectiveProtocol_t      *registration,
                    HL_CollectiveConfiguration_t *HL_CollectiveConfiguration_t,
		    int                           key);



    /* ************************************************************************* */
    /* ********* Transfer Types, used by geometry and xfer routines ************ */
    /* ************************************************************************* */
    typedef enum
	{
            HL_XFER_BROADCAST = 0,
            HL_XFER_ALLREDUCE,
            HL_XFER_REDUCE,
            HL_XFER_ALLGATHER,
            HL_XFER_ALLGATHERV,
            HL_XFER_SCATTER,
            HL_XFER_SCATTERV,
            HL_XFER_BARRIER,
            HL_XFER_ALLTOALLV,
            HL_XFER_COUNT
        }hl_xfer_type_t;

    /* ************************************************************************* */
    /* **************     Geometry (like groups/communicators)  **************** */
    /* ************************************************************************* */
    /**
     * \brief Initialize the geometry
     *
     * \param[out] geometry        Opaque geometry object to initialize
     * \param[in]  id              Unique identifier for this geometry
     *                             which globally represents this geometry
     * \param[in]  rank_slices     Array of node slices participating in the geometry
     *                             User must keep the array of slices in memory for the
     *                             duration of the geometry's existence
     * \param[in]  slice_count     Number of nodes participating in the geometry
     */
    int HL_Geometry_initialize (HL_Geometry_t            * geometry,
                                unsigned                   id,
                                HL_Geometry_range_t      * rank_slices,
                                unsigned                   slice_count);

    /**
     * \brief Analyze a protocol to ensure that it will run on the specified geometry
     *
     * \param[in]     geometry   An input geometry to be analyzed.
     * \param[in/out] protocol   A list of protocol pointers to be checked for this geometry
     *                           pointers will be set to NULL for invalid protocols on 
     *                           this geometry.
     * \param[in/out] num        number of protocols in the list in/requested, out/actual
     * \retval        HL_SUCCESS The protocol will run on the current geometry
     * \retval        ?????      The protocol does not support the current geometry
     */
    int HL_Geometry_algorithm (HL_Geometry_t            *geometry,
			       HL_CollectiveProtocol_t **protocols,
			       int                      *num);

    /**
     * \brief Free any memory allocated inside of a geometry. Mostly
     * the alltoall permutation array right now.
     * \param[in] geometry The geometry object to free
     * \retval HL_SUCCESS Memory free didn't fail
     */
    int HL_Geometry_finalize(HL_Geometry_t *geometry);

    /* *********************************************************************************** */
    /* **************    "Transfer Routine and Objects    ******************************** */
    /* *********************************************************************************** */
    /**
     * \brief Create and post a non-blocking alltoall vector operation.
     *
     * The alltoallv operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  sndbuf      The base address of the buffers containing data to be sent
     * \param[in]  sndlens     How much data to send to each node
     * \param[in]  sdispls     Where to find the data in sndbuf
     * \param[out] rcvbuf      The base address of the buffer for data reception
     * \param[in]  rcvlens     How much data to receive from each node
     * \param[in]  rdispls     Where to put the data in rcvbuf
     * \param[in]  sndcounters ???
     * \param[in]  rcvcounters ???
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        char                     * sndbuf;
        unsigned                 * sndlens;
        unsigned                 * sdispls;
        char                     * rcvbuf;
        unsigned                 * rcvlens;
        unsigned                 * rdispls;
        unsigned                 * sndcounters;
        unsigned                 * rcvcounters;
    }hl_alltoall_t;

    /**
     * \brief Create and post a non-blocking reduce operation.
     *
     * The reduce operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  root         Rank of the reduce root node.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  count        Number of elements to reduce.
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t            xfer_type;
        HL_CollectiveProtocol_t * registration;
        HL_CollectiveRequest_t  * request;
        HL_Callback_t             cb_done;
        HL_Geometry_t           * geometry;
        int                       root;
        char                    * sbuffer;
        char                    * rbuffer;
        unsigned                  count;
        HL_Dt                     dt;
        HL_Op                     op;
    }hl_reduce_t;

    /**
     * \brief Create and post a non-blocking broadcast operation.
     *
     * The broadcast operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  root         Rank of the node performing the broadcast.
     * \param[in]  src          Source buffer to broadcast.
     * \param[in]  bytes        Number of bytes to broadcast.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        char                     * dst;
        unsigned                   bytes;
    }hl_broadcast_t;

    /**
     * \brief Create and post a non-blocking allgather
     *
     * The allgather
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to allgather.
     * \param[in]  bytes        Number of bytes to allgather.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        char                     * src;
        char                     * dst;
        size_t                     bytes;
    }hl_allgather_t;

    /**
     * \brief Create and post a non-blocking allgatherv
     *
     * The allgatherv
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to allgatherv.
     * \param[in]  bytes        Number of bytes to allgatherv.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        char                     * src;
        char                     * dst;
        size_t                   * lengths;
    }hl_allgatherv_t;

    /**
     * \brief Create and post a non-blocking scatter
     *
     * The scatter
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to scatter.
     * \param[in]  bytes        Number of bytes to scatter.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        char                     * dst;
        size_t                     bytes;
    }hl_scatter_t;

    /**
     * \brief Create and post a non-blocking scatterv
     *
     * The scatterv
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  src          Source buffer to scatterv.
     * \param[in]  bytes        Number of bytes to scatterv.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        char                     * dst;
        size_t                   * lengths;
    }hl_scatterv_t;


    /**
     * \brief Create and post a non-blocking asynchronous broadcast operation.
     *
     * The asynchronous broadcast operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  root         Rank of the node performing the broadcast.
     * \param[in]  src          Source buffer to broadcast.
     * \param[in]  bytes        Number of bytes to broadcast.
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t             xfer_type;
        HL_CollectiveProtocol_t  * registration;
        HL_CollectiveRequest_t   * request;
        HL_Callback_t              cb_done;
        HL_Geometry_t            * geometry;
        unsigned                   root;
        char                     * src;
        unsigned                   bytes;
    }hl_async_broadcast_t;

    /**
     * \brief Create and post a non-blocking allreduce operation.
     *
     * The allreduce operation ...
     *
     * \warning Until the message callback is invoked, it is illegal to send it
     *          again, reset it, touch the attached buffers, or deallocate the
     *          request object.
     *
     * \param[in]  registration Protocol registration.
     * \param[in]  request      Opaque memory to maintain internal message state.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     * \param[in]  geometry     Geometry to use for this collective operation.
     *                          \c NULL indicates the global geometry.
     * \param[in]  sbuffer      Source buffer.
     * \param[in]  rbuffer      Receive buffer.
     * \param[in]  count        Number of elements to allreduce.
     * \param[in]  dt           Element data type
     * \param[in]  op           Reduce operation
     *
     * \retval     0            Success
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t            xfer_type;
        HL_CollectiveProtocol_t * registration;
        HL_CollectiveRequest_t  * request;
        HL_Callback_t             cb_done;
        HL_Geometry_t           * geometry;
        char                    * src;
        char                    * dst;
        unsigned                  count;
        HL_Dt                     dt;
        HL_Op                     op;
    }hl_allreduce_t;


    /**
     * \brief Create and post a non-blocking barrier operation.
     *
     * The barrier operation ...
     *
     * \param   geometry     Geometry to use for this collective operation.
     *                       \c NULL indicates the global geometry.
     * \param[in]  cb_done      Callback to invoke when message is complete.
     *
     * \retval  0            Success
     *
     * \see HL_Barrier_register
     *
     * \todo doxygen
     */
    typedef struct
    {
        hl_xfer_type_t            xfer_type;
        HL_CollectiveProtocol_t * registration;
        HL_CollectiveRequest_t  * request;
        HL_Callback_t             cb_done;
        HL_Geometry_t           * geometry;
    }hl_barrier_t;

    typedef union
    {
	hl_xfer_type_t        xfer_type;
        hl_allreduce_t        xfer_allreduce;
        hl_async_broadcast_t  xfer_async_broadcast;
        hl_broadcast_t        xfer_broadcast;
        hl_reduce_t           xfer_reduce;
        hl_allgather_t        xfer_allgather;
        hl_allgatherv_t       xfer_allgatherv;
        hl_scatter_t          xfer_scatter;
        hl_scatterv_t         xfer_scatterv;
        hl_alltoall_t         xfer_alltoall;
	hl_barrier_t          xfer_barrier;
    }hl_xfer_t;

    int HL_Xfer (void *context, hl_xfer_t *cmd);
    int HL_Poll();
    int HL_Rank();
    int HL_Size();

#ifdef __cplusplus
};
#endif

#endif
