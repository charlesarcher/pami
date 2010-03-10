///
/// \file test/context/post-multithreaded-perf.c
/// \brief Multithreaded XMI_Context_post() performance test
///
/// \todo There is a slight difference in the reported post times in the case
///       where the number of 'helper' threads == 1. For this test to be
///       completely accurate the times should be nearly equal.
///

#include "sys/xmi.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

//#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

#define MAXTHREADS 64
#define ITERATIONS 10000
//#define ITERATIONS 10

xmi_context_t   _context[MAXTHREADS];
volatile size_t _value[MAXTHREADS];
xmi_work_t      _work[ITERATIONS*MAXTHREADS];

volatile size_t _recv;
volatile size_t _thread_state[MAXTHREADS];
volatile size_t _main_state;

xmi_result_t do_work (xmi_context_t   context,
                      void          * cookie)
{
  TRACE((stderr, ">> do_work (%0x08x, %p)\n", (unsigned)context, cookie));

  size_t * value = (size_t *) cookie;
  TRACE((stderr, "   do_work (), value = %p, *value = %zu -> %zu\n", value, *value, *value - 1));
  (*value)--;

  TRACE((stderr, "<< do_work ()\n"));
  return XMI_SUCCESS;
}

void * thread_main (void * arg)
{
  size_t id = (size_t) arg;
  TRACE((stderr, ">> thread_main (%zu)\n", id));

  xmi_context_t context = _context[id];

  TRACE((stderr, "   thread_main (%zu) .. 0\n", id));

  /* Lock this context */
  xmi_result_t result = XMI_Context_lock (context);

  TRACE((stderr, "   thread_main (%zu) .. 1\n", id));

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to lock the xmi context. result = %d\n", result);
      exit(1);
    }

  /* signal thread is ready */
  _thread_state[id] = 1;

  /* wait until main thread is ready */
  while (_main_state == 0);

  TRACE((stderr, "   thread_main (%zu) .. 2, _value[id] = %zu\n", id, _value[id]));

  while (_value[id] > 0)
    {
      TRACE((stderr, "   thread_main (%zu) .. 3\n", id));
      result = XMI_Context_advance (context, 100);
#ifdef TRACE

      if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to advance the xmi context. result = %d\n", result);
          exit(1);
        }

#endif
    }

  TRACE((stderr, "   thread_main (%zu) .. 4\n", id));

  /* Unlock this context */
  result = XMI_Context_unlock (context);

  /* *********************************************************************** */
  /* post work to context 0                                                  */
  /* *********************************************************************** */

  /* signal thread is ready */
  _thread_state[id] = 0;

  /* wait until main thread is ready */
  while (_main_state == 1);

  TRACE((stderr, "   thread_main (%zu) .. post work to context 0, _recv = %zu ###############################\n", id, _recv));
  size_t i;

  for (i = 0; i < ITERATIONS; i++)
    {
      TRACE((stderr, "   thread_main (%zu), i = %zu, work index = %zu\n", id, i, id*ITERATIONS + i));
      result = XMI_Context_post (_context[0], &_work[id*ITERATIONS+i], do_work, (void *) & _recv);
#ifdef TRACE

      if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "   thread_main (%zu): Error. Unable to post work to context[0]. result = %d\n", id, result);
          exit(1);
        }

#endif
    }


  TRACE((stderr, "   thread_main (%zu) .. 5\n", id));

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to unlock the xmi context. result = %d\n", result);
      exit(1);
    }

  TRACE((stderr, "<< thread_main (%zu)\n", id));
  return NULL;
}


