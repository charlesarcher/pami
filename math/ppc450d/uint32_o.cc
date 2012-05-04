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
 * \file math/ppc450d/uint32_o.cc
 * \brief Optimized math routines for unsigned 32 bit integer operations on
 *        the ppc 450 dual fpu architecture.
 */
#include "math_coremath.h"
#include "Util.h"
#include "ppc450d/internal_o.h"

void _pami_core_uint32_band2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
        const uint32_t *s0 = srcs[0];
        const uint32_t *s1 = srcs[1];
        int num;
        int remainder = count;
        if (count <= 2048) {
                num = count >> 2;
                remainder = count - (num << 2);
                register int r0=0;
                register int r1=0;
                register int r2=0;
                register int r3=0;
                register int r4=0;
                register int r5=0;
                register int r6=0;
                register int r7=0;

                while (num--) {
                        asm volatile(
                                "lwz   %[r0],0(%[s0]);"
                                "lwz   %[r4],0(%[s1]);"
                                "lwz   %[r1],4(%[s0]);"
                                "lwz   %[r5],4(%[s1]);"

                                "and   %[r0],%[r0],%[r4];"
                                "stw   %[r0],0(%[dp]);"

                                "lwz   %[r2],8(%[s0]);"
                                "lwz   %[r6],8(%[s1]);"
                                "and   %[r1],%[r1],%[r5];"
                                "stw   %[r1],4(%[dp]);"

                                "lwz   %[r3],12(%[s0]);"
                                "lwz   %[r7],12(%[s1]);"
                                "and   %[r2],%[r2],%[r6];"
                                "stw   %[r2],8(%[dp]);"

                                "and   %[r3],%[r3],%[r7];"
                                "stw   %[r3],12(%[dp]);"

                                "addi  %[s0],%[s0],16;"
                                "addi  %[s1],%[s1],16;"
                                "addi  %[dp],%[dp],16;"

                                : [s0] "+b" (s0),
                                  [s1] "+b" (s1),
                                  [dp] "+b" (dst),
                                  [r0] "+r" (r0),
                                  [r1] "+r" (r1),
                                  [r2] "+r" (r2),
                                  [r3] "+r" (r3),
                                  [r4] "+r" (r4),
                                  [r5] "+r" (r5),
                                  [r6] "+r" (r6),
                                  [r7] "+r" (r7)
                                :
                                : "memory");
                }
                for (num = 0; num < remainder; ++num) {
                        dst[num] = s0[num] & s1[num];
                }
                return;
        }

        uint32_t *s2 = dst;
        num = (count - 16) >> 3;
        remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("and %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
        for (num = 0; num < remainder; ++num) {
                s2[num] = s0[num] & s1[num];
        }

        //fprintf (stderr, "<< Core_uint32_band()\n");
        return;
}

void _pami_core_uint32_bor2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
        const uint32_t *s0 = srcs[0];
        const uint32_t *s1 = srcs[1];
        int num;
        int remainder = count;
        if (count <= 2048) {
                num = count >> 2;
                remainder = count - (num << 2);
                register int r0=0;
                register int r1=0;
                register int r2=0;
                register int r3=0;
                register int r4=0;
                register int r5=0;
                register int r6=0;
                register int r7=0;

                while (num--) {
                        asm volatile(
                                "lwz   %[r0],0(%[s0]);"
                                "lwz   %[r4],0(%[s1]);"
                                "lwz   %[r1],4(%[s0]);"
                                "lwz   %[r5],4(%[s1]);"

                                "or    %[r0],%[r0],%[r4];"
                                "stw   %[r0],0(%[dp]);"

                                "lwz   %[r2],8(%[s0]);"
                                "lwz   %[r6],8(%[s1]);"
                                "or    %[r1],%[r1],%[r5];"
                                "stw   %[r1],4(%[dp]);"

                                "lwz   %[r3],12(%[s0]);"
                                "lwz   %[r7],12(%[s1]);"
                                "or    %[r2],%[r2],%[r6];"
                                "stw   %[r2],8(%[dp]);"

                                "or    %[r3],%[r3],%[r7];"
                                "stw   %[r3],12(%[dp]);"

                                "addi  %[s0],%[s0],16;"
                                "addi  %[s1],%[s1],16;"
                                "addi  %[dp],%[dp],16;"

                                : [s0] "+b" (s0),
                                  [s1] "+b" (s1),
                                  [dp] "+b" (dst),
                                  [r0] "+r" (r0),
                                  [r1] "+r" (r1),
                                  [r2] "+r" (r2),
                                  [r3] "+r" (r3),
                                  [r4] "+r" (r4),
                                  [r5] "+r" (r5),
                                  [r6] "+r" (r6),
                                  [r7] "+r" (r7)
                                :
                                : "memory");
                }
                for (num = 0; num < remainder; ++num) {
                        dst[num] = s0[num] | s1[num];
                }
                return;
        }

        uint32_t *s2 = dst;
        num = (count - 16) >> 3;
        remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("or %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
        for (num = 0; num < remainder; ++num) {
                s2[num] = s0[num] | s1[num];
        }

        //fprintf (stderr, "<< Core_uint32_bor()\n");
        return;
}

