/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/common.h
 * \brief Basic header file to define simple and common items
 */
#ifndef __util_common_h__
#define __util_common_h__

#include <new>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#if defined(__xlc__) || defined(__xlC__)
#include <builtins.h>
#endif

#include "sys/pami.h"
#include "Compiler.h"
#include "Arch.h"

/** \todo set the client information in the endpoint opaque type */
#define PAMI_ENDPOINT_INIT(client,task,offset) ((offset << 23) | task)
#define PAMI_ENDPOINT_INFO(endpoint,task,offset) { task = endpoint & 0x007fffff; offset = (endpoint >> 23) & 0x03f; }


#ifndef MIN
#define MIN(a,b)  (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b)  (((a)>(b))?(a):(b))
#endif

#ifndef CEIL
#define CEIL(x,y) (((x)+(y)-1)/(y))
#endif

/*
 * The following are only respected by GCC and the latest XL
 * compilers.  The "expected" value is assumed to be either a 1 or 0,
 * so best results are achieved when your expression evaluates to 1 or
 * 0.  It makes sense to use unlikely for error checking, since it
 * will move the object code away from the good-path.
 *
 * They can be used like this:

  if (likely(expression_that_is_probably_one(input)) { do_something() }

  if (unlikely(error_check_expression_that_is_probably_zero(input)) { do_something_to_handle_error() }

 */
#define   likely(x) (__builtin_expect(x,1))
#define unlikely(x) (__builtin_expect(x,0))

#define PAMIQuad_sizeof(x)  ((sizeof(x)+15)>>4)

/// abort macros defined for all assertion levels
#define PAMI_abort()                       abort()
#define PAMI_abortf(fmt...)                { fprintf(stderr, __FILE__ ":%d: \n", __LINE__); fprintf(stderr, fmt); abort(); }

#ifndef ASSERT_LEVEL
#define ASSERT_LEVEL 2
#warning ASSERT_LEVEL not set by config.  Defaulting to all asserts enabled
#endif

#if ASSERT_LEVEL==0    // All asserts are disabled
#define PAMI_assert(expr)
#define PAMI_assertf(expr, fmt...)
#define PAMI_assert_debug(expr)
#define PAMI_assert_debugf(expr, fmt...)

#elif ASSERT_LEVEL==1  // Only "normal" asserts, not debug, are enabled
#define PAMI_assert(expr)                assert(expr)
#define PAMI_assertf(expr, fmt...)       { if (!(expr)) PAMI_abortf(fmt); }
#define PAMI_assert_debug(expr)
#define PAMI_assert_debugf(expr, fmt...)

#else // ASSERT_LEVEL==2 ... All asserts are enabled
#define PAMI_assert(expr)                assert(expr)
#define PAMI_assertf(expr, fmt...)       { if (!(expr)) PAMI_abortf(fmt); }
#define PAMI_assert_debug(expr)          assert(expr)
#define PAMI_assert_debugf(expr, fmt...) PAMI_assertf(expr, fmt)

#endif // ASSERT_LEVEL


static inline int64_t min_nb64(int64_t x, int64_t y)
{
  return x + (((y - x) >> (63))&(y - x));
}

static inline int32_t min_nb32(int32_t x, int32_t y)
{
  return x + (((y - x) >> (31))&(y - x));
}

static inline int64_t max_nb64(int64_t x, int64_t y)
{
  return x - (((x - y) >> (63))&(x - y));
}

static inline int32_t max_nb32(int32_t x, int32_t y)
{
  return x - (((x - y) >> (31))&(x - y));
}
#if 0
inline void* operator new(size_t obj_size, void* pointer)
{
  /*   printf("%s: From %p for %u\n", __PRETTY_FUNCTION__, pointer, obj_size); */
//  CCMI_assert_debug(pointer != NULL);
  return pointer;
}
#endif

/**
 * \brief Creates a compile error if the condition is false.
 *
 * This macro must be used within a function for the compiler to process it.
 * It is suggested that C++ classes and C files create an inline function
 * similar to the following example. The inline function is never used at
 * runtime and should be optimized out by the compiler. It exists for the sole
 * purpose of moving runtime \c assert calls to compile-time errors.
 *
 * \code
 * static inline void compile_time_assert ()
 * {
 *   // This compile time assert will succeed.
 *   COMPILE_TIME_ASSERT(sizeof(char) <= sizeof(double));
 *
 *   // This compile time assert will fail.
 *   COMPILE_TIME_ASSERT(sizeof(double) <= sizeof(char));
 * }
 * \endcode
 *
 * Compile time assert errors will look similar to the following:
 *
 * \code
 * foo.h: In function compile_time_assert:
 * foo.h:43: error: duplicate case value
 * foo.h:43: error: previously used here
 * \endcode
 *
 * \note C++ template code must actually invoke the compile_time_assert
 *       function, typically in a class constructor, for the assertion
 *       to be evaluated. This is because the compile will not even
 *       parse the function unless it is used.
 */
