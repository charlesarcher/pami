/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/alltoallv_int.c
 * \brief Simple alltoallv_int test on world geometry (only bytes)
 */

#define COUNT     (4096)
/*
#define OFFSET     0
#define NITERLAT   100
#define NITERBW    MIN(10, niterlat/100+1)
#define CUTOFF     1024
*/

#include "../pami_util.h"


int *sndlens = NULL;
int *sdispls = NULL;
int *rcvlens = NULL;
int *rdispls = NULL;

void initialize_sndbuf(size_t r, void *sbuf, void *rbuf, int dt)
{
  size_t k;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *isbuf = (unsigned int *)  sbuf;
    unsigned int *irbuf = (unsigned int *)  rbuf;
    for (k = 0; k < sndlens[r]; k++)
    {
      isbuf[ sdispls[r]/get_type_size(dt_array[dt]) + k ] = ((r + k));
      irbuf[ rdispls[r]/get_type_size(dt_array[dt]) + k ] = 0xffffffff;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *dsbuf = (double *)  sbuf;
    double *drbuf = (double *)  rbuf;

    for (k = 0; k < sndlens[r]; k++)
    {
      dsbuf[ sdispls[r]/get_type_size(dt_array[dt]) + k ] = ((double)((r + k))) * 1.0;
      drbuf[ sdispls[r]/get_type_size(dt_array[dt]) + k ] = 0xffffffffffffffff;
    }
  }
  else if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *fsbuf = (float *)  sbuf;
    float *frbuf = (float *)  rbuf;

    for (k = 0; k < sndlens[r]; k++)
    {
      fsbuf[ sdispls[r]/get_type_size(dt_array[dt]) + k ] = ((float)((r + k))) * 1.0;
      frbuf[ sdispls[r]/get_type_size(dt_array[dt]) + k ] = 0xffffffffffffffff;
    }
  }
  else
  {
    char *csbuf = (char *)  sbuf;
    char *crbuf = (char *)  rbuf;

    for (k = 0; k < sndlens[r]; k++)
    {
      csbuf[ sdispls[r] + k ] = ((r + k) & 0xff);
      crbuf[ sdispls[r] + k ] = 0xff;
    }
  }
}

