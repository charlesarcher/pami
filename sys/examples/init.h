/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file sys/examples/init.h
 * \brief PAMI code examples
 */
#ifndef __examples_init_h__
#define __examples_init_h__

/**
 * \example init_coll.c
 *
 * <b>A PAMI Initialization Test Case</b>
 *
 * This test case uses common utility routines for context creation
 * For the common initialization routines, see the following:
 *
 * \ref init_util.h
 * \ref init.c
 *
 * For collective initialization routines, see the following:
 *
 * \ref coll_util.h
 * \ref init_coll.c
 *
 * \dontinclude init_coll.c
 *
 * This is a simple "Hello World" for collectives.  The initialization
 * and shutdown procedure is very similar to the standard initialization
 * in \ref init.c.  The difference is that once the client is created,
 * we will use an object called the "world geometry" to issue collectives.
 * The \b query_geometry_world will create and initialize a geometry,
 * and return a "stock" collective, a list of "always works" collectives,
 * and a list of collectives that must be queried before use.  This example
 * focuses on the "always works" collectives.  In addition, this call returns
 * metadata for each collective.
 *
 * \par
 * \skipline Docs01
 * \until    Docs02
 *
 * Now that we have our two lists of collectives, we can begin communication.
 * First we can set the "done" callback, a "polling flag", and an algorithm
 * from the list of collectives.  All API calls in PAMI are non-blocking,
 * but we will emulate a blocking call in this test case.
 *
 * \par
 * \skipline Docs03
 * \until    Docs04
 *
 * To fully understand this test case, we should look at \ref coll_util.h
 * where the utility routines are defined.  The first step in any of the
 * geometry creation routines is to create a
 * geometry object.  In this particular example, we will create a geometry
 * that is the "world" object, that is, a geometry that contains all the nodes
 * in the current client.  The geometry object is scoped to the client, but
 * and conceptually contains a list of all the tasks within the client.
 *
 * \dontinclude coll_util.h
 * \par
 * \skipline Docs01
 * \until    Docs02
 *
 * Now that we have a "world" geometry object, we need to query it
 * for a set of available algorithms.  In this example , we want to obtain
 * first the number of available algorithms.
 * memory to contain a pami_endpoint_t element for each endpoint in the system.
 * At the end of the call to \b PAMI_Geometry_algorithms_num, the num_algorithm
 * array will contain two entries, the "always works" list, and the "must query"
 * list.  In this example, we will only use the "always works" list of
 * collectives.
 *
 * \note To use collectives in the "must query" list, additional
 *       steps must be taken to determine if the collective will work for
 *       this particular geometry and call site parameters.
 *
 * \par
 * \skipline Docs03
 * \until    Docs04
 *
 *
 * Once we know the number of algorithms that are available in the "always works"
 * list, we can query for the algorithm object for barrier.  We call the
 * \b PAMI_Geometry_algorithms_query() function call to obtain the list.  The
 * storage is allocated using the number of algorithms from the call to
 * \b PAMI_Geometry_algorithms_num().  In addition to the algorithm object
 * being returned, metadata for the object is also returned.
 *
 * \par
 * \skipline Docs05
 * \until    Docs06
 *
 * We now have our algorithm object, and can issue the barrier call.
 * We must create a transfer struct that specifies the collective specific
 * parameters, in this case, barrier.
 * Since the barrier is nonblocking, it is callback driven.  We must specify
 * a callback, an application cookie to be delivered with that callback,
 * and an algorithm to use.  In this case, we will just use the first algorithm
 * in the list.  The appliation cookie is set to a flag that will indicate
 * if the barrier is currently active or not.

 * The _barrier call emulates a blocking barrier via a nonblocking barrier
 * by issuing the barrier and then calling advance until the barrier has
 * finished.  The \b PAMI_Collective() call is used to start the barrier,
 * and the \b PAMI_Context_advance() call is used to advance the context
 * until the "done" state has been met.  The done condition is when the
 * user flag is switched from 1 to 0.  This happens
 * when the barrier is satisfied and the done callback is issued in the
 * advance routine.
 *
 * \dontinclude coll_util.h
 *
 * \par
 * \skipline Docs07
 * \until    Docs08
 *
 * The done callback signature is given by a function called cb_done.
 * All PAMI done callbacks return a context, the application cookie,
 * and an error code.  In this case, the application cookie is a pointer to
 * the polling flag, it is an unsigned value indicating the state of the
 * barrier operation.
 *
 * \dontinclude coll_util.h
 *
 * \par
 * \skipline Docs09
 * \until Docs10
 *
 * <b>Full example source code</b>
 *
 */

#endif /* __sys_examples_init_h__ */
