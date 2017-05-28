
#ifndef _BFUNCS_H
#define _BFUNCS_H

/*
 * ----------------------------------------------------------------------------
 */

#include "types.h"
#include "flags.h"
#include "logger.h"
#include "part_stuff.h"
#include "par_mpi.h"
#include "stok.h"

#ifdef USE_AZTEC
#include "az_aztec.h"
#endif

/*
 * ----------------------------------------------------------------------------
 */

#define MAX(x,y) ((x)>(y)?x:y)
#define ABSMAX(x,y) (fabs(x)>fabs(y)? x : y)
#define MIN(x,y) (x<y?x:y)

#ifndef DEFAULT_VALUE
#   define DEFAULT_VALUE 0.0
#endif

/*
 * ----------------------------------------------------------------------------
 */


#define MAX_ITERS_PP 40
/*

Philosophy / design decision:

A state-to-part is defined BEFORE we load the file.
A part-to-proc mapping is defined BEFORE we load the file.
Every processor has a full state-to-part and part-to-proc.
Everyone parses the file.
We all skip the states that we don't care about.
*/

/*

DEFINITION OF THE MDP FILE FORMAT:

totalstates
state1 nactions
ndeps sprime1 prob1 sprime2 prob2 sprime3 prob3...
ndeps sprime1 prob1 sprime2 prob2 sprime3 prob3...
...
state2 nactions
ndeps sprime1 prob1 sprime2 prob2 sprime3 prob3...
ndeps sprime1 prob1 sprime2 prob2 sprime3 prob3...
...

DEFINITION OF THE PARTITIONING FILE FORMAT:

totalstates
totalpartitions
part for state 0
part for state 1
part for state 2
part for state 3
...

*/

/*
 * ----------------------------------------------------------------------------
 */

void peek_at_mdp( world_t *w, char *fn );

void load_state_to_part( world_t *w, char *fn );
void load_part_to_proc( world_t *w, char *fn );
void load_mdp( world_t *w, char *fn );

void acquire_state_to_part( world_t *w );
void acquire_part_to_proc( world_t *w );

void count_states_in_parts( world_t *w );
void allocate_states_in_parts( world_t *w );

int init_world( world_t *w, char *fn );

void misc_mpi_initialization( world_t *w );

int part_cmp_func( int lp_a, int lp_b, void *vw );
void part_swap_func( int lp_a, int lp_b, void *vw );
void part_add_func( int lp_obj, int pos, void *vw );
void init_part_heap( world_t *w );
void init_part_queue( world_t *w );


void compute_cross_partition_deps( world_t *w );
void reorder_states_within_partitions( world_t *w );
void compute_initial_partition_priorities( world_t *w );
void cache_dependencies_in_states( world_t *w );

prec_t value_iterate_partition( world_t *w, int l_part );
prec_t value_iterate( world_t *w );

prec_t value_update( world_t *w, int l_part, int l_state );
prec_t value_update_iters( world_t *w, int l_part, int l_state );

int get_policy( world_t *w, int l_part, int l_state );

int get_max_deps( state_t *st );
int get_max_deps_nd( state_t *st, int l_state );

#ifdef USE_MPI
prec_t get_val( world_t *w, int g_state );
#endif

prec_t get_heat( world_t *w, int l_part, int l_state );
prec_t max_val_heat( world_t *w, int l_part, int l_state );
prec_t std_diff_heat( world_t *w, int l_part, int l_state );

prec_t reward_or_value( world_t *w, int l_part, int l_state, int action );
prec_t reward_or_value_iters( world_t *w, int l_part, int l_state, int action );


int iterate_over_parts_seq( world_t *w, int *local_part, int *global_part );

void add_dep( world_t *w,
	      int g_start_state, int l_start_state,
	      int g_start_part, int l_start_part,
	      int g_end_state, int g_end_part );

void add_part_ext_dep_states( world_t *w,
                             int g_start_part, int l_start_part,
                             int l_start_state, int l_end_state, int g_end_part );

void add_cache_states(world_t *w,
                      int g_start_part, int l_start_part,
                      int l_start_state, int l_end_state, int g_end_part, val_t **arrayValptrs, int indexVal );



void switch_to_policy_normal( world_t *w, int l_part, int l_state,
			      int new_pol );
void switch_to_policy( world_t *w, int l_part_num, int l_state, int policy );

void part_matrix_init( world_t *w, int l_part_num );

void setup_initial_policy( world_t *w );

void initialize_partitions( world_t *w );

int state_to_partnum( world_t *w, int g_state );
int lsi_to_gsi( world_t *w, int l_part, int l_state );
int gsi_to_lsi( world_t *w, int g_state );

int odcd_enabled( void );
int attractors_enabled( void );

void save_resulting_vector( world_t *w, char *fn );
void check_movie( world_t *w );

void translate_and_negate_all( world_t *w );
void translate_to_local_matrix( world_t *w, int l_part );

prec_t get_remainder( world_t *w, int l_part, int l_state, int action );
void fold( world_t *w, int l_part );
void unfold( world_t *w, int l_part );
void _fold( world_t *w, int l_part, int add_or_sub );

int policy_improvement( world_t *w, int l_part );

prec_t policy_evaluate_normal( world_t *w, int l_part, int *iters );
prec_t policy_evaluate( world_t *w, int l_part, int *iters );

void negate_matrix( world_t *w, int l_part );

/* void copy_values_to_svhash( world_t *w, int l_part ); */

/*
 * ----------------------------------------------------------------------------
 */

#ifdef USE_AZTEC

prec_t policy_evaluate_aztec( world_t *w, int l_part, int *iters );
void switch_to_policy_aztec( world_t *w, int l_part, int l_state,
			     int new_pol );

char *az_why( int why );

void count_nz( world_t *w, int l_part, int *tsize );

void initial_matrix_to_aztec( world_t *w, int l_part );
void allocate_aztec( world_t *w, int l_part );
void deallocate_aztec( world_t *w, int l_part );

void setup_aztec_params( world_t *p );

#endif

/*
 * ----------------------------------------------------------------------------
 */

#endif