void _pami_core_uint32_bxor2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
        const uint32_t *s0 = srcs[0];
        const uint32_t *s1 = srcs[1];
        int num;
        int remainder = count;
        if (count <= 2048) {
                num = count >> 2;
                remainder = count - (num << 2);
                register int r0=0;
                register int r1=0;
                register int r2=0;
                register int r3=0;
                register int r4=0;
                register int r5=0;
                register int r6=0;
                register int r7=0;

                while (num--) {
                        asm volatile(
                                "lwz   %[r0],0(%[s0]);"
                                "lwz   %[r4],0(%[s1]);"
                                "lwz   %[r1],4(%[s0]);"
                                "lwz   %[r5],4(%[s1]);"

                                "xor   %[r0],%[r0],%[r4];"
                                "stw   %[r0],0(%[dp]);"

                                "lwz   %[r2],8(%[s0]);"
                                "lwz   %[r6],8(%[s1]);"
                                "xor   %[r1],%[r1],%[r5];"
                                "stw   %[r1],4(%[dp]);"

                                "lwz   %[r3],12(%[s0]);"
                                "lwz   %[r7],12(%[s1]);"
                                "xor   %[r2],%[r2],%[r6];"
                                "stw   %[r2],8(%[dp]);"

                                "xor   %[r3],%[r3],%[r7];"
                                "stw   %[r3],12(%[dp]);"

                                "addi  %[s0],%[s0],16;"
                                "addi  %[s1],%[s1],16;"
                                "addi  %[dp],%[dp],16;"

                                : [s0] "+b" (s0),
                                  [s1] "+b" (s1),
                                  [dp] "+b" (dst),
                                  [r0] "+r" (r0),
                                  [r1] "+r" (r1),
                                  [r2] "+r" (r2),
                                  [r3] "+r" (r3),
                                  [r4] "+r" (r4),
                                  [r5] "+r" (r5),
                                  [r6] "+r" (r6),
                                  [r7] "+r" (r7)
                                :
                                : "memory");
                }
                for (num = 0; num < remainder; ++num) {
                        dst[num] = s0[num] ^ s1[num];
                }
                return;
        }

        uint32_t *s2 = dst;
        num = (count - 16) >> 3;
        remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("xor %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
        for (num = 0; num < remainder; ++num) {
                s2[num] = s0[num] ^ s1[num];
        }

        //fprintf (stderr, "<< Core_uint32_bxor()\n");
        return;
}

void _pami_core_uint32_land2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const uint32_t *s0 = (const uint32_t *)srcs[0];
  const uint32_t *s1 = (const uint32_t *)srcs[1];

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
        "lwz     5,0(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,0(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   24,2,6;"
        "lwz     5,4(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,4(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   25,2,6;"
        "lwz     5,8(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,8(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   26,2,6;"
        "lwz     5,12(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,12(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   27,2,6;"
        "lwz     5,16(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,16(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   28,2,6;"
        "lwz     5,20(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,20(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   29,2,6;"
        "lwz     5,24(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,24(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   30,2,6;"
        "lwz     5,28(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,28(%[s1]);"

        "cmpwi   1,6,0;"

        "crnor   31,2,6;"


        "mfcr    5;"


        "rlwinm  6,5,25,31,31;"

        "stw     6,0(%[dp]);"
        "rlwinm  7,5,26,31,31;"

        "stw     7,4(%[dp]);"
        "rlwinm  6,5,27,31,31;"

        "stw     6,8(%[dp]);"
        "rlwinm  7,5,28,31,31;"

        "stw     7,12(%[dp]);"
        "rlwinm  6,5,29,31,31;"

        "stw     6,16(%[dp]);"
        "rlwinm  7,5,30,31,31;"

        "stw     7,20(%[dp]);"
        "rlwinm  6,5,31,31,31;"

        "stw     6,24(%[dp]);"
        "andi.   7,5,0x01;"

        "stw     7,28(%[dp]);"




      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory", "5", "6", "7"
    );

    s0 += 8;
    s1 += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    *dp = (*s0) && (*s1);
    dp++;
    s0++;
    s1++;
  }

  return;
}

