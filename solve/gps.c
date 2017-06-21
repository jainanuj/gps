
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>

#include "sanity.h"
#include "flags.h"
#include "opts.h"
#include "par_mpi.h"
#include "med_hash.h"
#include "part_stuff.h"
#include "logger.h"

#define NUM_CRANKS 200

#define SAVE_TICKS 100000

/*
 * ----------------------------------------------------------------------------
 */

double when( void );

void solve_using_prioritized_vi( world_t *w );
void solve_using_regular_vi( world_t *w, int run_type );

void solve_using_prioritized_pi( world_t *w );
void solve_using_regular_pi( world_t *w );

#ifdef USE_MPI
void mpi_solve_using_prioritized_vi( world_t *w );
void mpi_solve_using_regular_vi( world_t *w, int run_type );
#endif

void print_general_information( world_t *w );

/*
 * ----------------------------------------------------------------------------
 */

/* don't use floats with this function -- you'll run into precision
   problems! */
double when( void ) {
  struct timeval tv;
  gettimeofday( &tv, NULL );
  return (double)(tv.tv_sec) + (double)(tv.tv_usec)*(double)(1e-6);
}

/*
 * ----------------------------------------------------------------------------
 * Prioritized partition-based solver -- main algorithm
 * ----------------------------------------------------------------------------
 */

// this is the non-MPI run loop
void solve_using_prioritized_vi( world_t *w ) {
  int total_visits;

  total_visits = 0;
  while (1) {
    if ( pick_part_and_wash_it( w ) ) {
      total_visits++;
      check_movie( w );
    } else {
      break;
    }
  }

  if ( verbose ) {
    wlog( 1, "\n\nTotal partitions processed: %d\n\n", total_visits );
  }

}

/*
 * ----------------------------------------------------------------------------
 */

#ifdef USE_MPI

// this is the parallel version.
void mpi_solve_using_prioritized_vi( world_t *w ) {
  int total_visits, block;

  total_visits = 0;
  while (1) {

    if ( w->terminate ) {
      break;
    }

    // do local stuff
    if ( pick_part_and_wash_it( w ) ) {
      total_visits++;
      // there's still stuff for us to do. we don't want to block on
      // the MPI call.
      block = PROCESS_DONT_BLOCK;

      // if we were in a done state, and we just found work, then
      // we're in a working state again.
      if ( w->processing_done == 1 ) {
	w->processing_done = 0;
	send_proc_undone_msg( w );
      }

    } else {
      // we don't have any local partitions that need processing.
      // we can just block on an MPI update.
      block = PROCESS_BLOCK;
      if ( w->num_procs == 1 ) {
	break;
      }

      // if we were in a working state, and there isn't anything
      // for us to do, we're in a done state.
      if ( w->processing_done == 0 ) {
	w->processing_done = 1;
	send_proc_done_msg( w );
      }
    }

    process_incoming_msgs( w, block );
  }

  if ( verbose ) {
    wlog( 1, "Total partitions processed: %d\n\n", total_visits );
  }

}

#endif

/*
 * ----------------------------------------------------------------------------
 * Policy iteration solver -- main algorithm
 * ----------------------------------------------------------------------------
 */

int pick_part_and_solve_it( world_t *w ) {
  int l_part, changed, iters;
  float maxheat;

  l_part = pick_partition( w );
  if ( l_part == -1 ) {
    /* either there was an error, or there isn't any more heat. */
    return 0;
  }

  w->parts_processed++;
  w->parts[ l_part ].visits++;

/*   wlog( 1, "** %d\n", l_part ); */
  do {
    changed = policy_improvement( w, l_part );
    maxheat = policy_evaluate( w, l_part, &iters );
    w->pi_iters += iters;
/*     wlog( 1, "  %.6f %d %d\n", maxheat, changed, iters ); */
  } while ( changed != 0 );

  update_partition_potentials( w, l_part );
    
#ifdef USE_MPI
  /* inform foreign processors! */
  send_partition_update( w, l_part );
#endif

#ifdef PART_PQ
  /* when we picked the partition, we pulled it off the heap.
     now that its heat is 0, put it back */
  heap_add( w->part_heap, l_part );
#endif

  return 1;
}