int main (int argc, char ** argv)
{
  xmi_client_t client;
  char         cl_string[] = "TEST";
  xmi_result_t result = XMI_ERROR;
  size_t i;
  long long int max_threads = 0;

  TRACE((stderr, ">> main (), max_threads = %lld\n", max_threads));

  if (argc > 1)
    {
      max_threads = strtoll (argv[1], NULL, 10);
    }


  result = XMI_Client_initialize (cl_string, &client);

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
      return 1;
    }

  xmi_configuration_t configuration;

  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  xmi_task_t task = configuration.value.intval;

  if (task == 0)
    {
      fprintf (stdout, "XMI_Context_post() multi-threaded performance test\n");
    }

  if (max_threads == 0 || max_threads > MAXTHREADS)
    {
      if (task == 0)
        {
          fprintf (stdout, "\n");
          fprintf (stdout, "  Error: Number of peer threads must be [1..%zu], got %lld\n", MAXTHREADS, max_threads);
          fprintf (stdout, "\n");
          fprintf (stdout, "  Usage: %s threads\n", argv[0]);
          fprintf (stdout, "\n");
        }

      result = XMI_Client_finalize (client);

      if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
          return 1;
        }

      TRACE((stderr, "<< main ()\n"));

      return 1;
    }

  /* Initialize the contexts */
  result = XMI_Context_createv (client, NULL, 0, &_context[0], max_threads);

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to create first xmi context. result = %d\n", result);
      return 1;
    }

  if (task == 0)
    {
      /* Initialize the post counters */
      _main_state = 0;
      _recv = 0;

      for (i = 0; i < max_threads; i++)
        {
          _value[i] = ITERATIONS;
          _thread_state[i] = 0;
        }

      /* Create the "helper" or "endpoint" threads */
      pthread_t thread[max_threads];
      int rc = 0;
      size_t t, num_threads = 0;

      for (i = 0; i < max_threads && rc == 0; i++)
        {
          rc = pthread_create (&thread[i], NULL, thread_main, (void *)(i));

          if (rc == 0)
            {
              num_threads++;
              _recv += ITERATIONS;
            }
        }

      if (num_threads == 0)
        {
          fprintf (stderr, "Error. Unable to create any threads.\n");
        }
      else
        {
          fprintf (stdout, "\n");
          fprintf (stdout, "  Number of 'sender' threads:          %8zu\n", 1);
          fprintf (stdout, "  Number of 'receiver' threads:        %8zu\n", num_threads);
          fprintf (stdout, "  Number of posts to each thread:      %8zu\n", ITERATIONS);

          /* wait a bit to give threads time to start */
          usleep (1000);

          /* wait until all threads are ready */
          for (t = 0; t < num_threads; t++)
            {
              while (_thread_state[t] == 0);
            }

          /* signal main thread is ready */
          _main_state = 1;

          unsigned long long t0 = XMI_Wtimebase();

          /* post all of the work */
          for (i = 0; i < ITERATIONS; i++)
            {
              for (t = 0; t < num_threads; t++)
                {
                  TRACE((stderr, "   main (), i = %zu, t = %zu, work index = %zu\n", i, t, t*ITERATIONS + i));
                  result = XMI_Context_post (_context[t], &_work[t*ITERATIONS+i], do_work, (void *) & _value[t]);
#ifdef TRACE

                  if (result != XMI_SUCCESS)
                    {
                      fprintf (stderr, "Error. Unable to post work to context[%zu]. result = %d\n", t, result);
                      return 1;
                    }

#endif
                }
            }

          /* wait until all of the work is done */
          for (t = 0; t < num_threads; t++)
            {
              while (_value[t] > 0);
            }

          unsigned long long t1 = XMI_Wtimebase();
          unsigned long long cycles = ((t1 - t0) / ITERATIONS) / num_threads;
          fprintf (stdout, "  Average number of cycles to post:    %8lld\n", cycles);

          fprintf (stdout, "\n");
          fprintf (stdout, "  Number of 'sender' threads:          %8zu\n", num_threads);
          fprintf (stdout, "  Number of 'receiver' threads:        %8zu\n", 1);
          fprintf (stdout, "  Number of posts from all threads:    %8zu\n", ITERATIONS*num_threads);


          /* Lock this context */
          result = XMI_Context_lock (_context[0]);

          /* wait until all threads are ready */
          for (t = 0; t < num_threads; t++)
            {
              while (_thread_state[t] == 1);
            }

          /* signal main thread is ready */
          _main_state = 0;


          /* wait until all of the work is done */
          TRACE((stderr, "   main (), wait for all work to be received, _recv = %zu\n", _recv));
          t0 = XMI_Wtimebase();

          while (_recv > 0)
            {
              TRACE((stderr, "   main () .. recv advance loop, _recv = %zu\n", _recv));
              result = XMI_Context_advance (_context[0], 100);
#ifdef TRACE

              if (result != XMI_SUCCESS)
                {
                  fprintf (stderr, "Error. Unable to advance the xmi context. result = %d\n", result);
                  exit(1);
                }

#endif
            }

          t1 = XMI_Wtimebase();

          /* Unlock this context */
          result = XMI_Context_unlock (_context[0]);

          cycles = ((t1 - t0) / ITERATIONS) / num_threads;
          fprintf (stdout, "  Average number of cycles to receive: %8lld\n", cycles);
          fprintf (stdout, "\n");

        }
    }


  for (i = 0; i < MAXTHREADS; i++)
    {
      result = XMI_Context_destroy (_context[i]);

      if (result != XMI_SUCCESS)
        {
          fprintf (stderr, "Error. Unable to destroy context %zu. result = %d\n", i, result);
          return 1;
        }
    }

  result = XMI_Client_finalize (client);

  if (result != XMI_SUCCESS)
    {
      fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
      return 1;
    }

  TRACE((stderr, "<< main ()\n"));

  return 0;
};