void _pami_core_uint32_lor2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {

  const uint32_t *s0 = (const uint32_t *)srcs[0];
  const uint32_t *s1 = (const uint32_t *)srcs[1];
  uint32_t *dp = (uint32_t *)dst;

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
        "lwz    6,0(%[s0]);"

        "lwz    7,0(%[s1]);"

        "lwz    8,4(%[s0]);"
        "or.    5,6,7;"

        "crnot  24,2;"
        "lwz    9,4(%[s1]);"

        "or.    5,8,9;"
        "lwz    6,8(%[s0]);"

        "crnot  25,2;"
        "lwz    7,8(%[s1]);"

        "or.    5,6,7;"
        "lwz    8,12(%[s0]);"

        "crnot  26,2;"
        "lwz    9,12(%[s1]);"

        "or.    5,8,9;"
        "lwz    6,16(%[s0]);"

        "crnot  27,2;"
        "lwz    7,16(%[s1]);"

        "or.    5,6,7;"
        "lwz    8,20(%[s0]);"

        "crnot  28,2;"
        "lwz    9,20(%[s1]);"

        "or.    5,8,9;"
        "lwz    6,24(%[s0]);"

        "crnot  29,2;"
        "lwz    7,24(%[s1]);"

        "or.    5,6,7;"
        "lwz    8,28(%[s0]);"

        "crnot  30,2;"
        "lwz    9,28(%[s1]);"

        "or.    5,8,9;"

        "crnot  31,2;"


        "mfcr   5;"

        "rlwinm 6,5,25,31,31;"

        "stw    6,0(%[dp]);"
        "rlwinm 7,5,26,31,31;"

        "stw    7,4(%[dp]);"
        "rlwinm 8,5,27,31,31;"

        "stw    8,8(%[dp]);"
        "rlwinm 9,5,28,31,31;"

        "stw    9,12(%[dp]);"
        "rlwinm 6,5,29,31,31;"

        "stw    6,16(%[dp]);"
        "rlwinm 7,5,30,31,31;"

        "stw    7,20(%[dp]);"
        "rlwinm 8,5,31,31,31;"

        "stw    8,24(%[dp]);"
        "rlwinm 9,5,0,31,31;"

        "stw    9,28(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory", "5",  "6",  "7",  "8",  "9"
        );
    s0 += 8;
    s1 += 8;
    dp += 8;
  }

  n = count & 0x07;
  while ( n-- ) {
    (*dp) = (*s0) || (*s1);

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _pami_core_uint32_lxor2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const uint32_t *s0 = (const uint32_t *)srcs[0];
  const uint32_t *s1 = (const uint32_t *)srcs[1];

  int n = count >> 3;
  while ( n-- ) {
    asm volatile (
        "lwz     5,0(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,0(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   24,2,6;"
        "lwz     5,4(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,4(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   25,2,6;"
        "lwz     5,8(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,8(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   26,2,6;"
        "lwz     5,12(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,12(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   27,2,6;"
        "lwz     5,16(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,16(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   28,2,6;"
        "lwz     5,20(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,20(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   29,2,6;"
        "lwz     5,24(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,24(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   30,2,6;"
        "lwz     5,28(%[s0]);"

        "cmpwi   0,5,0;"
        "lwz     6,28(%[s1]);"

        "cmpwi   1,6,0;"

        "crxor   31,2,6;"


        "mfcr    5;"



        "rlwinm  6,5,25,31,31;"
        "stw     6,0(%[dp]);"

        "rlwinm  7,5,26,31,31;"

        "stw     7,4(%[dp]);"
        "rlwinm  6,5,27,31,31;"

        "stw     6,8(%[dp]);"
        "rlwinm  7,5,28,31,31;"

        "stw     7,12(%[dp]);"
        "rlwinm  6,5,29,31,31;"

        "stw     6,16(%[dp]);"
        "rlwinm  7,5,30,31,31;"

        "stw     7,20(%[dp]);"
        "rlwinm  6,5,31,31,31;"

        "stw     6,24(%[dp]);"
        "andi.   7,5,0x01;"

        "stw     7,28(%[dp]);"




      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory", "5", "6", "7"
    );

    s0 += 8;
    s1 += 8;
    dp += 8;
  }

  n = count & 0x07;
  register uint32_t s0_r;
  register uint32_t s1_r;
  while ( n-- ) {

    s0_r = *s0;
    s1_r = *s1;

    *dp = (s0_r && !s1_r) || (!s0_r && s1_r);

    dp++;
    s0++;
    s1++;
  }

  return;
}

