
#ifndef _CONT_WORLD_H
#define _CONT_WORLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* for getpid() */
#include <unistd.h>

#include "logger.h"
#include "flags.h"
#include "types.h"

#include "disc.h"

/* #define USE_FUNKY_CACHE */

/*
 * ----------------------------------------------------------------------------
 * Other includes and general defines
 * ----------------------------------------------------------------------------
 */

#define TWO_TOTHE_D (1<<DIMENSIONS)
#define TWO_TOTHE_2D (1<<(DIMENSIONS*2))

#define MAX_TICKS 1000

#define NO_END_STATE -1

#define MAX(x,y) ((x)>(y)?x:y)
#define ABSMAX(x,y) (fabs(x)>fabs(y)? x : y)
#define MIN(x,y) (x<y?x:y)

#define TICK_SPACE_EXIT 0
#define TICK_CONTINUE 1
#define TICK_ONE_TICK 2
#define TICK_SIMPLE 3

#ifndef DEFAULT_VALUE
#   define DEFAULT_VALUE 0.0
#endif

/*
 * ----------------------------------------------------------------------------
 * Discretization type information
 * ----------------------------------------------------------------------------
 */

#define DISC_T_STD       0
#define DISC_T_ASYM_EDGE 1
#define DISC_T_ASYM_MID  2

#define NUM_DISC_TYPES 3

/* coordinate to grid mapping */
typedef int(*disc_func_t)( space_t *s, int i, coord_t *c );
/* grid to coordinate mapping */
typedef float(*undisc_func_t)( space_t *s, int i, grid_coord_t *c );

extern disc_func_t disc_funcs[ NUM_DISC_TYPES ];
extern undisc_func_t undisc_funcs[ NUM_DISC_TYPES ];
extern char *disc_names[ NUM_DISC_TYPES ];

/*
 * ----------------------------------------------------------------------------
 * Function prototypes
 * ----------------------------------------------------------------------------
 */

void init_stuff( space_t *s );
void compute_dim_info( space_t *s );

int kt_equal( kt_t *a, kt_t *b );
int kt_vertex_comparator( void *va, void *vb );
void kt_sort_vertices( kt_t *t, bc_t *b );

int iterate_over_grid( grid_coord_t *ll, grid_coord_t *ur, grid_coord_t *t );

void grid_warp( space_t *s, grid_coord_t *g );
int grid_to_state( space_t *s, grid_coord_t *g );
void state_to_grid( space_t *s, int state, grid_coord_t *g );
void coord_to_grid( space_t *s, coord_t *c, grid_coord_t *g );
int coord_to_state( space_t *s, coord_t *c );
void grid_to_coord( space_t *s, grid_coord_t *g, coord_t *c );
int coord_outside_space( space_t *s, coord_t *c );
int grid_coord_cmp( grid_coord_t *a, grid_coord_t *b );

int grid_manhattan_dist( space_t *s, grid_coord_t *a, grid_coord_t *b );
int grid_funky_dist( space_t *s, grid_coord_t *a, grid_coord_t *b );

void grid_diff( grid_coord_t *ur, grid_coord_t *ll, grid_coord_t *diff );
int gen_grid_to_index( grid_coord_t *extents, grid_coord_t *pos );
int grid_to_diffs_to_index( grid_coord_t *gc, grid_coord_t *ll,
			    grid_coord_t *extents );

float euclidean_dist( coord_t *a, coord_t *b );
void get_short_coord( space_t *s, coord_t *start, coord_t *end,
		      coord_t *result );
int left_or_right( space_t *s, int i, float a, float b );

void make_ur( space_t *s, grid_coord_t *g );
void make_ur_minus_one( space_t *s, grid_coord_t *g );

void make_cube_ur( space_t *s, grid_coord_t *ll, grid_coord_t *ur );

void cube_rel_init( space_t *s, coord_t *c, coord_t *c_ll, coord_t *c_ur, 
		    cube_rel_t *h );
void cube_rel_swap( cube_rel_t *h, int a, int b );
void cube_rel_sort( cube_rel_t *h );

void coord_to_bcoords( space_t *s, coord_t *c, bc_t *bary_coords,
		       kt_t *triangle );

void tick_until_transition( space_t *s,
			    coord_t *start_c,
			    grid_coord_t *real_start_gc,
			    int action,
			    trans_t *t,
			    coord_t *end_c,
			    grid_coord_t *end_gc,
			    int *nt );

void compute_transitions( space_t *s, char *fn );

void plot_vf( space_t *s, char *in_fn, char *out_fn );
void plot_partitioning( space_t *s, char *in_fn, char *out_fn );

/* the problem will implement these for us */
extern void initialize_space_desc( space_t *s );
extern int tick( tick_interface_t *ti );
/* extern void initialize_partitioning_desc( part_desc_t *pd ); */

/*
 * ----------------------------------------------------------------------------
 */

#endif
