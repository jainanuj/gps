#ifndef _PAR_MPI_H
#define _PAR_MPI_H

#include <stdlib.h>

/*
 * ----------------------------------------------------------------------------
 */

#ifndef USE_MPI

// to simplify coding, all of these functions are called
// whether in MPI mode or not.  By defining them as macros that
// don't do much, the compiler should take care of things.

#define gsi_to_lp(x,y) (y)

#define gpi_to_lpi(x,y) (y)
#define lpi_to_gpi(x,y) (y)

#define lp_get_val(x,y) (x->states[y].max_value)
#define lp_get_gstate(x,y) (y)
#define is_partition_local(x,y) 1

/*
 * ----------------------------------------------------------------------------
 */

#else // USE_MPI is defined

#include "mpi.h"
/* #include "cont_world.h" */
#include "part_stuff.h"

/*
 * ---------------------------------------------------------------------------
 */

#define ZETAG 99

#define PROCESS_DONT_BLOCK 0
#define PROCESS_BLOCK      1

#define MSG_PART_UPDATE_MSG 42
#define MSG_LTFSD           43
#define MSG_LTFSD_DONE      44
#define MSG_LTFSD_GO        45
#define MSG_LTFSD_TERMINATE 46

#define MSG_PROC_DONE       51
#define MSG_PROC_UNDONE     52

#define MSG_PROC_TERMPING   63
#define MSG_PROC_TERMINATE  64

typedef struct {
  int msg_type;
  int originating_proc;
  int g_part_changed;
  int state_changed_count;
} part_update_msg_t;


typedef struct {
  int msg_type;
  int originating_proc;
  int state_count;
} ltfsd_msg_t;

typedef struct {
  int global_state_num;
  float new_val;
} state_changed_t;

/*
 * ---------------------------------------------------------------------------
 */

void mpi_init( world_t *w, int *argc, char **argv[] );

/* ----------------------------------------------------------------------- */

void attractor_assign_partitions( world_t *w, int *lpc );
void randomly_assign_partitions( world_t *w, int *lpc );

void assign_and_count_local_parts( world_t *w, int *lpc );
void count_local_states( world_t *w, int *lsc );

int is_partition_local( world_t *w, int pnum );

/* ----------------------------------------------------------------------- */

int gsi_to_lp( world_t *w, int g_state_num );
float lp_get_val( world_t *w, int lpnum );
int lp_get_gstate( world_t *w, int lpnum );

int gpi_to_lpi( world_t *w, int g_part_num );
int lpi_to_gpi( world_t *w, int l_part_num );

/* ----------------------------------------------------------------------- */

int send_msg( world_t *w, void *data, int length, int proc );

int are_msgs_waiting( void );
void process_incoming_msgs( world_t *w, int block );
int process_mpi_msgs( world_t *w, int block );

void send_proc_done_msg( world_t *w );
void process_proc_done_msg( world_t *w );

void send_proc_undone_msg( world_t *w );
void process_proc_undone_msg( world_t *w );

void send_terminate_msg( world_t *w, int proc );
void process_terminate_msg( world_t *w );

void send_termping_msg( world_t *w, int proc );
void process_termping_msg( world_t *w );

/* ----------------------------------------------------------------------- */

void coordinate_dependencies( world_t *w );

int send_ltfsd_msg( world_t *w, int *index1 );
void process_ltfsd_msg( world_t *w, ltfsd_msg_t *msg );

void send_ltfsd_done_msg( world_t *w );
void process_ltfsd_done_msg( world_t *w );

void send_ltfsd_terminate_msg( world_t *w, int foreign_proc );
void process_ltfsd_terminate_msg( world_t *w );

/* ----------------------------------------------------------------------- */

void send_partition_update( world_t *w, int l_part_num );
void process_partition_update( world_t *w, part_update_msg_t *msg );

void compute_max_mesg_size( world_t *w );

/* ----------------------------------------------------------------------- */

#endif // defined USE_MPI

/*
 * ---------------------------------------------------------------------------
 */

#endif // defined _PAR_MPI_H
