
#ifndef _TYPES_H
#define _TYPES_H

#include "med_hash.h"
#include "odcd.h"
#include "intheap.h"
#include "intqueue.h"
#include "small-matvec.h"

#ifdef USE_MPI
#include "mpi.h"
#endif

#ifdef USE_AZTEC
#include "az_aztec.h"
#endif

/*
 * ----------------------------------------------------------------------------
 * Structures for representing our model
 * ----------------------------------------------------------------------------
 */

/* these structures need to be as small as possible */
#pragma pack(1)

/* the results of taking an action from a state */
typedef struct trans_t {

  /* this is the number of dependencies internal to the partition */
  unsigned short int_deps;

  /* this is the number of dependencies external to the partition */
  unsigned short ext_deps;

  entry_t *entries;
  prec_t reward;
} trans_t;

typedef struct state {
  trans_t *tps; /* the results of each action */
  int global_state_index;  /* to map l_state_t to g_state_t */
  unsigned char policy;
  unsigned char num_actions;
  prec_t *external_dep_vals;
} state_t;

typedef struct {
  int owning_processor;
  int local_part_num;
} part_proc_t;

#pragma pack()
/* now the structures can be any size */

/*
 * ----------------------------------------------------------------------------
 * Structures for dealing with partitions
 * ----------------------------------------------------------------------------
 */

typedef struct part_t {

  /* In general, heat is defined strictly between partitions.  However,
     the first time that a partition is processed, there may be some
     heat internal to the partition, due to internal rewards.  That's
     why we use primary_heat.  It will always be zero after the
     first time the partition is processed */
  prec_t heat, primary_heat;

  int visits, washes, my_heap_num;

  /* these are the transition probabilities for each state in 
     this partition. */
  int num_states;
  state_t *states;
  /* we only use this variable while loading the MDP */
  int cur_local_state;

  /* this is an array indicating which order variables
     should be processed in. */
  int *variable_ordering;

  /* this is the matrix stuff we use */
  vec_t *values;
  vec_t *rhs;

  matrix_t *cur_pol_matrix;

#ifdef USE_AZTEC
  int *bindx, data_org[AZ_COMM_SIZE];
  double *val;
#endif

  /* this partition has heat links to several other partitions.
     we don't distinguish between local and foreign here.
     this is just a map from integers (which are partition numbers)
     to prec_ts (which are the heats). */
  med_hash_t *heat_links;

  /* this hash collects all of the partitions (LOCAL ONLY) that depend
     on this partition.  this hash maps partition numbers to hashes. */
  med_hash_t *my_local_dependents;

  /* for visualization stuff */
  char marked;

  /* the cache element for ODCD */
  odcd_cache_elem_t odcd_elem;

#ifdef USE_MPI
  int g_part_num; /* this is this partition's global partition number. */

  /* this hash collects all of the foreign processors that depend on
     some state within this partition. */
  med_hash_t *my_foreign_dependents;

  /* this hash collects information mapping end partitions to starting
     states.  Again, it's so that we can processing incoming messages */
  med_hash_t *endpart_to_startstate;
#endif

} part_t;

#ifdef USE_MPI
/* this structure is used for sending messages to other processors.
   We have a separate buffer for each processor, and a separate
   MPI_Request type (because we use non-blocking sends). */
typedef struct mpi_req_stuff {
  MPI_Request mpi_req;
  int in_use;
  int buf_size;
  char *buf;
} mpi_req_stuff;
#endif

/*
 * ----------------------------------------------------------------------------
 * The world_t.  This is the central data structure.
 * ----------------------------------------------------------------------------
 */

typedef struct world_t {

  int num_global_parts;
  int num_global_states;

  /* if we're using MPI, these numbers will be the
     same as num_global_{parts,states}. */
  int num_local_parts;
  int num_local_states;

  part_t *parts;

  heap *part_heap;
  queue *part_queue;
    
  int cur_part_sorting;
  int num_value_updates;

  /* this is the number of partitions that we've processed. */
  int parts_processed;
  /* this is the number of value-iteration sweeps */
  int vi_sweeps;
  /* this is the number of policy-iteration sweeps */
  int pi_sweeps, max_pi_sweeps, pi_iters;

  /* this is the state value hash (it maps gsi's to prec_ts)
     (the prec_t being the current value of the state) */
  med_hash_t *foreign_state_val_hash;

  /* the ODCD cache */
  odcd_cache_t odcd_cache;

  /* this maps GLOBAL states to GLOBAL partnums! */
  int *state_to_partnum;

  int *gsi_to_lsi;

  /* parameters for the iterative linear system subsolvers */
  int solver;
  int kss_size;  /* the gmres restart parameter */
  int max_iters;
  prec_t tol;

#ifdef USE_AZTEC
    int az_proc_config[AZ_PROC_SIZE];
    int az_options[AZ_OPTIONS_SIZE];
    double az_params[AZ_PARAMS_SIZE];
    double az_status[AZ_STATUS_SIZE];
#endif

#ifdef USE_MPI

  /* someone else will tell us when to quit */
  int terminate;
  /* am I done processing stuff? */
  int processing_done;
  /* the number of processors that are done.  only significant for root. */
  int processing_done_cnt;

  int term_ping;
  int term_state;


  int ltfsd_done_cnt;
  int ltfsd_done;
  int ltfsd_myturn;

  /* total number of messages we have sent */
  int messages_sent;
  /* total size of the messages we have sent */
  int message_size;

  /* partition to processor mapping.  Who has what? If we have the
     partition, then what's its index in our local partition array? */
  /* this maps GLOBAL parts to processors! */
  part_proc_t *part_to_proc;

  /* this helps for processing messages from other procs. */
  med_hash_t *endpart_to_startpart;
  med_hash_t *fproc_data;

  int num_procs;
  int my_proc_num;
  char proc_name[MPI_MAX_PROCESSOR_NAME];
  int proc_namelen;

  int *recv_buf;
  int max_recv_size;

  mpi_req_stuff *mpi_reqs;

#endif /* def USE_MPI */

} world_t;

#endif
