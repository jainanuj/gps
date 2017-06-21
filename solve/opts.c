
#include "opts.h"

void parse_opts( int argc, char *argv[] ) {
  int copt;
  int tmp_int;
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

    if ( strncmp( opt, "heat_metric", strlen("heat_metric") ) == 0 ) {
      if ( optarg == NULL ) {
	heat_metric = HEAT_STD;
      } else if ( strcmp( optarg, "std" ) == 0 ) {
	heat_metric = HEAT_STD;
      } else if ( strcmp( optarg, "abs" ) == 0 ) {
	heat_metric = HEAT_ABS;
      } else {
	fprintf( stderr, "invalid heat_metric value %s (should be <std|abs>)\n", optarg );
	exit( 0 );
      }

    } else if ( strncmp( opt, "mdp_fn", strlen("mdp_fn") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing mdp_fn value" );
	exit( 0 );
      } else {
	mdp_fn = strdup( optarg );
      }

    } else if ( strncmp( opt, "stat_fn", strlen("stat_fn") ) == 0 ) {
        if ( optarg == NULL ) {
            fprintf( stderr, "missing stat_fn value" );
            exit( 0 );
        } else {
            stat_fn = strdup( optarg );
        }
        
    } else if ( strncmp( opt, "ptp_fn", strlen("ptp_fn") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing ptp_fn value" );
	exit( 0 );
      } else {
	part_to_proc_fn = strdup( optarg );
      }

    } else if ( strncmp( opt, "stp_fn", strlen("stp_fn") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing stp_fn value" );
	exit( 0 );
      } else {
	state_to_part_fn = strdup( optarg );
      }

    } else if ( strncmp( opt, "sub_parts_st", strlen("sub_parts_st") ) == 0 ) {
        if ( optarg == NULL ) {
            fprintf( stderr, "missing sub_parts_st value" );
            exit( 0 );
        } else {
                part_to_sub_part_fn = strdup( optarg );
            }
    }
    else if ( strncmp( opt, "save_fn", strlen("save_fn") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing save_fn value" );
	exit( 0 );
      } else {
	save_fn = strdup( optarg );
      }

    } else if ( strncmp( opt, "run_type", strlen("run_type") ) == 0 ) {

      if ( optarg == NULL ) {
	run_type = RUN_PVI;
      } else if ( strcmp( optarg, "vi" ) == 0 ) {
	run_type = RUN_VI;
      } else if ( strcmp( optarg, "pvi" ) == 0 ) {
	run_type = RUN_PVI;
      } else if ( strcmp( optarg, "pi" ) == 0 ) {
	run_type = RUN_PI;
      } else if ( strcmp( optarg, "ppi" ) == 0 ) {
	run_type = RUN_PPI;
      } else {
	fprintf( stderr, "invalid run_type value %s (should be <vi|pvi|pi|ppi>)\n", optarg );
	exit( 0 );
      }

    } else if ( strncmp( opt, "epsilon", strlen("epsilon") ) == 0 ) {

      if ( optarg == NULL ) {
	fprintf( stderr, "missing epsilon value\n" );
	exit( 0 );
      } else {
	tmp_float = atof( optarg );
	if ( tmp_float <= 0 ) {
	  fprintf( stderr, "invalid epsilon value %s (should be > 0)\n", optarg );
	  exit( 0 );
	} else {
	  heat_epsilon = tmp_float;
	}
      }

    } else if ( strncmp( opt, "use_voting", strlen("use_voting") ) == 0 ) {

      if ( optarg == NULL) {
	use_voting = VOTE_YES;
      } else if ( *optarg == 'y' || *optarg == '1' ) {
	use_voting = VOTE_YES;
      } else if ( *optarg == 'n' || *optarg == '0' ) {
	use_voting = VOTE_NO;
      } else {
	fprintf( stderr, "invalid use_voting value %s (should be <yes|no>)\n", optarg );
	exit( 0 );
      }


    } else if ( strncmp( opt, "make_movie", strlen("make_movie") ) == 0 ) {

      if ( optarg == NULL) {
	make_movie = MM_YES;
      } else if ( *optarg == 'y' || *optarg == '1' ) {
	make_movie = MM_YES;
      } else if ( *optarg == 'n' || *optarg == '0' ) {
	make_movie = MM_NO;
      } else {
	fprintf( stderr, "invalid make_movie value %s (should be <yes|no>)\n", optarg );
	exit( 0 );
      }

    } else if ( strncmp( opt, "every_nth_frame",
			 strlen("every_nth_frame") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing every_nth_frame value\n" );
	exit( 0 );
      } else {
	tmp_int = atoi( optarg );
	if ( tmp_int <= 0 ) {
	  fprintf( stderr, "invalid every_nth_frame value %s (should be > 0)\n", optarg );
	  exit( 0 );
	} else {
	  every_nth_frame = tmp_int;
	}
      }


    } else if ( strncmp( opt, "movie_format", strlen("movie_format") ) == 0 ) {

      if ( optarg == NULL ) {
	fprintf( stderr, "missing movie_format value" );
	exit( 0 );
      } else {
	movie_format = strdup( optarg );
      }

    } else if ( strncmp( opt, "do_sanity_checks",
			 strlen("do_sanity_checks") ) == 0 ) {

      if ( optarg == NULL) {
	do_sanity_checks = 1;
      } else if ( *optarg == 'y' || *optarg == '1' ) {
	do_sanity_checks = 1;
      } else if ( *optarg == 'n' || *optarg == '0' ) {
	do_sanity_checks = 0;
      } else {
	fprintf( stderr, "invalid do_sanity_checks value %s (should be <yes|no>)\n", optarg );
	exit( 0 );
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
	exit( 0 );
      }

    } else if ( strncmp( opt, "echo_string", strlen("echo_string") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing echo_string value" );
	exit( 0 );
      } else {
	echo_string = strdup( optarg );
      }

    } else if ( strncmp( opt, "num_attractors",
                         strlen("num_attractors") ) == 0 ) {
      if ( optarg == NULL ) {
        fprintf( stderr, "missing num_attractors value\n" );
	exit( 0 );
      } else {
        tmp_int = atoi( optarg );
        if ( tmp_int < 0 ) {
          fprintf( stderr, "invalid num_attractors value %s (should be > 0)\n", optarg );
	  exit( 0 );
        } else {
          num_attractors = tmp_int;
        }
      }


    } else if ( strncmp( opt, "odcd_cache_size",
                         strlen("odcd_cache_size") ) == 0 ) {
      if ( optarg == NULL ) {
        fprintf( stderr, "missing odcd_cache_size value\n" );
	exit( 0 );
      } else {
        tmp_int = atoi( optarg );
	/* odcd_cache_size == -1 means to disable odcd_caching */
        if ( tmp_int <= -2 ) {
          fprintf( stderr, "invalid odcd_cache_size value %s (should be > -2)\n", optarg );
	  exit( 0 );
        } else {
          odcd_cache_size = tmp_int;
        }
      }


    } else if ( strncmp( opt, "odcd_cache_fn_format",
			 strlen("odcd_cache_fn_format") ) == 0 ) {
      if ( optarg == NULL ) {
	fprintf( stderr, "missing odcd_cache_fn_format value" );
	exit( 0 );
      } else {
	odcd_cache_fn_format = strdup( optarg );
      }


    } else if ( strncmp( opt, "solver",
                         strlen("solver") ) == 0 ) {
      if ( optarg == NULL ) {
        fprintf( stderr, "missing solver value\n" );
	exit( 0 );
      } else {

	if ( strcasecmp( optarg, "r" ) == 0 ) {
	  solver = RICHARDSON;
	} else if ( strcasecmp( optarg, "sd" ) == 0 ) {
	  solver = STEEPEST_DESCENT;
	} else if ( strcasecmp( optarg, "mr" ) == 0 ) {
	  solver = MINIMUM_RESIDUAL;
	} else if ( strcasecmp( optarg, "rnsd" ) == 0 ) {
	  solver = RESIDUAL_NORM_SD;
	} else if ( strcasecmp( optarg, "gmres" ) == 0 ) {
	  solver = GMRES;
	} else if ( strcasecmp( optarg, "ge" ) == 0 ) {
	  solver = GAUSSIAN_ELIM;
	} else if ( strcasecmp( optarg, "cg" ) == 0 ) {
	  solver = CONJUGATE_GRAD;
	} else if ( strcasecmp( optarg, "cgs" ) == 0 ) {
	  solver = CONJUGATE_GRAD_SQ;
	} else if ( strcasecmp( optarg, "cgnr" ) == 0 ) {
	  solver = CONJUGATE_GRAD_NR;
	} else if ( strcasecmp( optarg, "cgne" ) == 0 ) {
	  solver = CONJUGATE_GRAD_NE;

#ifdef USE_AZTEC
	} else if ( strcasecmp( optarg, "az_gmres" ) == 0 ) {
	  solver = AZ_GMRES;
	} else if ( strcasecmp( optarg, "az_cg" ) == 0 ) {
	  solver = AZ_CONJUGATE_GRAD;
	} else if ( strcasecmp( optarg, "az_cgs" ) == 0 ) {
	  solver = AZ_CONJUGATE_GRAD_SQ;
	} else if ( strcasecmp( optarg, "az_tfqmr" ) == 0 ) {
	  solver = AZ_TFQMR;
	} else if ( strcasecmp( optarg, "az_bicgstab" ) == 0 ) {
	  solver = AZ_BICGSTAB;
	} else if ( strcasecmp( optarg, "az_lu" ) == 0 ) {
	  solver = AZ_LU;
#endif

	} else {
	  fprintf( stderr, "invalid solver value!\n" );
	  exit( 0 );
	}

      }

    } else {
      fprintf( stderr, "don't understand option %s!\n", opt );
      exit( 0 );
    }
  }
}

void show_opts( int argc, char *argv[] ) {
  int i;

  for ( i=0; i<argc; i++ ) {
    fprintf( stderr, "%d: [%s]\n", i, argv[i] );
  }
}

