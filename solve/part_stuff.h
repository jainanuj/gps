
#ifndef _PART_STUFF_H
#define _PART_STUFF_H

#include <math.h>

#include "types.h"
#include "flags.h"
#include "bfuncs.h"

#define PART_PQ
/* #define PART_SEQ */

int pick_part_and_wash_it( world_t *w );

void update_partition_potentials( world_t *w, int l_part_num );
void clear_partition_heat( world_t *w, int l_part_num );
void compute_part_heat( world_t *w, int g_part_num );
prec_t part_bellman_error( world_t *w, int l_part );

inline float part_heat( world_t *w, int l_part_num );

int pick_partition( world_t *w );

void part_check_in( world_t *w, int l_part_num );

int get_next_part(world_t *w);
void add_partition_deps_for_eval(world_t *w, int l_part_changed);
int part_available_to_process(world_t *w);

#endif // defined _PART_STUFF_H