void solve_using_prioritized_pi( world_t *w ) {

  while (1) {
    if ( pick_part_and_solve_it( w ) ) {
      check_movie( w );
    } else {
      break;
    }
  }

  if ( verbose ) {
    wlog( 1, "\n\nTotal partitions processed: %d\n", w->parts_processed );
    wlog( 1, "Total iterations: %d\n\n", w->pi_iters );
  }

}

void solve_using_regular_pi( world_t *w ) {
  int changed, l_part, l_state, state_cnt, iters;
  prec_t maxheat, tmpheat;

  w->pi_sweeps = 0;

  check_movie( w );

  do {

    w->pi_sweeps++;

    /* piecewise evaluate and improve the policy */

    changed = 0;

    for ( l_part=0; l_part < w->num_local_parts; l_part++ ) {
      check_movie( w );
      policy_evaluate( w, l_part, &iters );
      changed += policy_improvement( w, l_part );
      w->pi_iters += iters;
    }

    /* figure out what the new maxnorm error is */
    maxheat = 0;
    for ( l_part=0; l_part < w->num_local_parts; l_part++ ) {
      state_cnt = w->parts[ l_part ].num_states;
      for ( l_state = 0; l_state < state_cnt; l_state++ ) {
	tmpheat = std_diff_heat( w, l_part, l_state );
	maxheat = MAX( maxheat, tmpheat );
      }
    }

/*     wlog( 1, "%d %.8lf\n", changed, maxheat ); */

    w->parts_processed += w->num_local_parts;

    if ( w->pi_sweeps >= w->max_pi_sweeps ) {
      break;
    }
    if ( maxheat < w->tol ) {
      break;
    }

  } while ( 1 );

    //  } while ( changed != 0 );

/*   for ( l_part=0; l_part < w->num_local_parts; l_part++ ) { */
/*     copy_values_to_svhash( w, l_part ); */
/*   } */

  if ( verbose ) {
    wlog( 1, "Iterations: %d; error = %.5lf\n", w->pi_sweeps, maxheat );
    wlog( 1, "Parts processed: %d\n", w->parts_processed );
    wlog( 1, "Total iterations: %d\n", w->pi_iters );
  }
}

/*
 * ----------------------------------------------------------------------------
 * Standard value iteration solver -- main algorithm
 * ----------------------------------------------------------------------------
 */

void solve_using_regular_vi( world_t *w, int type ) {
  prec_t heat;

  w->vi_sweeps = 0;
  w->parts_processed = 0;

  heat = value_iterate( w );
  (w->vi_sweeps)++;
  (w->parts_processed) += w->num_local_parts;

/*     if ( verbose ) { */
/*       wlog( 1, "%.5f\n", heat ); */
/*     } */

  check_movie( w );


  if ( verbose ) {
    wlog( 1, "Total sweeps: %d\n\n", w->vi_sweeps );
    wlog( 1, "Total parts processed: %d\n\n", w->parts_processed );
  }
}

/*
 * ----------------------------------------------------------------------------
 */

#ifdef USE_MPI

/* this is the parallel vi solver */
void mpi_solve_using_regular_vi( world_t *w, int type ) {
  prec_t heat;

  w->vi_sweeps = 0;
  while (1) {

    if ( w->terminate ) {
      break;
    }

    heat = value_iterate( w );
    (w->vi_sweeps)++;

    // do local stuff
    if ( heat > heat_epsilon ) {
      // if we were in a done state, and we just found work, then
      // we're in a working state again.
      if ( w->processing_done == 1 ) {
        w->processing_done = 0;
        send_proc_undone_msg( w );
      }

    } else {
      // we don't have any local partitions that need processing.
      if ( w->num_procs == 1 ) {
        break;
      }

      // if we were in a working state, and there isn't anything
      // for us to do, we're in a done state.
      if ( w->processing_done == 0 ) {
        w->processing_done = 1;
        send_proc_done_msg( w );
      }

      // block on an incoming MPI message.
      process_mpi_msgs( w, PROCESS_BLOCK );

    }
  }

  if ( verbose ) {
    wlog( 1, "Total sweeps: %d\n\n", w->vi_sweeps );
  }
}

