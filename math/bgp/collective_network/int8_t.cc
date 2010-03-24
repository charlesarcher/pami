/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file math/bgp/collective_network/int8_t.cc
 * \brief Default C math routines for 8 bit signed integer operations.
 */

#include "pami_bg_math.h"
#include "util/common.h"
//#include "internal.h"

static void _pami_core_int8_conv(uint8_t *dst, const int8_t *src, int count) {
#define OP(a) ((a)+(0x80))

#define TYPE uint8_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _pami_core_int8_conv_not(uint8_t *dst, const int8_t *src, int count) {
#define OP(a) (~((a)+(0x80)))

#define TYPE uint8_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _pami_core_int8_unconv(int8_t *dst, const uint8_t *src, int count) {
#define OP(a) ((a)-(0x80))

#define TYPE int8_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

static void _pami_core_int8_unconv_not(int8_t *dst, const uint8_t *src, int count) {
#define OP(a) (~((a)-(0x80)))

#define TYPE int8_t
#include "math/_single_src.x.h"
#undef TYPE
#undef OP
}

void _pami_core_int8_pre_all(uint8_t *dst, const int8_t *src, int count) {
  _pami_core_int8_conv(dst, src, count);
}

void _pami_core_int8_post_all(int8_t *dst, const uint8_t *src, int count) {
  _pami_core_int8_unconv(dst, src, count);
}

void _pami_core_int8_pre_min(uint8_t *dst, const int8_t *src, int count) {
  _pami_core_int8_conv_not(dst, src, count);
}

void _pami_core_int8_post_min(int8_t *dst, const uint8_t *src, int count) {
  _pami_core_int8_unconv_not(dst, src, count);
}
