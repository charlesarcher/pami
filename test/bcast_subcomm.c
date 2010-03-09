/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/bcast_subcomm.c
 * \brief ???
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

#include "sys/xmi.h"

#define BUFSIZE 1048576

// Geometry Objects
volatile unsigned        _g_barrier_active;
volatile unsigned        _g_broadcast_active;

static double timer()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}
void cb_geom_init (void *context, void * clientdata, xmi_result_t res)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_barrier (void *context, void * clientdata, xmi_result_t res)
{
  int * active = (int *) clientdata;
  (*active)--;
}

void cb_broadcast (void *context, void * clientdata, xmi_result_t res)
{
    int * active = (int *) clientdata;
    (*active)--;
}

void _barrier (xmi_context_t  context,
               xmi_xfer_t *barrier)
{
  _g_barrier_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)barrier);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to issue barrier collective. result = %d\n", result);
        exit(1);
      }
  while (_g_barrier_active)
    result = XMI_Context_advance (context, 1);
}

void _broadcast (xmi_context_t    context,
                 xmi_xfer_t *broadcast)
{
  _g_broadcast_active++;
  xmi_result_t result;
  result = XMI_Collective(context, (xmi_xfer_t*)broadcast);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to issue broadcast collective. result = %d\n", result);
        exit(1);
      }
  while (_g_broadcast_active)
    result = XMI_Context_advance (context, 1);
}



