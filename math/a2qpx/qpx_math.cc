
#include "sys/pami.h"
#include "math/math_coremath.h"
#include "math_optimath.h"

#include "qpx_2way_sum.h"
#include "qpx_2way_min.h"
#include "qpx_2way_max.h"

#include "qpx_4way_sum.h"
#include "qpx_4way_min.h"
#include "qpx_4way_max.h"

#include "qpx_8way_sum.h"
#include "qpx_8way_min.h"
#include "qpx_8way_max.h"

#include "qpx_16way_sum.h"
#include "qpx_16way_min.h"
#include "qpx_16way_max.h"

void _pami_qpx_fp64_sum_2way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_sum_2way(dst, (double*) srcs[0], (double *)srcs[1], count);
}
void _pami_qpx_fp64_min_2way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_min_2way(dst, (double*) srcs[0], (double *)srcs[1], count);
}
void _pami_qpx_fp64_max_2way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_max_2way(dst, (double*) srcs[0], (double *)srcs[1], count);
}


void _pami_qpx_fp64_sum_4way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_sum_4way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], count);
}
void _pami_qpx_fp64_min_4way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_min_4way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], count);
}
void _pami_qpx_fp64_max_4way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_max_4way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], count);
}


void _pami_qpx_fp64_sum_8way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_sum_8way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], 
		       (double*) srcs[4], (double *)srcs[5], (double*) srcs[6], (double *)srcs[7], 		       
		       count);
}
void _pami_qpx_fp64_min_8way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_min_8way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], 
		       (double*) srcs[4], (double *)srcs[5], (double*) srcs[6], (double *)srcs[7], 		       
		       count);  
}
void _pami_qpx_fp64_max_8way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_max_8way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], 
		       (double*) srcs[4], (double *)srcs[5], (double*) srcs[6], (double *)srcs[7], 		       
		       count);
}


void _pami_qpx_fp64_sum_16way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_sum_16way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], 
			(double*) srcs[4],  (double *)srcs[5],  (double*) srcs[6],  (double *)srcs[7], 		       
			(double*) srcs[8],  (double *)srcs[9],  (double*) srcs[10], (double *)srcs[11],      
			(double*) srcs[12], (double *)srcs[13], (double*) srcs[14], (double *)srcs[15],     
			count);
}
void _pami_qpx_fp64_min_16way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_min_16way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], 
			(double*) srcs[4],  (double *)srcs[5],  (double*) srcs[6],  (double *)srcs[7], 		       
			(double*) srcs[8],  (double *)srcs[9],  (double*) srcs[10], (double *)srcs[11], 	    
			(double*) srcs[12], (double *)srcs[13], (double*) srcs[14], (double *)srcs[15],     
			count);  
}
void _pami_qpx_fp64_max_16way(double *dst, const double **srcs, int nsrc, int count) {
  quad_double_max_16way(dst, (double*) srcs[0], (double *)srcs[1], (double*) srcs[2], (double *)srcs[3], 
			(double*) srcs[4],  (double *)srcs[5],  (double*) srcs[6],  (double *)srcs[7], 		       
			(double*) srcs[8],  (double *)srcs[9],  (double*) srcs[10], (double *)srcs[11], 	      
			(double*) srcs[12], (double *)srcs[13], (double*) srcs[14], (double *)srcs[15],       
			count); 
}