#endif

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void true_error( world_t *w ) {
  int l_part, l_state, cnt;
  prec_t maxheat, tmp;

  maxheat = 0;
  for ( l_part=0; l_part<w->num_local_parts; l_part++ ) {
    cnt = w->parts[ l_part ].num_states;

    for ( l_state=0; l_state<cnt; l_state++ ) {
      tmp = std_diff_heat( w, l_part, l_state );
      if ( tmp > maxheat ) {
	maxheat = tmp;
      }
    }
  }

  wlog( 1, " TRUE ERROR: %.6f\n", maxheat );

}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void print_general_information( world_t *w ) {
  int size;
    int i; double avg_part_size;

  wlog( 1, "Local States        = %d\n", w->num_local_states );
  wlog( 1, "Global States       = %d\n", w->num_global_states );
  wlog( 1, "Local Partitions    = %d\n", w->num_local_parts );
  wlog( 1, "Global Partitions   = %d\n", w->num_global_parts );
  wlog( 1, "Sub-Parts / part    = %d\n", w->parts[0].num_sub_parts);
    
  if ( use_voting == VOTE_YES ) {
    wlog( 1, "  using voting!\n" );
  } else {
    wlog( 1, "  NOT using voting!\n" );
  }
  wlog( 1, "Epsilon             = %f\n", heat_epsilon );

  wlog( 1, "\n" );

  if ( attractors_enabled() ) {
    wlog( 1, "Attractors per part = %d\n", num_attractors );
  }

  if ( odcd_enabled() ) {
    wlog( 1, "ODCD cache size     = %d\n", odcd_cache_size );
  }

  wlog( 1, "\n" );

  if ( run_type == RUN_VI ) {
    wlog( 1, "Using normal value iteration\n" );
  } else if ( run_type == RUN_PVI ) {
    wlog( 1, "Using partitioned value iteration\n" );
  } else if ( run_type == RUN_PI ) {
    wlog( 1, "Using normal policy iteration\n" );
  } else if ( run_type == RUN_PPI ) {
    wlog( 1, "Using partitioned policy iteration\n" );
  }

  if ( heat_metric == HEAT_STD ) {
    wlog( 1, "Using standard heat metric\n" );
  } else if ( heat_metric == HEAT_ABS ) {
    wlog( 1, "Using absolute heat metric\n" );
  }

  // I used the 'size' variable here to silence some compiler
  // warnings. Sigh.
  wlog( 1, "\n" );

//    wlog(1, "Total part 0 size  = %d\n", w->parts[0].size_states + w->parts[0].size_values + w->parts[0].size_my_local_deps
//                                        + w->parts[0].size_my_ext_parts + w->parts[0].size_rhs);
/*    for (i = 0; i < w->num_local_parts; i++)
    {
        wlog(1, "Total part %d size= %d, ld=%d;ed=%d;v=%d;r=%d\n", i, w->parts[i].size_states, w->parts[i].size_my_local_deps, w->parts[i].size_my_ext_parts, w->parts[i].size_values, w->parts[i].size_rhs);
    }
*/
    wlog(1, "All parts sizes 0= %d; 1 = %d; 2 = %d; 3=%d; 4=%d; 5=%d; 6=%d\n",
         w->size_parts[0], w->size_parts[1], w->size_parts[2], w->size_parts[3],
         w->size_parts[4], w->size_parts[5], w->size_parts[6]);
    
    avg_part_size = (double)(w->size_parts[0] +w->size_parts[1] + w->size_parts[2]
                             + w->size_parts[3] + w->size_parts[4] +w->size_parts[5] + w->size_parts[6])/w->num_local_parts;
    
    wlog(1, "Avg partition size = %.6f\n", avg_part_size);
    
    
    wlog(1, "GSI to LSI size = %d\n", w->size_gsi_to_lsi);
    wlog(1, "state to part# = %d\n", w->size_state_to_partnum);
    wlog(1, "Queue size = %d\n", w->size_part_queue);// + sizeof(long) * w->part_queue->bitqueue->max_bit_arrays);

  wlog( 1, "\n" );
}

