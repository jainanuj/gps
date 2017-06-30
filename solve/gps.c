
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
void solve_using_regular_vi( world_t *w, int run_type );
void print_general_information( world_t *w );

/*
 * ----------------------------------------------------------------------------
 */

/* don't use floats with this function -- you'll run into precision
   problems! */
double when( void )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return (double)(tv.tv_sec) + (double)(tv.tv_usec)*(double)(1e-6);
}

/*
 * ----------------------------------------------------------------------------
 * Standard value iteration solver -- main algorithm
 * ----------------------------------------------------------------------------
 */

void solve_using_regular_vi( world_t *w, int type )
{
    prec_t heat;

    w->vi_sweeps = 0;
    w->parts_processed = 0;
    heat = value_iterate( w );
    (w->vi_sweeps)++;
    (w->parts_processed) += w->num_local_parts;
    check_movie( w );
    if ( verbose )
    {
        wlog( 1, "Total sweeps: %d\n\n", w->vi_sweeps );
        wlog( 1, "Total parts processed: %d\n\n", w->parts_processed );
    }
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void print_general_information( world_t *w ) {
//  int size;
    double avg_part_size;

    wlog( 1, "Local States        = %d\n", w->num_local_states );
    wlog( 1, "Global States       = %d\n", w->num_global_states );
    wlog( 1, "Local Partitions    = %d\n", w->num_local_parts );
    wlog( 1, "Global Partitions   = %d\n", w->num_global_parts );
//  wlog( 1, "Sub-Parts / part    = %d\n", w->parts[0].num_sub_parts);
    
    if ( use_voting == VOTE_YES )
    {
        wlog( 1, "  using voting!\n" );
    }
    else
    {
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

/*
 * ============================================================================
 * ============================================================================
 */

int main( int argc, char *argv[] )
{
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
    if ( w == NULL )
    {
        fprintf( stderr, "Couldn't allocate world!\n" );
        exit( 0 );
    }
    parse_opts( argc, argv );
    //  open_logfile_default();
    open_logfile_stdout();
    if ( verbose )
    {
        wlog( 1, "\n" );
        wlog( 1, "====================================================\n\n" );
        wlog( 1, "My pid is %d\n\n", getpid() );
        for ( i=0; i<argc; i++ )
        {
            wlog( 1, "  %s\n", argv[i] );
        }
        wlog( 1, "====================================================\n\n" );
    }
    //Read up the MDP File
    if ( !init_world( w, mdp_fn ) )
    {
        wlog( 1, "Error initializing world!\n");
        exit( 0 );
    }
    t_end = when();
    if ( verbose )
    {
        wlog( 1, "World init took %.6f seconds\n\n", t_end - t_start );
//        exit(0);
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

    /* ------------ allocate and initialize the partition data structures */

    if ( verbose ) { wlog( 1, "Creating queue ...\n" ); }
    t_start = when();
    init_level1_part_queue( w );     //Using the queue for regular VI.
    t_end = when();
    if ( verbose ) { wlog( 1, "Took %.6f seconds\n\n", t_end - t_start ); }

    if ( verbose ) { print_general_information( w ); }

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
    setup_initial_policy( w );


  /* ------------ begin processing ------------ */

    if ( verbose && run_type != RUN_VI && run_type != RUN_PVI )
    {
        wlog( 1, "===================================================\n" );
        wlog( 1, "Solving with %s...\n", solver_name( w->solver ) );
    }

    if ( verbose )
    {
        wlog( 1, "===================================================\n" );
        wlog( 1, "GO!\n\n" );
    }
    wlog_flush();

    t_start = when();
    if ( run_type == RUN_VI )
    {
        value_iterate_flag = 1;
        solve_using_regular_vi( w, run_type );
    }
    else
    {
        wlog( 1, "HEY! Bad run type!\n" );
        exit( 0 );
    }
    t_end = when();

    iter_time = t_end - t_start;
    if ( echo_string != NULL )
    {
/*     wlog( 1, "%s %.6f %d\n", echo_string, iter_time, w->num_value_updates ); */

        wlog( 1, "%-10s %-10.6f %-10.6f %-10.6f %-9d %-9d %-9d\n",
	  echo_string,
	  iter_time, reorder_time, iter_time + reorder_time, 
	  w->vi_sweeps, w->parts_processed, w->num_value_updates + w->num_value_updates_iters );
    }
    else
    {
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
    if ( verbose )
    {
        if ( value_iterate_flag )
        {
            wlog( 1, "  Sweeps %d times. Total washed: %d times\n", w->vi_sweeps, totalWashes );
        }
        wlog( 1, "===================================================\n" );
        wlog( 1, "\n\n");
    }

    if ( save_fn != NULL )
    {
        save_resulting_vector( w, save_fn );
    }

  /* -------------------- DONE!!!  ---------------------- */

    if ( verbose )
    {
        final_stats( w, iter_time, coord_time );
    }

    if ( do_sanity_checks )
    {
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
    return 0;
}
