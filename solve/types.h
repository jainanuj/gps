
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
  prec_t *external_dep_vals;       //array for storing aggregate of all extrnal states to this state so they are not recomputed
  val_t ***external_state_vals;     //Pointer to value of external state stored in the partition.

    int size_external_dep_vals;            //Just for tracking the memory occupied by the state.
    int size_external_state_vals;     //Just for tracking the memory occupied by the state.
    int size_tps;                     //Tracking the memory occupied by transitions from the state.
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

//Each part is further divided into sub parts
typedef struct level1_part_t {
    int *sub_parts;        //This is an array of state indices for that sub part
    int num_sub_parts;     //Number of states in each sub_part (read in so far);
    med_hash_t *my_local_dependents;
} level1_part_t;

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


  /* this is the matrix stuff we use */
  vec_t *values;
  vec_t *rhs;

  matrix_t *cur_pol_matrix;

  /* this hash collects all of the partitions (LOCAL ONLY) that depend
     on this partition.  this hash maps partition numbers to hashes. */
  med_hash_t *my_local_dependents;
  med_hash_t *my_ext_parts_states;
    med_hash_t *my_global_dependents;

  /* for visualization stuff */
  char marked;
    

  /* the cache element for ODCD */
  odcd_cache_elem_t odcd_elem;
    
    int size_states;                    //Just for tracking the memory occupied by states in the partition.
    int size_my_local_deps;             //Just for tracking the memory occupied in the partition.
    int size_my_ext_parts;              //Just for tracking the memory occupied in the partition.
    int size_values;                    //Just for tracking the memory occupied in the partition.
    int size_rhs;                       //Just for tracking the memory occupied in the partition.
    int size_sub_parts;                 //Tracking memory by everything sub_part.
} part_t;


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
    
    int num_level1_parts;

  part_t *parts;
    
    level1_part_t   *level1_parts;

  heap *part_heap;
  queue *part_queue;
  queue *part_level1_queue;
    bit_queue *part_level0_bit_queue;
    
  int cur_part_sorting;
  int num_value_updates;
  int num_value_updates_iters;      //Counter for all cached updates.

  /* this is the number of partitions that we've processed. */
  int parts_processed;
  /* this is the number of value-iteration sweeps */
  int vi_sweeps;
  /* this is the number of policy-iteration sweeps */
  int pi_sweeps, max_pi_sweeps, pi_iters;

  
  /* the ODCD cache */
  odcd_cache_t odcd_cache;

  /* this maps GLOBAL states to GLOBAL partnums! */
  int *state_to_partnum;
    //This maps level0 parts to level1 parts
    int *part_level0_to_level1;
    
    
  int *gsi_to_lsi;

  /* parameters for the iterative linear system subsolvers */
  int solver;
  int kss_size;  /* the gmres restart parameter */
  int max_iters;
  prec_t tol;
//  double reward_or_value_updatetime;
    
    int size_part_queue;                        //Just for tracking the memory occupied by queue in the world.
    int size_parts[7];                             //Just for tracking the memory occupied by parts in the world.
    int size_state_to_partnum;                  //Just for tracking the memory occupied by state to part in the world.
    int size_gsi_to_lsi;                        //Just for tracking the memory occupied by gsi_to_lsi in the world.


} world_t;

#endif
