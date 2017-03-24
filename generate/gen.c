
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "flags.h"
#include "cont_world.h"
#include "opts.h"
#include "logger.h"

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void print_general_information( space_t *s ) {

  wlog( 1, "\n\nGENERATING MDP FOR ==> %s <==\n\n", PROB_STR );

  wlog( 1, "Dimensions          = %d\n", DIMENSIONS );
  wlog( 1, "Total states        = %d\n", s->total_states );
  wlog( 1, "Gamma               = %f\n", discount_factor );
  wlog( 1, "Base timestep       = %f\n", base_timestep );

  if ( use_variable_timestep == VT_YES ) {
    wlog( 1, "  using variable timestep!\n" );
  } else {
    wlog( 1, "  NOT using variable timestep!\n" );
  }

  wlog( 1, "\n" );
}

/*
 * ============================================================================
 * ============================================================================
 */

int main( int argc, char *argv[] ) {
  space_t s;

  memset( &s, 0, sizeof(space_t) );

  open_logfile_stdout();

  parse_opts( argc, argv );

  init_stuff( &s );

  if ( verbose ) {
    print_general_information( &s );
    wlog( 1, "Computing transitions...\n" );
  }

  if ( flag_plot_vf ) {
    plot_vf( &s, in_fn, out_fn );

  } else if ( flag_plot_part ) {
    plot_partitioning( &s, in_fn, out_fn );

  } else {
    compute_transitions( &s, out_fn );
  }

  if ( verbose ) {
    wlog( 1, "\nDone!\n" );
  }

  return 0;
}
