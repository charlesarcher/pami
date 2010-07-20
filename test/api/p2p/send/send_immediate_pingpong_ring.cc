/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/p2p/send/send_immediate_pingpong_ring.cc
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include <pami.h>

size_t ITERATIONS;
size_t WARMUP;
size_t START;
size_t END;

#define START_DISPATCH_ID 10

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif



typedef struct
{
  size_t id;
  volatile size_t counter;
} info_t;

typedef struct
{
  info_t ping;
  info_t pong;
} dispatch_t;

#define BUFSIZE 10240
uint8_t         _recv_buffer[BUFSIZE] __attribute__ ((__aligned__(16)));

dispatch_t      _dispatch[100];
unsigned        _dispatch_count;

pami_task_t _task;
size_t      _size;



/* --------------------------------------------------------------- */
static void dispatch_ping (pami_context_t    context,
                           void            * cookie,
                           const void      * header,
                           size_t            hdrlen,
                           const void      * data,
                           size_t            sndlen,
                           pami_endpoint_t   origin,
                           pami_recv_t     * recv)
{
  /* Copy the data "somewhere" to simulate an application environment */
  memcpy (_recv_buffer, data, sndlen);

  size_t n = (size_t) cookie;

  /* Send a "pong" response */
  pami_send_immediate_t parameters;
  parameters.dispatch        = _dispatch[n].pong.id;
  parameters.header.iov_base = &sndlen;
  parameters.header.iov_len  = sizeof(size_t);
  parameters.data.iov_base   = NULL;
  parameters.data.iov_len    = 0;
  parameters.dest            = origin;

  PAMI_Send_immediate (context, &parameters);

  /* Increment the counter */
  _dispatch[n].ping.counter++;
}

/* --------------------------------------------------------------- */
static void dispatch_pong (pami_context_t    context,
                           void            * cookie,
                           const void      * header,
                           size_t            hdrlen,
                           const void      * data,
                           size_t            sndlen,
                           pami_endpoint_t   origin,
                           pami_recv_t     * recv)
{
  /* Copy the data "somewhere" to simulate an application environment */
  memcpy (_recv_buffer, data, sndlen);

  size_t n = (size_t) cookie;

  /* Increment the counter */
  _dispatch[n].pong.counter++;
}

/**
 * \brief Perform a specific pingpong ring test
 *
 *
 * \param[in] client   The communcation client
 * \param[in] context  The communication context
 * \param[in] n        Send protocol id to test
 * \param[in] hdrlen   Number of bytes of header data to transfer
 * \param[in] sndlen   Number of bytes of source data to transfer
 *
 * \return Average number of cycles for a half ping-pong
 */
unsigned long long test (pami_client_t  client,
                         pami_context_t context,
                         size_t         n,
                         size_t         hdrlen,
                         size_t         sndlen)
{
  char metadata[BUFSIZE];
  char buffer[BUFSIZE];

  pami_send_immediate_t parameters;
  parameters.dispatch        = _dispatch[n].ping.id;
  parameters.header.iov_base = metadata;
  parameters.header.iov_len  = hdrlen;
  parameters.data.iov_base   = buffer;
  parameters.data.iov_len    = sndlen;

  PAMI_Endpoint_create (client, (_task+1)%_size, 0, &parameters.dest);

  size_t sent = 0;
  unsigned long long t0, t1;

  /* This handles the "send to self" case */
  if (_size == 1) _dispatch[n].ping.counter = ITERATIONS;

  if (_task == 0)
  {
    /* Initiate send pingpong test to the next task, then block until
     * this task receives all "pings" from a remote task */
    t0 = PAMI_Wtimebase();
    for (sent = 1; sent <= ITERATIONS; sent++)
    {
      PAMI_Send_immediate (context, &parameters);
      while (_dispatch[n].pong.counter < sent)
      {
        PAMI_Context_advance (context, 100);
      }
    }
    t1 = PAMI_Wtimebase();

    while (_dispatch[n].ping.counter < ITERATIONS)
      PAMI_Context_advance (context, 100);
  }
  else
  {
    /* Block until this task receives all "pings" from a remote task,
     * then initiate send pingpong test to the next task */
    while (_dispatch[n].ping.counter < ITERATIONS)
      PAMI_Context_advance (context, 100);

    t0 = PAMI_Wtimebase();
    for (sent = 1; sent <= ITERATIONS; sent++)
    {
      PAMI_Send_immediate (context, &parameters);
      while (_dispatch[n].pong.counter < sent)
      {
        PAMI_Context_advance (context, 100);
      }
    }
    t1 = PAMI_Wtimebase();
  }

  _dispatch[n].pong.counter = 0;
  _dispatch[n].ping.counter = 0;

  return ((t1-t0)/ITERATIONS)/2;
}

