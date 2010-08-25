/**
 * \file sys/pami_ext_percs.h
 * \brief platform-specific messaging interface
 * \todo  put this into percs subdir and rename to pami_ext.h
 */
#ifndef __pami_ext_percs_h__
#define __pami_ext_percs_h__

#include "pami.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*****************************************************************************/
  /**
   * \defgroup ext_percs PERCS specific extensions to core PAMI
   *
   * This extension defines features that are available only on PERCS and
   * InfiniBand platforms.
   * \{
   */
  /*****************************************************************************/

  /**
   * \defgroup ext_attr Extended attributes for configuration
   *
   * Explicit cast from \ref pami_attribute_name_ext_t into
   * \ref pami_attribute_name_t may be needed.
   *
   * \{
   */

  typedef enum {
    /* Attribute             usage : type : default : description   */
    PAMI_PERCS_ATTR = PAMI_EXT_ATTR,
    PAMI_CLIENT_RELIABLE,          /**< CQ : bool : true : run in reliable mode */
    PAMI_CLIENT_THREAD_SAFE,       /**< CQ : bool : true : run in thread-safe mode */
    PAMI_CLIENT_RECEIVE_INTERRUPT, /**< CQU: bool : false: enable receive interrupt */
    PAMI_CLIENT_ERROR_HANDLER,     /**< CQ : pami_error_handler_t : NULL : asynchronous error handler */
    PAMI_CLIENT_STATISTICS,        /**<  Q : pami_statistics_t : N/A : retrieve communication statistics */
    PAMI_CLIENT_TRIGGER,           /**<   U: pami_trigger_t : N/A : add or remove a trigger */
  } pami_attribute_name_ext_t;

  /** \} */ /* end of "ext_attr" group */

  /**
   * \defgroup error_handler Asynchronous error handler
   * \{
   */
  /**
   * \brief PAMI asynchronous error handler
   *
   * In general, an asynchronous error handler should terminate the process when
   * invoked. If the handler returns, the process will not function normally.
   *
   * \param [in]  context        The PAMI context
   * \param [in]  result         The result (error code)
   *
   */
  typedef void (pami_error_handler_t)( pami_context_t  context,
                                       pami_result_t   result);

  /** \} */ /* end of "error_handler" group */

  /**
   * \defgroup comm_stat Communication statistics
   * \{
   *
   * \ref PAMI_Client_query with \c PAMI_STATISTICS returns in
   * \ref pami_attribute_value_t.chararray a pointer to \ref pami_statistics_t
   * whose memory is managed by PAMI internally.
   *
   * \c counters field in \ref pami_statistics_t is a variable-length array.
   */

  #define PAMI_COUNTER_NAME_LEN  40   /**< Maximum counter name length */

  /**
   * \brief Counter for statistics
   */
  typedef struct
  {
    char               name[PAMI_COUNTER_NAME_LEN]; /**< Counter name  */
    unsigned long long value;                       /**< Counter value */
  } pami_counter_t;

  /**
   * \brief Array of counters for statistics
   */
  typedef struct
  {
    int                count;       /**< Number of counters */
    pami_counter_t     counters[1]; /**< Array of counters  */
  } pami_statistics_t;

  #define PAMI_VOID_CONTEXT  NULL   /**< PAMI context for indirect PAMI user */

  /**
   * \brief Signature of trigger functions
   */
  typedef int (*pami_trigger_func_t) (
    pami_context_t     context,
    int                argc,
    char               *argv[]);

  /**
   * \brief Triggers
   */
  typedef struct
  {
    pami_trigger_func_t trigger_func;
    char                *trigger_name;
  } pami_trigger_t;

  /** \} */ /* end of "comm_stat" group */

  /*****************************************************************************/
  /**
   * \defgroup mutex_cond Context mutex and condition
   *
   * This extension defines more functions that are similar to
   * pthread_mutex_* and pthread_cond_* functions.
   * \{
   */
  /*****************************************************************************/

  typedef void * pami_cond_t;  /**< Condition */

  /**
   * \brief Get the owner of the context lock
   *
   * \param[in]  context PAMI communication context
   * \param[out] owner   Owner of the context lock, compatible to pthread_t
   *
   * \retval PAMI_SUCCESS  The mutex owner has been retrieved.
   * \retval PAMI_INVAL    The \c context or the \c owner pointer is invalid.
   */
  pami_result_t PAMI_Mutex_getowner (pami_context_t context, unsigned long *owner);

  /*
   * The following functions are modelled after pthread_cond_* functions.
   */
  /**
   * \brief Create a condition
   *
   * \param[in]  context PAMI communication context
   * \param[out] cond    The condition created
   *
   * \retval PAMI_SUCCESS  The condition has been created.
   * \retval PAMI_INVAL    The \c context or the \c cond pointer is invalid.
   */
  pami_result_t PAMI_Cond_create (pami_context_t context, pami_cond_t *cond);

  /**
   * \brief Wait on a condition
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to wait
   *
   * \retval PAMI_SUCCESS  The condition has been met.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Cond_wait (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Wait on a condition with timeout
   * \note  The caller must have the context lock. Upon return, the caller
   *        still has the lock.
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to wait
   * \param[in]  time    The time to wait
   *
   * \retval PAMI_SUCCESS  The condition has been met.
   * \retval PAMI_EAGAIN   The wait time has expired.
   * \retval PAMI_INVAL    The \c context, the \c cond or the \c time pointer
   *                       is invalid.
   */
  pami_result_t PAMI_Cond_timedwait (pami_context_t context, pami_cond_t cond,
          struct timespec *time);

  /**
   * \brief Signal to wake up a waiter on a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to signal
   *
   * \retval PAMI_SUCCESS  The signal has been delivered.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Cond_signal (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Signal to wake up all waiters on a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to sigal
   *
   * \retval PAMI_SUCCESS  The signal has been delivered.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Cond_broadcast (pami_context_t context, pami_cond_t cond);

  /**
   * \brief Destroy a condition
   *
   * \param[in]  context PAMI communication context
   * \param[in]  cond    The condition to destroy
   *
   * \retval PAMI_SUCCESS  The condition has been destroyed.
   * \retval PAMI_INVAL    The \c context or the \c cond is invalid.
   */
  pami_result_t PAMI_Cond_destroy (pami_context_t context, pami_cond_t cond);

  /** \} */ /* end of "mutex_cond" group */
  /** \} */ /* end of "ext_percs" group */

#ifdef __cplusplus
}
#endif

#endif