void _pami_core_uint32_max2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const uint32_t *s0 = (const uint32_t *)srcs[0];
  const uint32_t *s1 = (const uint32_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"

      "lwz     9,0(%[s1]);"

      "lwz     6,4(%[s0]);"
      "cmplw   5,9;"

      "lwz     10,4(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0:    stw     5,0(%[dp]);"

      "lwz     7,8(%[s0]);"

      "lwz     11,8(%[s1]);"
      "cmplw   6,10;"

      "lwz     8,12(%[s0]);"
      "bge     1f;"

      "mr      6,10;"

"1:    stw     6,4(%[dp]);"
      "cmplw   7,11;"

      "lwz     12,12(%[s1]);"
      "bge     2f;"

      "mr      7,11;"

"2:    stw     7,8(%[dp]);"
      "cmplw   8,12;"

      "bge     3f;"

      "mr      8,12;"

"3:    stw     8,12(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register uint32_t s0_r;
  register uint32_t s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s0_r > s1_r) *(dp) = s0_r;
    else *(dp) = s1_r;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _pami_core_uint32_min2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {

  uint32_t *dp = (uint32_t *)dst;
  const uint32_t *s0 = (const uint32_t *)srcs[0];
  const uint32_t *s1 = (const uint32_t *)srcs[1];

  int n = count >> 2;
  while ( n-- ) {
    asm volatile (
      "lwz     5,0(%[s0]);"

      "lwz     9,0(%[s1]);"

      "lwz     6,4(%[s0]);"
      "cmplw   9,5;"

      "lwz     10,4(%[s1]);"
      "bge     0f;"

      "mr      5,9;"

"0: "
      "stw     5,0(%[dp]);"

      "lwz     7,8(%[s0]);"
      "cmplw   10,6;"

      "lwz     11,8(%[s1]);"
      "bge     1f;"

      "mr      6,10;"

"1: "
      "stw     6,4(%[dp]);"
      "cmplw   11,7;"

      "lwz     8,12(%[s0]);"
      "bge     2f;"

      "mr      7,11;"

"2: "
      "lwz     12,12(%[s1]);"

      "stw     7,8(%[dp]);"
      "cmplw   12,8;"

      "bge     3f;"

      "mr      8,12;"

"3: "
      "stw     8,12(%[dp]);"

      : // no outputs
      : [s0] "b" (s0),
        [s1] "b" (s1),
        [dp] "b" (dp)
      : "memory",
        "5",  "6",  "7",  "8",  "9",  "10", "11", "12"
        );
    dp += 4;
    s0 += 4;
    s1 += 4;
  }

  register uint32_t s0_r;
  register uint32_t s1_r;
  n = count & 0x03;
  while ( n-- ) {
    s0_r = *s0;
    s1_r = *s1;
    if (s1_r > s0_r) *(dp) = s0_r;
    else *(dp) = s1_r;

    s0++;
    s1++;
    dp++;
  }

  return;
}

void _pami_core_uint32_prod2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
        const uint32_t *s0 = srcs[0];
        const uint32_t *s1 = srcs[1];
        int num;
        int remainder = count;
        if (count <= 2048) {
                num = count >> 2;
                remainder = count - (num << 2);
                register int r0=0;
                register int r1=0;
                register int r2=0;
                register int r3=0;
                register int r4=0;
                register int r5=0;
                register int r6=0;
                register int r7=0;
                while (num--) {
                        asm volatile(
                                "lwz   %[r0],0(%[s0]);"
                                "lwz   %[r4],0(%[s1]);"
                                "lwz   %[r1],4(%[s0]);"
                                "lwz   %[r5],4(%[s1]);"

                                "mullw %[r0],%[r0],%[r4];"
                                "stw   %[r0],0(%[dp]);"

                                "lwz   %[r2],8(%[s0]);"
                                "lwz   %[r6],8(%[s1]);"
                                "mullw %[r1],%[r1],%[r5];"
                                "stw   %[r1],4(%[dp]);"

                                "lwz   %[r3],12(%[s0]);"
                                "lwz   %[r7],12(%[s1]);"
                                "mullw %[r2],%[r2],%[r6];"
                                "stw   %[r2],8(%[dp]);"

                                "mullw %[r3],%[r3],%[r7];"
                                "stw   %[r3],12(%[dp]);"

                                "addi  %[s0],%[s0],16;"
                                "addi  %[s1],%[s1],16;"
                                "addi  %[dp],%[dp],16;"

                                : [s0] "+b" (s0),
                                  [s1] "+b" (s1),
                                  [dp] "+b" (dst),
                                  [r0] "+r" (r0),
                                  [r1] "+r" (r1),
                                  [r2] "+r" (r2),
                                  [r3] "+r" (r3),
                                  [r4] "+r" (r4),
                                  [r5] "+r" (r5),
                                  [r6] "+r" (r6),
                                  [r7] "+r" (r7)
                                :
                                : "memory");
                }
                for (num = 0; num < remainder; ++num) {
                        dst[num] = s0[num] * s1[num];
                }
                return;
        }

        uint32_t *s2 = dst;
        num = (count - 16) >> 3;
        remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("mullw %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
        for (num = 0; num < remainder; ++num) {
                s2[num] = s0[num] * s1[num];
        }

        //fprintf (stderr, "<< Core_uint32_prod()\n");
        return;
}

