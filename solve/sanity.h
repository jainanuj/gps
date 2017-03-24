#ifndef _SANITY_H
#define _SANITY_H

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "flags.h"
#include "bfuncs.h"

void check_heap( world_t *w );
void check_heat( world_t *w );
void check_parts( world_t *w );
void check_bcoords( world_t *w );

void dep_stats( world_t *w );
void compute_tc_wc_unv( world_t *w, float *tc, float *wc, int *unv );
void compute_dep_stats( world_t *w, float *sdp, float *pdp );
void final_stats( world_t *w, float iter_time, float coord_time );

void my_heap_dump_r( world_t *w, heap *h, int pos, int depth );
void my_heap_dump( world_t *w, heap *h );

void sanity_checks( world_t *w );

#endif