#define COMPILE_TIME_ASSERT(expr) if(0){switch(0){case 0:case expr:;}}

/**
 * \brief This is like COMPILE_TIME_ASSERT, but should match the new
 *        system used in "C++ 0x"
 *
 *  http://en.wikipedia.org/wiki/C%2B%2B0x#Static_assertions
 */
#define static_assert(expr, string) COMPILE_TIME_ASSERT(expr)

typedef pami_geometry_t (*pami_mapidtogeometry_fn) (int comm);

#ifdef __cplusplus
#define ENFORCE_CLASS_MEMBER(class,member)	{ PAMI_assert_debug(&((class *)this)->member || true); }
#endif // __cplusplus

/*
 * The following inlines help with initialization of shared (memory) resources,
 * such that only one participant will initialize and no participants start to
 * use the resource until initialization has finished.
 */
#ifndef __local_barriered_ctrzero_fn__
#define __local_barriered_ctrzero_fn__
#ifdef __cplusplus
///
/// This assumes that the last two counters will not be accessed
/// by any participant too soon after return from this routine...
///
template <class T_Counter>
inline void local_barriered_ctrzero(T_Counter *ctrs, size_t num,
                                size_t participants, bool master) {
        PAMI_assertf(num >= 2, "local_barriered_ctrzero() requires at least two counters\n");
        size_t c0 = num - 2;
        size_t c1 = num - 1;
        if (master) {
                size_t value = ctrs[c1].fetch() + participants;
                size_t i;
                for (i = 0; i < c0; ++i) {
                        ctrs[i].fetch_and_clear();
                }
                ctrs[c1].fetch_and_inc();
                while (ctrs[c1].fetch() != value) {
                        ctrs[c0].fetch_and_inc();
                }
                ctrs[c1].fetch_and_clear();
                ctrs[c0].fetch_and_clear();
        } else {
                size_t value = ctrs[c0].fetch();
                while (ctrs[c0].fetch() == value);
                ctrs[c1].fetch_and_inc();
                while (ctrs[c0].fetch() != 0);
        }
}
#endif // __cplusplus
// try to do an un-templated version for C programs?
#endif // __local_barriered_ctrzero_fn__

#ifndef __local_barriered_shmemzero_fn__
#define __local_barriered_shmemzero_fn__
#include <string.h>
///
/// This assumes that the area at the end of "mem" will not be accessed
/// by any participant too soon after return from this routine...
///
/// This also requires that compiler builtin atomics exist, and
/// are named (or aliased) the same as GCC __sync_*() atomics.
/// It also assumes these atomics work on "size_t" values.
/// It additionally assumes that counters may be fetched without special
/// semantics (i.e. just a load from 'volatile *').
///
inline void local_barriered_shmemzero(void *shmem, size_t len,
                                size_t participants, bool master) {
        volatile size_t *ctrs = (volatile size_t *)shmem;
        size_t num = len / sizeof(size_t);
        PAMI_assertf(num >= 2, "local_barriered_shmemzero() requires enough shmem for at least two counters\n");
        size_t c0 = num - 2;
        size_t c1 = num - 1;
        if (master) {
                size_t value = ctrs[c1] + participants;
                size_t blk1 = (char *)&ctrs[c0] - (char *)shmem;
                size_t blk2 = len - ((char *)&ctrs[c1] - (char *)shmem);
                memset(shmem, 0, blk1);
                __sync_fetch_and_add(&ctrs[c1], 1);
                while (ctrs[c1] != value) {
                        __sync_fetch_and_add(&ctrs[c0], 1);
                        mem_sync();
                }
                memset((void *)&ctrs[c1], 0, blk2);
                __sync_fetch_and_and(&ctrs[c0], 0);
        } else {
                size_t value = ctrs[c0];
                while (ctrs[c0] == value);
                __sync_fetch_and_add(&ctrs[c1], 1);
                while (ctrs[c0] != 0);
        }
}
#endif // __local_barriered_shmemzero_fn__

/// Templatized iovec array, used as pointer parameters in template specialization.
template <unsigned T_Niov>
struct iov
{
  struct iovec v[T_Niov];
};



#endif // __util_common_h__