void _pami_core_uint32_sum2(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
        //fprintf (stderr, ">> Core_uint32_sum()\n");

        const uint32_t *s0 = srcs[0];
        const uint32_t *s1 = srcs[1];
        int num;
        int remainder = count;
        if (count <= 2048) {
                num = count >> 2;
                remainder = count - (num << 2);
                register int r0=0;
                register int r1=0;
                register int r2=0;
                register int r3=0;
                register int r4=0;
                register int r5=0;
                register int r6=0;
                register int r7=0;

                while (num--) {
                        asm volatile(
                                "lwz   %[r0],0(%[s0]);"
                                "lwz   %[r4],0(%[s1]);"
                                "lwz   %[r1],4(%[s0]);"
                                "lwz   %[r5],4(%[s1]);"

                                "add   %[r0],%[r0],%[r4];"
                                "stw   %[r0],0(%[dp]);"

                                "lwz   %[r2],8(%[s0]);"
                                "lwz   %[r6],8(%[s1]);"
                                "add   %[r1],%[r1],%[r5];"
                                "stw   %[r1],4(%[dp]);"

                                "lwz   %[r3],12(%[s0]);"
                                "lwz   %[r7],12(%[s1]);"
                                "add   %[r2],%[r2],%[r6];"
                                "stw   %[r2],8(%[dp]);"

                                "add   %[r3],%[r3],%[r7];"
                                "stw   %[r3],12(%[dp]);"

                                "addi  %[s0],%[s0],16;"
                                "addi  %[s1],%[s1],16;"
                                "addi  %[dp],%[dp],16;"

                                : [s0] "+b" (s0),
                                  [s1] "+b" (s1),
                                  [dp] "+b" (dst),
                                  [r0] "+r" (r0),
                                  [r1] "+r" (r1),
                                  [r2] "+r" (r2),
                                  [r3] "+r" (r3),
                                  [r4] "+r" (r4),
                                  [r5] "+r" (r5),
                                  [r6] "+r" (r6),
                                  [r7] "+r" (r7)
                                :
                                : "memory");
                }
                for (num = 0; num < remainder; ++num) {
                        dst[num] = s0[num] + s1[num];
                }
                return;
        }

        uint32_t *s2 = dst;
        num = (count - 16) >> 3;
        remainder = (count & 0x07) + 16;
#define OP2(a,b)	asm volatile ("add %0, %0, %1" : "=r"(a) : "r"(b))
#include "ppc450d/_optim_uint32_dual_src.x.h"
#undef OP2
        for (num = 0; num < remainder; ++num) {
                s2[num] = s0[num] + s1[num];
        }

        //fprintf (stderr, "<< Core_uint32_sum()\n");
        return;
}

void _pami_core_uint32_band4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a&b&c&d)
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}

void _pami_core_uint32_bor4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a|b|c|d)
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}

void _pami_core_uint32_bxor4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a^b^c^d)
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}

void _pami_core_uint32_max4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	MAX(MAX(a,b),MAX(c,d))
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}

void _pami_core_uint32_min4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	MIN(MIN(a,b),MIN(c,d))
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}

void _pami_core_uint32_prod4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a*b*c*d)
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}

void _pami_core_uint32_sum4(uint32_t *dst, const uint32_t **srcs, int nsrc, int count) {
const uint32_t *src0 = srcs[0];
const uint32_t *src1 = srcs[1];
const uint32_t *src2 = srcs[2];
const uint32_t *src3 = srcs[3];
#define OP(a,b,c,d)	(a+b+c+d)
#define TYPE		uint32_t
#include "_quad_src.x.h"
#undef OP
#undef TYPE
        return;
}