int main (int argc, char ** argv)
{
  ITERATIONS = 10;
  WARMUP     = 0;
  START      = 0;
  END        = 256;

  {
    int foo;
    for (foo=0; foo<argc; foo++)
      fprintf (stderr, "argv[%d] = \"%s\"\n", foo, argv[foo]);
  }

  extern char *optarg;

  int x;
  while ((x = getopt(argc, argv, "i:ws:e:")) != EOF)
  {
    fprintf(stderr, "x = %d ('%c')\n", x, x);
    switch (x)
    {
      case 'i':
        ITERATIONS = strtoul (optarg, NULL, 0);
        break;
      case 'w':
        WARMUP = 1;
        break;
      case 's':
        START = strtoul (optarg, NULL, 0);
        break;
      case 'e':
        END = strtoul (optarg, NULL, 0);
        break;
      default:
        fprintf (stderr, "Usage: %s [-i iterations][-s start][-e end][-w]\n", argv[0]);
        exit (1);
        break;
    }
  }


  TRACE_ERR((stderr, "Start test ...\n"));
  size_t hdrcnt = 1;
  size_t hdrlen[1024];
  hdrlen[0] = 0;

  pami_client_t client;
  char clientname[]="PAMI";
  TRACE_ERR((stderr, "... before PAMI_Client_create()\n"));
  PAMI_Client_create (clientname, &client);
  TRACE_ERR((stderr, "...  after PAMI_Client_create()\n"));
  pami_context_t context;
  TRACE_ERR((stderr, "... before PAMI_Context_createv()\n"));
  { size_t _n = 1; PAMI_Context_createv (client, NULL, 0, &context, _n); }
  TRACE_ERR((stderr, "...  after PAMI_Context_createv()\n"));


  pami_result_t result;
  pami_dispatch_callback_fn fn;
  pami_send_hint_t options={};
  _dispatch_count = 0;

  /* Register the protocols to test */

  /* --- test default dispatch, no hints --- */
  _dispatch[_dispatch_count].ping.id = START_DISPATCH_ID + _dispatch_count;
  _dispatch[_dispatch_count].ping.counter = 0;
  fn.p2p = dispatch_ping;
  result = PAMI_Dispatch_set (context,
                              _dispatch[_dispatch_count].ping.id,
                              fn,
                              (void *) _dispatch_count,
                              options);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  _dispatch[_dispatch_count].pong.id = START_DISPATCH_ID + _dispatch_count + 1;
  _dispatch[_dispatch_count].pong.counter = 0;
  fn.p2p = dispatch_pong;
  result = PAMI_Dispatch_set (context,
                              _dispatch[_dispatch_count].pong.id,
                              fn,
                              (void *) _dispatch_count,
                              options);
  if (result != PAMI_SUCCESS)
  {
    fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
    return 1;
  }

  _dispatch_count += 2;
  /* --- test default dispatch, no hints --- */


  pami_configuration_t configuration;

  configuration.name = PAMI_TASK_ID;
  result = PAMI_Configuration_query(client, &configuration);
  _task = configuration.value.intval;

  configuration.name = PAMI_NUM_TASKS;
  result = PAMI_Configuration_query(client, &configuration);
  _size = configuration.value.intval;

  configuration.name = PAMI_WTICK;
  result = PAMI_Configuration_query(client, &configuration);
  double tick = configuration.value.doubleval;

#if 0
  /* Display some test header information */
  if (_my_rank == 0)
  {
    char str[2][1024];
    int index[2];
    index[0] = 0;
    index[1] = 0;

    index[0] += sprintf (&str[0][index[0]], "#          ");
    index[1] += sprintf (&str[1][index[1]], "#    bytes ");

    fprintf (stdout, "# PAMI_Send_immediate() nearest-neighor half-pingpong blocking latency performance test\n");
    fprintf (stdout, "#\n");

    unsigned i;
    for (i=0; i<hdrcnt; i++)
    {
      if (i==0)
        fprintf (stdout, "# testcase %d : header bytes = %3zd\n", i, hdrsize[i]);
      else
        fprintf (stdout, "# testcase %d : header bytes = %3zd (argv[%d])\n", i, hdrsize[i], i);
      index[0] += sprintf (&str[0][index[0]], "[- testcase %d -] ", i);
      index[1] += sprintf (&str[1][index[1]], " cycles    usec  ");
    }

    fprintf (stdout, "#\n");
    fprintf (stdout, "%s\n", str[0]);
    fprintf (stdout, "%s\n", str[1]);
    fflush (stdout);
  }
#endif

  for (unsigned i = 0; i < 10000000;i++){};

  unsigned long long cycles;
  double usec;

  char str[10240];

  size_t sndlen;
  for (sndlen = START; sndlen < END; sndlen = sndlen*3/2+1)
  {
    int index = 0;
    index += sprintf (&str[index], "%10zd ", sndlen);

    unsigned hdr;
    for (hdr=0; hdr<hdrcnt; hdr++)
    {
      if (WARMUP)
        test (client, context, 0, hdrlen[hdr], sndlen);

      cycles = test (client, context, 0, hdrlen[hdr], sndlen);
      usec   = cycles * tick * 1000000.0;
      index += sprintf (&str[index], "%7lld %7.4f  ", cycles, usec);
      //index += sprintf (&str[index], "%7lld  ", cycles);
    }

    fprintf (stdout, "[task %02u] %s\n", _task, str);
  }

  PAMI_Client_destroy (&client);

  return 0;
}
#undef TRACE_ERR