int main(int argc, char*argv[])
{
  xmi_client_t  client;
  xmi_context_t context;
  xmi_result_t  result = XMI_ERROR;
  double        tf,ti,usec;
  char          buf[BUFSIZE];
  char          cl_string[] = "TEST";
  result = XMI_Client_initialize (cl_string, &client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", result);
        return 1;
      }

	{ size_t _n = 1; result = XMI_Context_createv(client, NULL, 0, &context, _n); }
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", result);
        return 1;
      }


  xmi_configuration_t configuration;
  configuration.name = XMI_TASK_ID;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t rank = configuration.value.intval;

  configuration.name = XMI_NUM_TASKS;
  result = XMI_Configuration_query(client, &configuration);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, result);
        return 1;
      }
  size_t          sz    = configuration.value.intval;
  size_t          half  = sz/2;
  size_t          set[2];
  xmi_geometry_t  world_geometry;
  result = XMI_Geometry_world (client, &world_geometry);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to get world geometry. result = %d\n", result);
        return 1;
      }

  xmi_algorithm_t *world_algorithm=NULL;
  int num_algorithm[2] = {0, 0};
  result = XMI_Geometry_algorithms_num(context,
                                       world_geometry,
                                       XMI_XFER_BARRIER,
                                       num_algorithm);
  if (result != XMI_SUCCESS)
  {
    fprintf (stderr,
             "Error. Unable to query barrier algorithm. result = %d\n",
             result);
    return 1;
  }

  assert(num_algorithm[0] != 0);
  if (num_algorithm[0])
  {
    world_algorithm = (xmi_algorithm_t*)
      malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
    result = XMI_Geometry_algorithms_info(context,
                                          world_geometry,
                                          XMI_XFER_BARRIER,
                                          world_algorithm,
                                          (xmi_metadata_t*)NULL,
                                          num_algorithm[0],
                                          NULL,
                                          NULL,
                                          0);

  }

  xmi_xfer_t world_barrier;
  world_barrier.cb_done   = cb_barrier;
  world_barrier.cookie    = (void*)&_g_barrier_active;
  world_barrier.algorithm = world_algorithm[0];
  _barrier(context, &world_barrier);

  xmi_geometry_t           bottom_geometry;
  xmi_geometry_range_t     bottom_range;
  xmi_algorithm_t      *bottom_bar_algorithm=NULL;
  xmi_algorithm_t      *bottom_bcast_algorithm=NULL;
  xmi_xfer_t            bottom_barrier;
  xmi_xfer_t            bottom_broadcast;

  xmi_geometry_t           top_geometry;
  xmi_geometry_range_t     top_range;
  xmi_algorithm_t      *top_bar_algorithm=NULL;
  xmi_algorithm_t      *top_bcast_algorithm=NULL;
  xmi_xfer_t            top_barrier;
  xmi_xfer_t            top_broadcast;
  int                   geom_init = 1;


  if(rank>=0 && rank<=half-1)
      {
        fprintf(stderr, "%d:  Creating Bottom Geometry\n", (int)rank);
        bottom_range.lo =0;
        bottom_range.hi =half-1;
        result = XMI_Geometry_create_taskrange (client,
                                          &bottom_geometry,
                                                world_geometry,
                                          1,
                                          &bottom_range,
                                                1,
                                                context,
                                                cb_geom_init,
                                                &geom_init);
        while (geom_init == 1)
          result = XMI_Context_advance (context, 1);



        result = XMI_Geometry_algorithms_num(context,
                                             bottom_geometry,
                                             XMI_XFER_BARRIER,
                                             num_algorithm);
        if (result != XMI_SUCCESS)
        {
          fprintf (stderr,
                   "Error. Unable to query barrier algorithm. result = %d\n",
                   result);
          return 1;
        }
        assert(num_algorithm[0] != 0);
        if (num_algorithm[0])
        {
          bottom_bar_algorithm = (xmi_algorithm_t*)
            malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
          result = XMI_Geometry_algorithms_info(context,
                                                bottom_geometry,
                                                XMI_XFER_BARRIER,
                                                bottom_bar_algorithm,
                                                (xmi_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);
        }
        bottom_barrier.cb_done   = cb_barrier;
        bottom_barrier.cookie    = (void*)&_g_barrier_active;
        bottom_barrier.algorithm = bottom_bar_algorithm[0];

        result = XMI_Geometry_algorithms_num(context,
                                             bottom_geometry,
                                             XMI_XFER_BROADCAST,
                                             num_algorithm);
        assert(num_algorithm[0] != 0);
        if (num_algorithm[0])
        {
          bottom_bcast_algorithm = (xmi_algorithm_t*)
            malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);

          result = XMI_Geometry_algorithms_info(context,
                                                bottom_geometry,
                                                XMI_XFER_BROADCAST,
                                                bottom_bcast_algorithm,
                                                (xmi_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }
        bottom_broadcast.cb_done   = cb_broadcast;
        bottom_broadcast.cookie    = (void*)&_g_broadcast_active;
        bottom_broadcast.algorithm = bottom_bcast_algorithm[0];
        bottom_broadcast.cmd.xfer_broadcast.root      = 0;
        bottom_broadcast.cmd.xfer_broadcast.buf       = NULL;
        bottom_broadcast.cmd.xfer_broadcast.type      = XMI_BYTE;
        bottom_broadcast.cmd.xfer_broadcast.typecount = 0;


        set[0]=1;
        set[1]=0;
      }
  else
      {
        fprintf(stderr, "%d:  Creating Top Geometry\n", (int)rank);
        top_range.lo =half;
        top_range.hi =sz-1;
        result = XMI_Geometry_create_taskrange (client,
                                          &top_geometry,
                                                world_geometry,
                                          2,
                                          &top_range,
                                                1,
                                                context,
                                                cb_geom_init,
                                                &geom_init);

        while (geom_init == 1)
          result = XMI_Context_advance (context, 1);

        result = XMI_Geometry_algorithms_num(context,
                                             top_geometry,
                                             XMI_XFER_BARRIER,
                                             num_algorithm);
        if (result != XMI_SUCCESS)
        {
          fprintf (stderr,
                   "Error. Unable to query barrier algorithm. result = %d\n",
                   result);
          return 1;
        }

        assert(num_algorithm[0] != 0);
        if (num_algorithm[0])
        {
          top_bar_algorithm = (xmi_algorithm_t*)
            malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
          result = XMI_Geometry_algorithms_info(context,
                                                top_geometry,
                                                XMI_XFER_BARRIER,
                                                top_bar_algorithm,
                                                (xmi_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }

        top_barrier.cb_done   = cb_barrier;
        top_barrier.cookie    = (void*)&_g_barrier_active;
        top_barrier.algorithm = top_bar_algorithm[0];

        result = XMI_Geometry_algorithms_num(context,
                                             top_geometry,
                                             XMI_XFER_BROADCAST,
                                             num_algorithm);
        assert(num_algorithm[0] != 0);
        if (num_algorithm[0])
        {
          top_bcast_algorithm = (xmi_algorithm_t*)
            malloc(sizeof(xmi_algorithm_t) * num_algorithm[0]);
          result = XMI_Geometry_algorithms_info(context,
                                                top_geometry,
                                                XMI_XFER_BROADCAST,
                                                top_bcast_algorithm,
                                                (xmi_metadata_t*)NULL,
                                                num_algorithm[0],
                                                NULL,
                                                NULL,
                                                0);

        }

        top_broadcast.cb_done   = cb_broadcast;
        top_broadcast.cookie    = (void*)&_g_broadcast_active;
        top_broadcast.algorithm = top_bcast_algorithm[0];
        top_broadcast.cmd.xfer_broadcast.root      = 0;
        top_broadcast.cmd.xfer_broadcast.buf       = NULL;
        top_broadcast.cmd.xfer_broadcast.type      = XMI_BYTE;
        top_broadcast.cmd.xfer_broadcast.typecount = 0;


        set[0]=0;
        set[1]=1;
      }


  xmi_xfer_t   *barriers   [] = {&bottom_barrier, &top_barrier};
  xmi_xfer_t   *broadcasts [] = {&bottom_broadcast, &top_broadcast};
  size_t           roots[]        = {0, half};
  int             i,j,k;
  for(k=1; k>=0; k--)
      {
        if (rank == roots[k])
            {
              printf("# Broadcast Bandwidth Test -- root = %d\n", (int)roots[k]);
              printf("# Size(bytes)           cycles    bytes/sec    usec\n");
              printf("# -----------      -----------    -----------    ---------\n");
            }
        if(set[k])
            {
              printf("Participant:  %d\n", (int)rank);
              fflush(stdout);
              _barrier (context, barriers[k]);
              for(i=1; i<=BUFSIZE; i*=2)
                  {
                    long long dataSent = i;
                    int          niter = 100;
                    _barrier (context, barriers[k]);
                    ti = timer();
                    for (j=0; j<niter; j++)
                        {
                          broadcasts[k]->cmd.xfer_broadcast.root      = roots[k];
                          broadcasts[k]->cmd.xfer_broadcast.buf       = buf;
                          broadcasts[k]->cmd.xfer_broadcast.typecount = i;
                          _broadcast(context, broadcasts[k]);
                        }
                    tf = timer();
                    _barrier (context, barriers[k]);
                    usec = (tf - ti)/(double)niter;
                    if (rank == roots[k])
                        {
                          printf("  %11lld %16lld %14.1f %12.2f\n",
                                 dataSent,
                                 0LL,
                                 (double)1e6*(double)dataSent/(double)usec,
                                 usec);
                          fflush(stdout);
                        }
                  }
            }
        _barrier (context, &world_barrier);
      }
  _barrier (context, &world_barrier);

  result = XMI_Context_destroy (context);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to destroy xmi context. result = %d\n", result);
        return 1;
      }

  result = XMI_Client_finalize (client);
  if (result != XMI_SUCCESS)
      {
        fprintf (stderr, "Error. Unable to finalize xmi client. result = %d\n", result);
        return 1;
      }
  free(world_algorithm);
  //  free(top_algorithm);
  //  free(bottom_algorithm);
  return 0;
}
