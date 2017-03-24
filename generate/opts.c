
#include "opts.h"

void parse_opts( int argc, char *argv[] ) {
  int copt, tmpint, ddind;
  float tmp_float;
  char *opt, *optarg;

  // we can safely skip the 0 argument
  for ( copt=1; copt<argc; copt++ ) {

    opt = argv[copt];
    if ( strncmp( opt, "--", 2 ) != 0 ) {
      fprintf( stderr, "don't understand option %s!\n", opt );
      continue;
    }
    opt += 2;

    optarg = index( opt, '=' );

    if ( optarg != NULL ) {
      // they've specified something like "--verbose=1"
      optarg++;
    } else {
      if ( copt+1 > argc-1 ) {
	// they've specified something like "--verbose", and there is
	// no next argument
	optarg = NULL;
      } else {
	if ( strncmp( argv[copt+1], "--", 2 ) == 0 ) {
	  // they've specified something like "--verbose", and the
	  // next argument looks like a different argument
	  optarg = NULL;
	} else {
	  // the next argument is a parameter to this argument
	  optarg = argv[copt+1];
	}
      }
    }

    if ( strncmp( opt, "discount_factor",
			 strlen("discount_factor") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing discount_factor value\n" );
      } else {
	tmp_float = atof( optarg );
	if ( tmp_float < 0 || tmp_float >= 1.0 ) {
	  fprintf( stderr, "invalid discount_factor value %s (should be 0 <= gamma < 1.0)\n", optarg );
	} else {
	  discount_factor = tmp_float;
	}
      }

    } else if ( strncmp( opt, "base_timestep",
			 strlen("base_timestep") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing base_timestep value\n" );
      } else {
	tmp_float = atof( optarg );
	if ( tmp_float <= 0 ) {
	  fprintf( stderr, "invalid base_timestep value %s (should be > 0)\n", optarg );
	} else {
	  base_timestep = tmp_float;
	}
      }

    } else if ( strncmp( opt, "plotvf",
			 strlen("plotvf") ) == 0 ) {
      flag_plot_vf = 1;


    } else if ( strncmp( opt, "plotp",
			 strlen("plotp") ) == 0 ) {
      flag_plot_part = 1;

    } else if ( strncmp( opt, "use_variable_timestep",
			 strlen("use_variable_timestep") ) == 0 ) {

      if ( optarg == NULL) {
	use_variable_timestep = VT_YES;
      } else if ( *optarg == 'y' || *optarg == '1' ) {
	use_variable_timestep = VT_YES;
      } else if ( *optarg == 'n' || *optarg == '0' ) {
	use_variable_timestep = VT_NO;
      } else {
	fprintf( stderr, "invalid use_variable_timestep value %s (should be <yes|no>)\n", optarg );
      }

    } else if ( strncmp( opt, "verbose", strlen("verbose") ) == 0 ) {
      if ( optarg == NULL) {
	verbose = 1;
      } else if ( *optarg == 'y' || *optarg == '1' ) {
	verbose = 1;
      } else if ( *optarg == 'n' || *optarg == '0' ) {
	verbose = 0;
      } else {
	fprintf( stderr, "invalid verbose value %s (should be <yes|no>)\n", optarg );
      }

    } else if ( strncmp( opt, "out", strlen("out") ) == 0 ) {
      if ( optarg == NULL) {
	out_fn = NULL;
      } else {
	out_fn = strdup( optarg );
      }

    } else if ( strncmp( opt, "in", strlen("in") ) == 0 ) {
      if ( optarg == NULL) {
	in_fn = NULL;
      } else {
	in_fn = strdup( optarg );
      }

    } else if ( sscanf( opt, "dd%d", &ddind) == 1 ) {
      if ( optarg == NULL) {
	fprintf( stderr, "invalid dd option\n" );
      } else {
	tmpint = atoi( optarg );

	if ( tmpint < 0 ) {
	  fprintf( stderr, "invalid value for dd%d: %d\n", ddind, tmpint );

	} else {

	  if ( ddind < MAX_DD && ddind >= 0 ) {
	    dd[ddind] = tmpint;
	  } else {
	    fprintf( stderr, "invalid dimension for dd: %d\n", ddind );
	  }
	}
      }

    } else {
	fprintf( stderr, "don't understand option %s!\n", opt );
    }
  }
}

void show_opts( int argc, char *argv[] ) {
  int i;

  for ( i=0; i<argc; i++ ) {
    fprintf( stderr, "%d: [%s]\n", i, argv[i] );
  }
}

