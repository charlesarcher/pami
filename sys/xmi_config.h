/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file xmi_config.h
 * \brief ???
 */

#ifndef __xmi_config_h__
#define __xmi_config_h__


#warning "xmi_config.h should be generated by configure"
#define PLATFORM_DUMMY

#ifdef PLATFORM_DUMMY
#define XMI_PROTOCOL_NQUADS	48
#define XMI_REQUEST_NQUADS	32
typedef struct XMIQuad
{
    unsigned w0; /**< Word[0] */
    unsigned w1; /**< Word[1] */
    unsigned w2; /**< Word[2] */
    unsigned w3; /**< Word[3] */
}
XMIQuad __attribute__ ((__aligned__ (16)));
typedef XMIQuad XMI_CollectiveProtocol_t [32*2];   /**< Opaque datatype for collective protocols */
typedef XMIQuad XMI_CollectiveRequest_t  [32*8*4]; /**< Opaque datatype for collective requests */
#endif

#ifdef PLATFORM_SOCKETS

#endif

#ifdef PLATFORM_BGP

#endif

#ifdef PLATFORM_BGQ

#endif

#ifdef PLATFORM_LAPIUNIX

#endif

#ifdef PLATFORM_MPI

#endif


#endif /* __xmi_config_h__ */