int check_rcvbuf(size_t sz, size_t myrank, void *rbuf, int dt)
{
  size_t r, k;

  if (dt_array[dt] == PAMI_TYPE_UNSIGNED_INT)
  {
    unsigned int *irbuf = (unsigned int *)rbuf;
    for (r = 0; r < sz; r++)
      for (k = 0; k < rcvlens[r]; k++)
      {
        if (irbuf[ rdispls[r]/get_type_size(dt_array[dt]) + k ] != (unsigned int)((myrank + k)))
        {
          fprintf(stderr, "%s:Check(%u) failed rbuf[%u+%zu]:%02x instead of %02zx (rank:%zu)\n",
                  gProtocolName, sndlens[r],
                  rdispls[r], k,
                  irbuf[ rdispls[r] + k ],
                  ((myrank + k)),
                  r );
        return 1;
        }
      }
  }
  else if (dt_array[dt] == PAMI_TYPE_DOUBLE)
  {
    double *drbuf = (double *)rbuf;
    for (r = 0; r < sz; r++)
      for (k = 0; k < rcvlens[r]; k++)
      {
        if (drbuf[ rdispls[r]/get_type_size(dt_array[dt]) + k ] != (double)(((myrank*1.0) + (k*1.0))))
        {
          fprintf(stderr, "%s:Check(%u) failed rbuf[%u+%zu]:%02f instead of %02zx (rank:%zu)\n",
                  gProtocolName, sndlens[r],
                  rdispls[r], k,
                  drbuf[ rdispls[r] + k ],
                  ((r + k)),
                  r );
        return 1;
        }
      }
  }
  if (dt_array[dt] == PAMI_TYPE_FLOAT)
  {
    float *frbuf = (float *)rbuf;
    for (r = 0; r < sz; r++)
      for (k = 0; k < rcvlens[r]; k++)
      {
        if (frbuf[ rdispls[r]/get_type_size(dt_array[dt]) + k ] != (float)(((myrank*1.0) + (k*1.0))))
        {
          fprintf(stderr, "%s:Check(%u) failed rbuf[%u+%zu]:%02f instead of %02zx (rank:%zu)\n",
                  gProtocolName, sndlens[r],
                  rdispls[r], k,
                  frbuf[ rdispls[r] + k ],
                  ((r + k)),
                  r );
        return 1;
        }
      }
  }

  return 0;
}

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* alltoallv_int variables */
  size_t               alltoallv_int_num_algorithm[2];
  pami_algorithm_t    *alltoallv_int_always_works_algo = NULL;
  pami_metadata_t     *alltoallv_int_always_works_md = NULL;
  pami_algorithm_t    *alltoallv_int_must_query_algo = NULL;
  pami_metadata_t     *alltoallv_int_must_query_md = NULL;
  pami_xfer_type_t     alltoallv_int_xfer = PAMI_XFER_ALLTOALLV_INT;
  volatile unsigned    alltoallv_int_poll_flag = 0;

  int                  nalg = 0;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          alltoallv_int;

  /* Process environment variables and setup globals */
  setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);


  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &gNum_contexts, /* gNum_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc == 1)
    return 1;

  /*  Allocate buffer(s) */
  int err = 0;
  void* sbuf = NULL;
  err = posix_memalign((void*) & sbuf, 128, (gMax_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;

  void* rbuf = NULL;
  err = posix_memalign((void*) & rbuf, 128, (gMax_count * num_tasks) + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  sndlens = (int*) malloc(num_tasks * sizeof(int));
  assert(sndlens);
  sdispls = (int*) malloc(num_tasks * sizeof(int));
  assert(sdispls);
  rcvlens = (int*) malloc(num_tasks * sizeof(int));
  assert(rcvlens);
  rdispls = (int*) malloc(num_tasks * sizeof(int));
  assert(rdispls);

  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    /*  Query the world geometry for barrier algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               barrier_xfer,
                               barrier_num_algorithm,
                               &bar_always_works_algo,
                               &bar_always_works_md,
                               &bar_must_query_algo,
                               &bar_must_query_md);

    if (rc == 1)
      return 1;

    /*  Query the world geometry for alltoallv_int algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               alltoallv_int_xfer,
                               alltoallv_int_num_algorithm,
                               &alltoallv_int_always_works_algo,
                               &alltoallv_int_always_works_md,
                               &alltoallv_int_must_query_algo,
                               &alltoallv_int_must_query_md);

    if (rc == 1)
      return 1;

    barrier.cb_done   = cb_done;
    barrier.cookie    = (void*) & bar_poll_flag;
    barrier.algorithm = bar_always_works_algo[0];
    blocking_coll(context[iContext], &barrier, &bar_poll_flag);

    for (nalg = 0; nalg < alltoallv_int_num_algorithm[0]; nalg++)
    {
      alltoallv_int.cb_done    = cb_done;
      alltoallv_int.cookie     = (void*) & alltoallv_int_poll_flag;
      alltoallv_int.algorithm  = alltoallv_int_always_works_algo[nalg];
      alltoallv_int.cmd.xfer_alltoallv_int.sndbuf        = sbuf;
      alltoallv_int.cmd.xfer_alltoallv_int.stype         = PAMI_TYPE_BYTE;
      alltoallv_int.cmd.xfer_alltoallv_int.stypecounts   = sndlens;
      alltoallv_int.cmd.xfer_alltoallv_int.sdispls       = sdispls;
      alltoallv_int.cmd.xfer_alltoallv_int.rcvbuf        = rbuf;
      alltoallv_int.cmd.xfer_alltoallv_int.rtype         = PAMI_TYPE_BYTE;
      alltoallv_int.cmd.xfer_alltoallv_int.rtypecounts   = rcvlens;
      alltoallv_int.cmd.xfer_alltoallv_int.rdispls       = rdispls;

      gProtocolName = alltoallv_int_always_works_md[nalg].name;

      if (task_id == 0)
      {
        printf("# alltoallv_int Bandwidth Test(size:%zu) -- context = %d, protocol: %s\n",
               num_tasks, iContext, gProtocolName);
        printf("# Size(bytes)           cycles    bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(alltoallv_int_always_works_md[nalg].name, gSelected) == NULL) && gSelector) ||
          ((strstr(alltoallv_int_always_works_md[nalg].name, gSelected) != NULL) && !gSelector))  continue;

      int i, j;
      int dt,op=4/*SUM*/;

      for (dt = 0; dt < dt_count; dt++)
      {
          if (gValidTable[op][dt])
          {
            if (task_id == 0)
              printf("Running Alltoallv: %s\n", dt_array_str[dt]);

            for (i = 1; i <= gMax_count/get_type_size(dt_array[dt]); i *= 2)
            {
              size_t  dataSent = i;
              int          niter;

              if (dataSent < CUTOFF)
                niter = gNiterlat;
              else
                niter = NITERBW;

              for (j = 0; j < num_tasks; j++)
              {
                sndlens[j] = rcvlens[j] = i;
                sdispls[j] = rdispls[j] = i * j * get_type_size(dt_array[dt]);

                initialize_sndbuf( j, sbuf, rbuf, dt );

              }
              alltoallv_int.cmd.xfer_alltoallv_int.rtype = dt_array[dt];
              alltoallv_int.cmd.xfer_alltoallv_int.stype = dt_array[dt];

              blocking_coll(context[iContext], &barrier, &bar_poll_flag);

              /* Warmup */
              blocking_coll(context[iContext], &alltoallv_int, &alltoallv_int_poll_flag);

              blocking_coll(context[iContext], &barrier, &bar_poll_flag);

              ti = timer();

              for (j = 0; j < niter; j++)
              {
                blocking_coll(context[iContext], &alltoallv_int, &alltoallv_int_poll_flag);
              }

              tf = timer();
              blocking_coll(context[iContext], &barrier, &bar_poll_flag);

              int rc_check;
              rc |= rc_check = check_rcvbuf(num_tasks, task_id, rbuf, dt);

              if (rc_check) fprintf(stderr, "%s FAILED validation\n", gProtocolName);

              usec = (tf - ti) / (double)niter;

              if (task_id == 0)
              {
                 printf("  %11lld %16d %14.1f %12.2f\n",
                 (long long)dataSent,
                 niter,
                 (double)1e6*(double)dataSent / (double)usec,
                 usec);
                 fflush(stdout);
              }
            }
          }
      }
    }
    free(bar_always_works_algo);
    free(bar_always_works_md);
    free(bar_must_query_algo);
    free(bar_must_query_md);
    free(alltoallv_int_always_works_algo);
    free(alltoallv_int_always_works_md);
    free(alltoallv_int_must_query_algo);
    free(alltoallv_int_must_query_md);
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  sbuf = (char*)sbuf - gBuffer_offset;
  free(sbuf);

  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  free(sndlens);
  free(sdispls);
  free(rcvlens);
  free(rdispls);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}