void calc_swap_time( int save, double iter_time ) {
  static struct rusage r_s;
  struct rusage r;
  double te, tb;

  if ( save ) {
    getrusage( RUSAGE_SELF, &r_s );
    return;
  }

  getrusage( RUSAGE_SELF, &r );

  wlog( 1, "ITER TIME: %.6f\n", iter_time );

  tb = (double)(r_s.ru_utime.tv_sec) +
    (double)(r_s.ru_utime.tv_usec)*(double)(1e-6);
  te = (double)(r.ru_utime.tv_sec) +
    (double)(r.ru_utime.tv_usec)*(double)(1e-6);
  wlog( 1, "USER TIME: %.6f\n", te-tb );

  tb = (double)(r_s.ru_stime.tv_sec) +
    (double)(r_s.ru_stime.tv_usec)*(double)(1e-6);
  te = (double)(r.ru_stime.tv_sec) +
    (double)(r.ru_stime.tv_usec)*(double)(1e-6);
  wlog( 1, "SYSTEM TIME: %.6f\n", te-tb );

  wlog( 1, "CORRECTED ITER TIME: %.6f\n", iter_time - (te-tb) );
}

/*
 * ============================================================================
 * ============================================================================
 */

int main( int argc, char *argv[] ) {
  int i, value_iterate_flag, totalWashes = 0;
  world_t *w;
    double t_start, t_end, global_start, global_end;
  double iter_time, coord_time, reorder_time = 0;
    double avg_part_size;
    FILE *fp;



  /* ------------ initialize the world ------------ */
    printf("Started gps");
//    sleep(10);
    
  t_start = when();
    global_start = when();

  w = (world_t *)malloc( sizeof( world_t ) );
  if ( w == NULL ) {
    fprintf( stderr, "Couldn't allocate world!\n" );
    exit( 0 );
  }

#ifdef USE_MPI
  mpi_init( w, &argc, &argv );
  if ( verbose ) {
    fprintf( stderr, "USING MPI: I AM NODE %d/%d [%s]\n",
	     w->my_proc_num, w->num_procs, w->proc_name );
  }

#endif

  parse_opts( argc, argv );

#ifdef USE_MPI
  open_logfile_mpi( w->proc_name, w->my_proc_num, w->num_procs );
#else
  //  open_logfile_default();
  open_logfile_stdout();
#endif

  if ( verbose ) {
    wlog( 1, "\n" );
    wlog( 1, "====================================================\n\n" );
    wlog( 1, "My pid is %d\n\n", getpid() );
    for ( i=0; i<argc; i++ ) {
      wlog( 1, "  %s\n", argv[i] );
    }
    wlog( 1, "====================================================\n\n" );
  }

    //Read up the MDP File
  if ( !init_world( w, mdp_fn ) ) {
    wlog( 1, "Error initializing world!\n");
    exit( 0 );
  }

//  if ( verbose ) {
//    print_general_information( w );
//  }

  t_end = when();
  if ( verbose ) {
    wlog( 1, "World init took %.6f seconds\n\n", t_end - t_start );
  }

  /* ------------ compute deps ------------ */

  if ( verbose ) { wlog( 1, "Computing cross-partition dependencies...\n" ); }
  t_start = when();
  compute_cross_partition_deps( w );
  cache_dependencies_in_states( w );

  t_end = when();
  if ( verbose ) { wlog( 1, "Took %.6f seconds\n\n", t_end - t_start ); }


  /* ------------ Translate global indices to local ones ------------ */

  if ( verbose ) { wlog( 1, "Translating matrices...\n" ); }
  t_start = when();
  translate_and_negate_all( w );
  t_end = when();
  if ( verbose ) { wlog( 1, "Took %.6f seconds\n\n", t_end - t_start ); }


  /* ------------ compute partition priorities ------------ */
/*
Deleted.
*/
  /* ------------ tell foreign processors about dependencies ------------ */

#ifdef USE_MPI
  if ( verbose ) { wlog( 1, "Coordinating dependencies...\n" );}
  t_start = when();
  compute_max_mesg_size( w );
  coordinate_dependencies( w );
  t_end = when();
  coord_time = t_end - t_start;
  if ( verbose ) { wlog( 1, "Took %.6f seconds\n\n", coord_time ); }
#else
  coord_time = 0;
#endif


  /* ------------ allocate and initialize the partition data structures */

  if ( verbose ) { wlog( 1, "Creating queue ...\n" ); }
  t_start = when();
//  init_part_heap( w );
    init_part_queue( w );     //Using the queue for regular VI.
  t_end = when();
  if ( verbose ) { wlog( 1, "Took %.6f seconds\n\n", t_end - t_start ); }

    if ( verbose ) {
        print_general_information( w );
    }

  /* ------------ other checks ------------ */

//  if ( do_sanity_checks ) {
//    sanity_checks( w );
//  }

  /* ------------ other inits ------------ */

  w->solver = solver;
  w->max_iters = 10000;
  w->tol = heat_epsilon;
  w->kss_size = 2;
  w->max_pi_sweeps = 200;

#ifdef USE_AZTEC
  if ( is_aztec_solver( w->solver ) ) {
    if ( verbose ) {
      wlog( 1, "Initializing aztec parameters...\n" );
    }
    t_start = when();
    setup_aztec_params( w );
    t_end = when();
    if ( verbose ) { wlog( 1, "Took %.6f seconds\n\n", t_end - t_start ); }
  }
#endif

  setup_initial_policy( w );


  /* ------------ begin processing ------------ */

  if ( verbose && run_type != RUN_VI && run_type != RUN_PVI ) {
    wlog( 1, "===================================================\n" );
    wlog( 1, "Solving with %s...\n", solver_name( w->solver ) );
  }

  if ( verbose ) {
    wlog( 1, "===================================================\n" );
    wlog( 1, "GO!\n\n" );
  }

  wlog_flush();

/*   calc_swap_time( 1, 0 ); */

  t_start = when();

  if ( run_type == RUN_VI ) {
    value_iterate_flag = 1;
#ifdef USE_MPI
    mpi_solve_using_regular_vi( w, run_type );
#else
    solve_using_regular_vi( w, run_type );
#endif

  } else if ( run_type == RUN_PVI ) {

    value_iterate_flag = 0;
#ifdef USE_MPI
    mpi_solve_using_prioritized_vi( w );
#else
    solve_using_prioritized_vi( w );
#endif

  } else if ( run_type == RUN_PI ) {
    value_iterate_flag = 0;
    solve_using_regular_pi( w );

  } else if ( run_type == RUN_PPI ) {
    value_iterate_flag = 0;
    solve_using_prioritized_pi( w );

  } else {
    wlog( 1, "HEY! Bad run type!\n" );
    exit( 0 );
  }

  t_end = when();

  iter_time = t_end - t_start;

/*   calc_swap_time( 0, iter_time ); */

  if ( echo_string != NULL ) {
/*     wlog( 1, "%s %.6f %d\n", echo_string, iter_time, w->num_value_updates ); */

    wlog( 1, "%-10s %-10.6f %-10.6f %-10.6f %-9d %-9d %-9d\n",
	  echo_string,
	  iter_time, reorder_time, iter_time + reorder_time, 
	  w->vi_sweeps, w->parts_processed, w->num_value_updates + w->num_value_updates_iters );


    /* the second number is the recompute ratio */
/*     wlog( 1, "%s %.2f %.2f\n", echo_string, */
/* 	  100 * odcd_hit_ratio( &(w->odcd_cache) ),  */
/* 	  (prec_t)(w->odcd_cache.cache_tries - */
/* 		   w->odcd_cache.cache_hits)/ */
/* 	  (prec_t)(w->num_local_parts) ); */

  } else {
    wlog( 1, "---- Took %.6f seconds ----\n", iter_time );
    wlog( 1, "---- Number of updates: %d ----\n",
	    w->num_value_updates + w->num_value_updates_iters);
    wlog( 1, "---- Number of Iter updates: %d ----\n",
        w->num_value_updates_iters);

  }

    for (i = 0; i < w->num_local_parts; i++)
    {
        totalWashes += w->parts[i].washes;
    }
    if ( verbose ) {
    if ( value_iterate_flag ) {
      wlog( 1, "  Sweeps %d times. Total washed: %d times\n", w->vi_sweeps, totalWashes );
    }
    wlog( 1, "===================================================\n" );
    wlog( 1, "\n\n");
  }




/*   if ( verbose ) {  */
/*     wlog( 1, "Collisions in state_val_hash: %d,%d,%d = %.2f\n", */
/* 	  w->state_val_hash->collisions, */
/* 	  w->state_val_hash->nelts, */
/* 	  w->state_val_hash->nalloc, */
/* 	  100 * (prec_t)(w->state_val_hash->collisions) / */
/* 	  (prec_t)(w->state_val_hash->nelts) ); */
/*   } */


  /* -------------- extra value iteration washes --------------- */

/*   start = time( NULL ); */

/*   fast_value_iterate( w ); */
/*   fast_value_iterate( w ); */

/*   wlog( 1, "Took %d seconds\n", (int)(end-start) ); */

  /* --------------- make sure we really converged  ------------------ */
  
/*   true_error( w ); */

  /* --------------- save the results  ------------------ */

  if ( save_fn != NULL ) {
    save_resulting_vector( w, save_fn );
  }

  /* -------------------- DONE!!!  ---------------------- */

  if ( verbose ) {
    final_stats( w, iter_time, coord_time );
  }

  if ( do_sanity_checks ) {
    check_heat( w );
  }

  odcd_cache_destroy( &( w->odcd_cache ) );
    if ( verbose ) { wlog( 1, "Queue Add + Pop time taken - Took %.6f seconds\n\n", w->part_queue->add_time +  w->part_queue->pop_time); }
    
//    if ( verbose ) { wlog( 1, "Reward_value updates - Took %.6f seconds\n\n", w->reward_or_value_updatetime); }

    //if ( verbose ) { wlog( 1, "Queue Pop time taken - Took %.6f seconds\n\n", w->part_queue->pop_time); }
    global_end = when();
    if ( verbose ) { wlog( 1, "Overal time taken - Took %.6f seconds\n\n", global_end - global_start ); }
    
    
    avg_part_size = (double)(w->size_parts[0] +w->size_parts[1] + w->size_parts[2]
                             + w->size_parts[3] + w->size_parts[4] +w->size_parts[5])/w->num_local_parts;
    
    if (stat_fn != NULL)
    {
        fp = fopen(stat_fn, "ab");
        fprintf(fp, "%.6f, %d, %.6f, %.6f, %.6f \n", avg_part_size, w->num_value_updates + w->num_value_updates_iters,
                global_end - global_start, iter_time, w->part_queue->add_time +  w->part_queue->pop_time);
        fclose(fp);
    }

#ifdef USE_MPI
  MPI_Finalize();
#endif

  return 0;
}
