
#include "bfuncs.h"

#ifdef PREC_IS_FLOAT
#ifdef USE_AZTEC
#error Hey! Aztec has to use double precision!
#endif
#endif


extern double when( void );

/*
 * ----------------------------------------------------------------------------
 */

void peek_at_mdp( world_t *w, char *fn ) {
  FILE *fp;
  int r;

  fp = fopen( fn, "rb" );
  if ( fp == NULL ) {
    wlog( 1, "Couldn't open %s!\n", fn );
    exit( 0 );
  }

  r = fscanf( fp, "%d\n", &( w->num_global_states ) );
  if ( r != 1 ) {
    wlog( 1, "Error reading number of states!\n" );
    exit( 0 );
  }

  fclose( fp );
}

void load_state_to_part( world_t *w, char *fn ) {
  FILE *fp;
  int i, max_part;

  if ( fn == NULL ) {
    w->state_to_partnum = NULL;
    w->num_global_parts = 1;
    return;
  }

  fp = fopen( fn, "rb" );       //.part file. - Anuj
  if ( fp == NULL ) {
    wlog( 1 , "Error opening file %s!\n", fn );
    exit( 0 );
  }

  reset_tokenizer();

  w->state_to_partnum = (int *)malloc( sizeof(int) * w->num_global_states );
  if ( w->state_to_partnum == NULL ) {
    wlog( 1, "Error allocating state_to_partnum!\n" );
    exit( 0 );
  }
    w->size_state_to_partnum = sizeof(int) * w->num_global_states;

  max_part = 0;
  for ( i=0; i<w->num_global_states; i++ ) {
    get_token( TOKEN_INT, fp, &( w->state_to_partnum[i] ) );
    /* fscanf( fp, "%d", &( w->state_to_partnum[i] ) ); */
    if ( w->state_to_partnum[i] > max_part ) {
      max_part = w->state_to_partnum[i];
    }
  }

  w->num_global_parts = max_part + 1;

  fclose( fp );
}


void load_states_to_sub_part( world_t *w, char *fn)
{
    FILE *fp;
    int i, max_part, j, cnt;
    int sub_part;
    struct stat st_buf;
    int status;
    
    w->size_parts[6] = 0;
    if ( fn == NULL )
    {
        w->state_to_subpartnum = NULL;
        return;
    }
    status = stat (fn, &st_buf);
    if ( (status != 0) || (S_ISDIR (st_buf.st_mode)) )
    {
        wlog (1, "%s is a directory.\n", fn);
        w->state_to_subpartnum = NULL;
        return;
    }
    fp = fopen( fn, "rb" );       //.sub-part file. - Anuj
    if ( fp == NULL )
    {
        w->state_to_subpartnum = NULL;
        return;
    }
    
    reset_tokenizer();
    
    w->state_to_subpartnum = (int *)malloc( sizeof(int) * w->parts[0].num_states );
    w->size_parts[6] += sizeof(int) * w->parts[0].num_states;
    

    if ( w->state_to_subpartnum == NULL )
    {
        wlog( 1, "Error allocating state_to_partnum!\n" );
        exit( 0 );
    }
    
    max_part = 0;

    //Assuming all parts have same number of states as part 0.
    for ( i=0; i<w->parts[0].num_states; i++ )
    {
        get_token( TOKEN_INT, fp, &( w->state_to_subpartnum[i] ) );
        if ( w->state_to_subpartnum[i] > max_part )
        {
            max_part = w->state_to_subpartnum[i];
        }
    }
    fclose( fp );

    for (i = 0; i< w->num_local_parts; i++)
    {
        w->parts[i].num_sub_parts = max_part + 1;
        w->parts[i].sub_parts = (sub_part_t *)malloc(sizeof(sub_part_t) * w->parts[i].num_sub_parts);
        
        w->parts[i].size_sub_parts = sizeof(sub_part_t) * w->parts[i].num_sub_parts;
        w->size_parts[6] += sizeof(sub_part_t) * w->parts[i].num_sub_parts;
        
        memset(w->parts[i].sub_parts, 0, sizeof(sub_part_t) * w->parts[i].num_sub_parts);
    }

    for ( i=0; i<w->parts[0].num_states; i++ )
    {
        sub_part = w->state_to_subpartnum[i];
        w->parts[ 0 ].sub_parts[sub_part].num_states_sub++;
    }
    cnt = w->parts[ 0 ].sub_parts[0].num_states_sub;
    
    for (i = 0; i < w->num_local_parts; i++)
    {
        for (j = 0; j < w->parts[i].num_sub_parts; j++)
        {
            w->parts[i].sub_parts[j].num_states_sub = 0;
            w->parts[i].sub_parts[j].sub_parts_states = (int *)malloc(sizeof(int) * cnt);

            w->parts[i].size_sub_parts += sizeof(int) * cnt;
            w->size_parts[6] += sizeof(int) * cnt;
        }
        for ( j=0; j<w->parts[i].num_states; j++ )
        {
            sub_part = w->state_to_subpartnum[j];
            w->parts[i].sub_parts[sub_part].sub_parts_states[
                                                             w->parts[i].sub_parts[sub_part].num_states_sub++
                                                             ] = j;
        }
    }
}

void load_part_to_proc( world_t *w, char *fn ) {
  /*

this function does two things: first, it decides how to assign
partitions to processors, and second, it creates a mapping from global
partition numbers to the local storage of partition information.

So, for example, global partition 42 might be assigned to us, and
stored in bin 4 of the the local_parts array.

We currently use a random assignment.  This works because we
initialize to the same random seed for every processor, so we end up
generating the same random sequence.  Alternatively, the framework is
in place to be able to load the mapping from a file.

  */

#ifdef USE_MPI
  int N;

  N = w->num_global_parts;
  w->part_to_proc = (part_proc_t *)malloc( N * sizeof(part_proc_t) );
  if ( w->part_to_proc == NULL ) {
    wlog( 1, "Error allocating w->part_to_proc!\n" );
    exit( 0 );
  }

  randomly_assign_partitions( w, &( w->num_local_parts ) );

#else
  wlog( 1, "This is an MPI-only function!\n" );
  exit( 0 );
#endif
}

void load_mdp( world_t *w, char *fn ) {
  FILE *fp;
  int line_no;
  int i, j, a, g_state, l_state, g_part, l_part;
  int nacts, ndeps, tmpstates;
  int beg_dep, end_dep;
  int sprime;
  entry_t *et;
  trans_t *tt;
  prec_t *external_deps;
  state_t *st;
  prec_t total_prob, prob;
    
    int size_st_delta = 0;
#ifdef USE_MPI
  prec_t reward;
#endif
    val_t*** external_state_vals;

  fp = fopen( fn, "rb" );
  if ( fp == NULL ) {
    wlog( 1 , "Error opening file %s!\n", fn );
    exit( 0 );
  }

  /* check to make sure the MDP is compatible with the state_to_part
     mapping! */

  reset_tokenizer();

  if ( !get_token( TOKEN_INT, fp, &tmpstates ) ) {
    wlog( 1, "Error reading nstates!\n" );
/*   } else { */
/*     fprintf( stderr, "Nstates: %d\n", tmpstates ); */
  }

  if ( tmpstates != w->num_global_states ) {
    wlog( 1, "State mismatch: %d vs. %d!\n", tmpstates, w->num_global_states );
    exit( 0 );
  }

  w->gsi_to_lsi = (int *)malloc( sizeof(int) * tmpstates );
  if ( w->gsi_to_lsi == NULL ) {
    wlog( 1, "Error allocating gsi_to_lsi!\n" );
      w->size_gsi_to_lsi = 0;
    exit( 0 );
  }
    w->size_gsi_to_lsi = sizeof(int) * tmpstates;

  line_no = 1;

  for ( i=0; i<w->num_global_states; i++ ) {

    if ( !get_token( TOKEN_INT, fp, &g_state ) ||
	 !get_token( TOKEN_INT, fp, &nacts ) ) {
      fprintf( stderr, "Error reading state/nacts!\n" );
/*     } else { */
/*       fprintf( stderr, "State/nacts: %d/%d\n", g_state, nacts ); */
    }

    line_no++;

    g_part = state_to_partnum( w, g_state );

#ifdef USE_MPI
    /* if this isn't a local state, we don't care about it.
       just skip it. */
    if ( !is_partition_local( w, g_part ) ) {
      /* eat the rest of the line */
      for ( a=0; a<nacts; a++ ) {
	line_no++;

	if ( !get_token( TOKEN_FLOAT, fp, &reward ) ||
	     !get_token( TOKEN_INT, fp, &ndeps ) ) {
	  wlog( 1, "Error reading reward/ndeps!\n" );
	}
	for ( j=0; j<ndeps; j++ ) {
	  if ( !get_token( TOKEN_INT, fp, &sprime ) ||
	       !get_token( TOKEN_FLOAT, fp, &prob ) ) {
	    wlog( 1, "Error reading col/entry!\n" );
	  }
	}
      }
      continue;
    }
#endif

    l_part = gpi_to_lpi( w, g_part );   //global partition number to local partition number - ANUJ (relevant in MPI)

    l_state = w->parts[ l_part ].cur_local_state;
    w->parts[ l_part ].cur_local_state++;

    w->gsi_to_lsi[ g_state ] = l_state;

/*     fprintf( stderr, "Loading gs=%d ls=%d gp=%d lp=%d na=%d\n", */
/* 	     g_state, l_state, g_part, l_part, nacts ); */
/*     fprintf( stderr, "pos = %ld\n", ftell( fp ) ); */

    st = &( w->parts[ l_part ].states[ l_state ] );

    /* allocate the transitions that this state can take */
    /* "tps" stands for "transition probabilities" */
    tt = (trans_t *)malloc( sizeof(trans_t) * nacts );
    if ( tt == NULL ) {
      wlog( 1 , "Error allocating transition!\n" );
      exit( 0 );
    }
      
    //allocate external vals cache for each action.
    //This is aggregate impact of all external states corresponding to this action.
    //It remains constant for this action after first iteration through partition while evaluating the partition.
    external_deps = (prec_t *)malloc(sizeof(prec_t) * nacts);
    if ( external_deps == NULL ) {
      wlog( 1 , "Error allocating external_deps cache!\n" );
      exit( 0 );
    }
    
    external_state_vals = (val_t ***)malloc(sizeof(val_t **) * nacts);

    /* set up this state's information */
    st->tps = tt;
      st->external_dep_vals = external_deps;
      st->external_state_vals = external_state_vals;

      st->size_tps = sizeof(trans_t) * nacts;           //Tracking memory for hte state.
      st->size_external_state_vals = sizeof(val_t **) * nacts;         //Tracking memory for the state.
      st->size_external_dep_vals = sizeof(prec_t) * nacts;      //Tracking memory for the state.

      st->num_actions = nacts;
    st->global_state_index = g_state;

    /* read in the rewards and transition probabilities */
    for ( a=0; a<nacts; a++ ) {
      line_no++;

#ifdef PREC_IS_FLOAT
      if ( !get_token( TOKEN_FLOAT, fp, &( st->tps[a].reward ) ) ||
	   !get_token( TOKEN_INT, fp, &ndeps ) ) {
	wlog( 1, "Error reading reward/ndeps!\n" );
      }
#else
      if ( !get_token( TOKEN_DOUBLE, fp, &( st->tps[a].reward ) ) ||
	   !get_token( TOKEN_INT, fp, &ndeps ) ) {
	wlog( 1, "Error reading reward/ndeps!\n" );
      }
#endif

      if ( ndeps == 0 ) {
	st->tps[ a ].entries = NULL;
	st->tps[ a ].int_deps = 0;
	st->tps[ a ].ext_deps = 0;
	continue;
      }

/*       fprintf( stderr, "  %d: %.2f %d\n", a, st->tps[a].reward, ndeps ); */

      et = (entry_t *)malloc( sizeof(entry_t) * ndeps );
      if ( et == NULL ) {
	wlog( 1 , "Error allocating entry!\n" );
	exit( 0 );
      }

      st->tps[ a ].entries = et;
        
        st->size_tps += sizeof(entry_t) * ndeps;

      total_prob = 0;
      end_dep = ndeps-1;
      beg_dep = 0;

      for ( j=0; j<ndeps; j++ ) {

#ifdef PREC_IS_FLOAT
	if ( !get_token( TOKEN_INT, fp, &sprime ) ||
	     !get_token( TOKEN_FLOAT, fp, &prob ) ) {
	  wlog( 1, "Error reading sprime/prob!\n" );
	}
#else
	if ( !get_token( TOKEN_INT, fp, &sprime ) ||
	     !get_token( TOKEN_DOUBLE, fp, &prob ) ) {
	  wlog( 1, "Error reading sprime/prob!\n" );
	}
#endif

	total_prob += prob;

	if ( state_to_partnum( w, sprime ) != g_part ) {
	  /* ok.  this state depends on sprime, which is not inside of our
	     partition.  That's an external dep */
	  et[end_dep].col = sprime;
	  et[end_dep].entry = prob;
	  end_dep--;

	} else {
	  /* internal dependency */
	  et[beg_dep].col = sprime;
	  et[beg_dep].entry = prob;
	  beg_dep++;
	}

/*         fprintf( stderr, "    %d %d %.2f\n", j, et[j].col, et[j].entry ); */
      }

      if ( beg_dep != end_dep + 1 ) {
	wlog( 1, "HEY: beg (%d) != end+1 (%d)\n", beg_dep, end_dep+1 );
      }

      st->tps[ a ].int_deps = beg_dep;
      st->tps[ a ].ext_deps = ndeps - beg_dep;

      if ( total_prob >= 1.0 || total_prob < 0 ) {
	fprintf( stderr, " QUESTIONABLE PROBABILITY (line %d): %.5f\n",
		 line_no, total_prob );
      }
        
        if (st->tps[ a ].ext_deps > 0)
            st->external_state_vals[a] = (val_t **)malloc(sizeof(val_t *) * st->tps[ a ].ext_deps);
        else
            st->external_state_vals[a] = NULL;
        
        st->size_external_state_vals += sizeof(val_t *) * st->tps[ a ].ext_deps;
        
    }       //End of all actions for a state.
      size_st_delta = 0;
      size_st_delta += st->size_external_state_vals + st->size_tps + st->size_external_dep_vals;
      
      w->parts[l_part].size_states += size_st_delta;
      w->size_parts[1] += size_st_delta;
  }         //End of the states.

}

/*
 * ----------------------------------------------------------------------------
 */

void acquire_state_to_part( world_t *w ) {
  load_state_to_part( w, state_to_part_fn );
}

void sub_divide_parts (world_t *w)
{
    load_states_to_sub_part(w, part_to_sub_part_fn);
}


void acquire_part_to_proc( world_t *w ) {

#ifdef USE_MPI
  load_part_to_proc( w, part_to_proc_fn );
#else
  w->num_local_parts = w->num_global_parts;
  w->num_local_states = w->num_global_states;
#endif

}

/*
 * ----------------------------------------------------------------------------
 */

void count_states_in_parts( world_t *w ) {
  int i, g_part, l_part;

  for ( i=0; i<w->num_global_states; i++ ) {
    g_part = state_to_partnum( w, i );
    if ( is_partition_local( w, g_part ) ) {
      l_part = gpi_to_lpi( w, g_part );
      w->parts[ l_part ].num_states++;
    }
  }
}

void allocate_states_in_parts( world_t *w ) {
  int l_part, cnt;

  for ( l_part=0; l_part < w->num_local_parts; l_part++ ) {
    cnt = w->parts[ l_part ].num_states;

    w->parts[ l_part ].states = (state_t *)malloc( sizeof(state_t)*cnt );
    if ( w->parts[ l_part ].states == NULL ) {
      wlog( 1, "Error allocating states!\n" );
      exit( 0 );
    }
    memset( w->parts[ l_part ].states, 0, sizeof(state_t)*cnt );
      
      w->parts[l_part].size_states = sizeof(state_t)*cnt;
      
      w->size_parts[1] += w->parts[l_part].size_states;
  }
}

/*
 * ----------------------------------------------------------------------------
 */

int init_world( world_t *w, char *fn ) {
  char odcd_fn[256];
    int i;

  /* figure out how many states we're expecting */
  peek_at_mdp( w, mdp_fn );

  /* this is the mapping from states to partitions. */
  /* every processor has a full copy. */
  acquire_state_to_part( w );

  /* this is the mapping from partitions to processors. */
  /* again, every processor has a full copy. */
  acquire_part_to_proc( w );

  if ( verbose ) {
    wlog( 1, "GLOBALLY: %d PARTITIONS, %d STATES!\n",
	  w->num_global_parts, w->num_global_states );
    wlog( 1, " LOCALLY: %d PARTITIONS, %d STATES!\n",
	  w->num_local_parts, w->num_local_states );
  }

  /* allocate partitions */

  w->parts = (part_t *)malloc( sizeof(part_t) *
			       w->num_local_parts );
  if ( w->parts == NULL ) {
    wlog( 1, "Could not allocate partition array!\n" );
    exit( 0 );
  }
  memset( w->parts, 0, sizeof(part_t) * w->num_local_parts );
    
    for (i = 0; i< 7; i++)
        w->size_parts[i] = 0;
    
//    memset(w->size_parts, 0, sizeof(w->size_parts) )
    w->size_parts[0] = sizeof(part_t) * w->num_local_parts;

  count_states_in_parts( w );

  /* allocate the states in each partition */
  allocate_states_in_parts( w );
    
    sub_divide_parts(w);

  w->part_queue = queue_create(w->num_local_parts);
  if ( w->part_queue == NULL ) {
        wlog( 1, "Error creating queue!\n" );
        exit( 0 );
    }

    w->size_part_queue = sizeof(queue);
    w->size_part_queue += w->num_local_parts * sizeof(int);
    
    //Create a queue for each partition to process the sub-parts in that part.
    if (w->state_to_subpartnum != NULL)
    {
        for (i = 0; i < w->num_local_parts; i++)
        {
            w->parts[i].sub_part_queue = queue_create(w->parts[i].num_sub_parts);
            if (w->parts[i].sub_part_queue == NULL )
            {
                wlog( 1, "Error creating sub-queue!\n" );
                exit( 0 );
            }
            w->size_parts[6] += sizeof(queue);
            w->size_parts[6] += w->parts[i].num_sub_parts * sizeof(int);
            w->parts[i].size_sub_parts += sizeof(queue) + w->parts[i].num_sub_parts * sizeof(int);
        }
    }
    
  /* we need to initialize the foreign_state_val_hash */
#ifdef USE_MPI
  w->foreign_state_val_hash = med_hash_create( 4096 );
  if ( w->foreign_state_val_hash == NULL ) {
    wlog( 1, "Error allocating foreign_state_val_hash!\n" );
    exit( 0 );
  }
#endif

  /* Miscellaneous initializations */

//  misc_mpi_initialization( w );

/*  if ( odcd_enabled() ) {
    sprintf( odcd_fn, odcd_cache_fn_format, getpid() );
    odcd_cache_init( &(w->odcd_cache), odcd_cache_size, odcd_fn );
  }
*/
  if ( verbose ) {
    wlog( 1, "  Done initializing partitions.\n" );
  }

  w->parts_processed = 0;

  /* Everything should be allocated and ready to go.  Load the MDP! */
  load_mdp( w, mdp_fn );


  initialize_partitions( w );
    if (w->state_to_subpartnum != NULL)
        initialize_sub_partitions(w);
//  w->reward_or_value_updatetime = 0.0;

  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void misc_mpi_initialization( world_t *w ) {

#ifdef USE_MPI
  /* this hash maps partitions that are on foreign processors
     to local partitions.  It helps us when processing incoming messages
  */
  w->endpart_to_startpart = med_hash_create( 4 );

  /* This one is a temporary hash that tells us what foreign states
     we depend on, and what processors they're on.
  */
  w->fproc_data = med_hash_create( 4 );

  if ( w->endpart_to_startpart == NULL ||
       w->fproc_data == NULL ) {
    wlog( 1, "Error allocating MPI hashes!\n" );
    exit( 0 );
  }

  w->mpi_reqs = (mpi_req_stuff *)malloc( sizeof(mpi_req_stuff) *
					 w->num_procs );

  if ( w->mpi_reqs == NULL ) {
    wlog( 1, "Error allocating mpi wait structures!\n" );
    exit( 0 );
  }

  memset( w->mpi_reqs, 0, sizeof(mpi_req_stuff)*w->num_procs );

  w->terminate = 0;
  w->processing_done = 0;
  w->processing_done_cnt = 0;

  w->term_ping = 0;
  w->term_state = 0;

  w->ltfsd_done = 0;
  w->ltfsd_done_cnt = 0;

  w->messages_sent = 0;
  w->message_size = 0;

  /* compute the g_part_num field of the parts array.  Indices in our
     local parts array no longer correspond to partition numbers.
     Figure out what's the right mapping. */

  int g_part_num, l_part_num;
  for ( g_part_num=0,l_part_num=0; g_part_num<w->num_global_parts; g_part_num++ ) {
    if ( is_partition_local( w, g_part_num ) ) {
      w->parts[ l_part_num ].g_part_num = g_part_num;
      l_part_num++;
    }
  }

#endif

}


/*
 * ----------------------------------------------------------------------------
 */

/* a and b need to be LOCAL partition indices! */

int part_cmp_func( int lp_a, int lp_b, void *vw ) {
  prec_t a_heat, b_heat;

  /*  wlog( 1, "  comparing %d to %d: %.2f %.2f\n", */
  /*	   a, b, w->parts[a].heat, w->parts[b].heat ); */

  a_heat = ((world_t *)vw)->parts[ lp_a ].heat;
  b_heat = ((world_t *)vw)->parts[ lp_b ].heat;

  return a_heat > b_heat;

/*   if ( a_heat < b_heat ) { */
/*     return 1; */
/*   } else if ( a_heat > b_heat ) { */
/*     return -1; */
/*   } */
/*   return 0; */
}

void part_swap_func( int lp_a, int lp_b, void *vw ) {
  int tmp;
  world_t *w;

  w = (world_t *)vw;
  tmp = w->parts[ lp_a ].my_heap_num;
  w->parts[ lp_a ].my_heap_num = w->parts[ lp_b ].my_heap_num;
  w->parts[ lp_b ].my_heap_num = tmp;
}

void part_add_func( int lp_obj, int pos, void *vw ) {
  world_t *w;

  w = (world_t *)vw;
  w->parts[ lp_obj ].my_heap_num = pos;
}

void init_part_heap( world_t *w ) {
  int l_part_num;
  
  for ( l_part_num=0; l_part_num<w->num_local_parts; l_part_num++ ) {
    heap_add( w->part_heap, l_part_num );
  }
}


void init_part_queue( world_t *w )
{
    int l_part_num;
    
    for ( l_part_num=0; l_part_num<w->num_local_parts; l_part_num++ )
    {
        queue_add( w->part_queue, l_part_num );
    }
}

void init_sub_part_queue( world_t *w, int part )
{
    int l_sub_part_num;
    for ( l_sub_part_num=0; l_sub_part_num<w->parts[part].num_sub_parts; l_sub_part_num++ )
    {
        queue_add( w->parts[part].sub_part_queue, l_sub_part_num );
    }
}

/*
 * ----------------------------------------------------------------------------
 */

void compute_cross_partition_deps( world_t *w ) {
  int g_start_state, l_start_state, state_cnt, l_part, l_sub_part;
    int l_start_sub_part, l_end_sub_part;
  int l_start_part, g_start_part;
  int g_end_state, g_end_part, l_end_state;
  int action, s, dep_cnt;
  trans_t *t;
  state_t *st;
    
    
    int indexParthash =0, keyParthash =0;
    val_t *ptr_valparthash;
    med_hash_t *m2;
#ifdef USE_MPI
  int r;
#endif

  l_start_part = -1;
  g_start_part = -1;
  while ( iterate_over_parts_seq( w, &l_start_part, &g_start_part ) ) {

    part_check_in( w, l_start_part );

    state_cnt = w->parts[ l_start_part ].num_states;

    for ( l_start_state = 0; l_start_state < state_cnt; l_start_state++ ) {

      g_start_state = lsi_to_gsi( w, l_start_part, l_start_state );
      st = &( w->parts[ l_start_part ].states[ l_start_state ] );


#ifdef USE_MPI

      /* we need to make a place for this state. */
/*       fprintf( stderr, "Adding %d to svh (%d,%d)\n", */
/* 	       g_start_state, l_start_part, l_start_state ); */
#ifdef PREC_IS_FLOAT
      r = med_hash_add_float( w->foreign_state_val_hash, g_start_state,
			      DEFAULT_VALUE );
#else
      r = med_hash_add_double( w->foreign_state_val_hash, g_start_state,
			       DEFAULT_VALUE );
#endif
      assert( r == MH_ADD_OK || r == MH_ADD_REPLACED );

#endif

      for (action=0; action<st->num_actions; action++) {

	t = &( st->tps[ action ] );

	dep_cnt = t->int_deps + t->ext_deps;
	for ( s=0; s<dep_cnt; s++ ) {

	  /* right now, t->entries[ i ].col is a global state index. */
	  g_end_state = t->entries[ s ].col;

	  /* ok.  l_start_state depends on g_end_state.
	     where is g_end_state, and what do we need to do about it?
	     first, figure out what partition it's in. */
	  g_end_part = state_to_partnum( w, g_end_state );

	  /* we need to add end_state to our state value cache.  this
	     maps gsi's to prec_ts (representing the value of the
	     state) */
#ifdef USE_MPI

          if (!med_hash_check( w->foreign_state_val_hash, g_end_state )) {
#ifdef PREC_IS_FLOAT
              r = med_hash_add_float(
                      w->foreign_state_val_hash, g_end_state, DEFAULT_VALUE
                      );
#else
              r = med_hash_add_double(
                      w->foreign_state_val_hash, g_end_state, DEFAULT_VALUE
                      );
#endif

              assert( r == MH_ADD_OK || r == MH_ADD_REPLACED );
/* 	      fprintf( stderr, "  (adding %d to svh)\n", g_end_state ); */
          }

#endif

	  if ( g_start_part == g_end_part )
      {
          if (w->state_to_subpartnum != NULL)
          {
              l_end_state = gsi_to_lsi(w, g_end_state);
              l_start_sub_part = w->state_to_subpartnum[l_start_state];
              l_end_sub_part = w->state_to_subpartnum[l_end_state];
              if (l_start_sub_part != l_end_sub_part)
              {
                  med_hash_set_add( w->parts[ g_start_part ].sub_parts[l_end_sub_part].my_local_dependents,
                                   l_start_sub_part, l_start_state ); //l_start_state might need to be replaced by l_start_sub_state. TODO - Anuj
              }
          }
          continue;
      }

	  /* ok.  g_end_state isn't in our partition.
	     that means that g_end_part has an inverse link to us. */

	  add_dep( w,
		   g_start_state, l_start_state,
		   g_start_part, l_start_part,
		   g_end_state, g_end_part );
        
        
        //Also add the state to ext deps list of the partition.
        //Put this as a list in buckets of external partitions.
        
        //So every partition will mantain a hash for external partitions.
        //In each of the external partitions will be hash of states belonging to that partition this partition depends on.
        l_end_state = gsi_to_lsi(w, g_end_state);
        add_part_ext_dep_states(w, g_start_part, l_start_part, l_start_state, l_end_state, g_end_part);
	
    }       //End of each dependent for an action.
      }     //End of each action in a state
    }       //End of looping over each state in a partitoin
      
      
      w->parts[l_start_part].size_my_ext_parts = sizeof(med_hash_t);
      w->parts[l_start_part].size_my_ext_parts +=
                w->parts[l_start_part].my_ext_parts_states->nalloc * sizeof(datum_t);
      
      
      indexParthash = 0; keyParthash = 0;
      while(med_hash_iterate(w->parts[l_start_part].my_ext_parts_states, &indexParthash, &keyParthash, &ptr_valparthash))
      {
          m2 = ptr_valparthash->vptr;
          w->parts[l_start_part].size_my_ext_parts += m2->nalloc * sizeof(datum_t);
      }
      w->size_parts[3] += w->parts[l_start_part].size_my_ext_parts;
  }         //End of looping over each partition
    
    for (l_part = 0; l_part < w->num_local_parts; l_part++)
    {
        w->parts[l_part].size_my_local_deps = sizeof(med_hash_t);
        w->parts[l_part].size_my_local_deps += w->parts[l_part].my_local_dependents->nalloc
                                                        * sizeof(datum_t);
        w->size_parts[2] += w->parts[l_part].size_my_local_deps;
        if (w->state_to_subpartnum != NULL)
        {
            for (l_sub_part = 0; l_sub_part < w->parts[l_part].num_sub_parts; l_sub_part++)
            {
                w->parts[l_part].size_sub_parts += sizeof(med_hash_t);
                w->parts[l_part].size_sub_parts += w->parts[l_part].sub_parts[l_sub_part].my_local_dependents->nalloc
                                                * sizeof(datum_t);
                w->size_parts[6] += sizeof(med_hash_t) +
                                                w->parts[l_part].sub_parts[l_sub_part].my_local_dependents->nalloc
                                                * sizeof(datum_t);
            }
        }
    }
    
}           //End of Function


void cache_dependencies_in_states( world_t *w )
{
    int l_start_state, state_cnt;
    int l_start_part, g_start_part;
    int g_end_state, g_end_part, l_end_state;
    int action, s, dep_cnt;
    trans_t *t;
    state_t *st;

    l_start_part = -1;
    g_start_part = -1;
    while ( iterate_over_parts_seq( w, &l_start_part, &g_start_part ) )
    {
        state_cnt = w->parts[ l_start_part ].num_states;
        for ( l_start_state = 0; l_start_state < state_cnt; l_start_state++ )
        {
            st = &( w->parts[ l_start_part ].states[ l_start_state ] );
            for (action=0; action<st->num_actions; action++)
            {
                t = &( st->tps[ action ] );
                dep_cnt = t->ext_deps;
                for ( s=0; s<dep_cnt; s++ )
                {
                    g_end_state = t->entries[ s + t->int_deps ].col;
                    
                    /* ok.  l_start_state depends on g_end_state.
                     where is g_end_state, and what do we need to do about it?
                     first, figure out what partition it's in. */
                    g_end_part = state_to_partnum( w, g_end_state );
                    if ( g_start_part == g_end_part )
                    {
                        /* no further action required if it's in our partition. */
                        continue;
                    }
                    l_end_state = gsi_to_lsi(w, g_end_state);
                    add_cache_states(w, g_start_part, l_start_part, l_start_state, l_end_state, g_end_part,
                                            w->parts[l_start_part].states[l_start_state].external_state_vals[action], s);
                }
            }
        }
    }
    
}

/*
 * ----------------------------------------------------------------------------
 */

/*
# Select a vertex that has in-degree zero.
# Add the vertex to the sort.
# Delete the vertex and all the edges emanating from it from the graph.

maintain a list of zero-degree vertices.

Pick one, add it to the final order.
  is the list empty?
  find the minimal vertex entry.
subtract successors
  adding zero-degree vertices to the list.


*/

void reorder_states_within_partition( world_t *w, int l_part ) {
  part_t *pp;
  state_t *sp;
  int i, j, total, done_cnt, scnt, a;
  int *zero_deg_list, zd_cnt;
  int *deg_cnt, min_deg;
  int g_state, l_state, misses;
/*   int toadd; */

  pp = &( w->parts[ l_part ] );
  total = pp->num_states;

  /* allocate fun and interesting data structures */

  pp->variable_ordering = (int *)malloc( sizeof(int) * total );
  zero_deg_list = (int *)malloc( sizeof(int) * total );
  deg_cnt = (int *)malloc( sizeof(float) * total );
  if ( zero_deg_list == NULL || pp->variable_ordering == NULL || deg_cnt == NULL ) {
    wlog( 1, "Whoa! Couldn't allocate reordering data structures!\n" );
    exit( 0 );
  }
  memset( pp->variable_ordering, 0, sizeof(int)*total );
  memset( zero_deg_list, 0, sizeof(int)*total );
  memset( deg_cnt, 0, sizeof(int)*total );

  /* initialize the degree counts */
  /* these are in-degrees! */

  for ( i=0; i<total; i++ ) {
    sp = &( pp->states[ i ] );

    for ( a=0; a<sp->num_actions; a++ ) {
      scnt = sp->tps[a].int_deps;

      for ( j=0; j<scnt; j++ ) {
	g_state = sp->tps[a].entries[j].col;
	/* 'kay. local state i depends on global state g_state. */
/* 	toadd = (int)( (float)100.0 * sp->tps[a].entries[j].entry ); */
/* 	deg_cnt[ gsi_to_lsi( w, g_state ) ] += toadd; */
	deg_cnt[ gsi_to_lsi( w, g_state ) ] += 1;
      }
    }
  }

  /* initialize the zero-degree states */

  zd_cnt = 0;
  for ( i=0; i<total; i++ ) {
    if ( deg_cnt[i] == 0 ) {
      zero_deg_list[zd_cnt] = i;
      zd_cnt++;
    }
  }

  /* main loop */

  misses = 0;
  done_cnt = 0;
  while ( done_cnt < total ) {

    /* pick a state with zero degree */
    if ( zd_cnt > 0 ) {
      zd_cnt--;
      i = zero_deg_list[zd_cnt];

    } else {
      /* we have a cycle. Pick the state with the lowest degree */
      min_deg = 1e9; /* arbitrary big number */
      i = -1;
      for ( j=0; j<total; j++ ) {
	/* note: some degree counts can become negative because we
	   force deg_cnt[i] to be zero! */
	if ( deg_cnt[j] > 0 && deg_cnt[j] < min_deg ) {
	  i = j;
	  min_deg = deg_cnt[j];
	}
      }

      if ( i == -1 ) {
	wlog( 1, "WARGH! No more nodes to be done!\n" );
      }

      /* the number of times we had to do a full walk over the
	 states */
      misses++;
    }

    /* force this to be negative, otherwise we might pick it up again */
    deg_cnt[i] = -1;

    /* State i is the state with the lowest in-degree.  Having the
       lowest in-degree means that nothing depends on it, meaning that
       we want to process it *last*.  That's why we put it at the
       *back* of the final ordering.
     */
    pp->variable_ordering[ total-1 - done_cnt ] = i;
    /*    pp->variable_ordering[ done_cnt ] = i; */
    done_cnt++;

    /* subtract out edges to dependencies. */
    /* if the dependency node has zero degree after subtracting, add it
       to the queue */

    sp = &( pp->states[ i ] );
    for ( a=0; a<sp->num_actions; a++ ) {
      scnt = sp->tps[a].int_deps;

      for ( j=0; j<scnt; j++ ) {
	g_state = sp->tps[a].entries[j].col;
	/* 'kay. local state i depends on global state g_state. */
	l_state = gsi_to_lsi( w, g_state );

/* 	toadd = (int)( (float)100.0 * sp->tps[a].entries[j].entry ); */
/* 	deg_cnt[ l_state ] -= toadd; */
	deg_cnt[ l_state ] -= 1;

/* 	if ( toadd != 0 && deg_cnt[ l_state ] == 0 ) { */
	if ( deg_cnt[ l_state ] == 0 ) {
	  zero_deg_list[zd_cnt] = l_state;
	  zd_cnt++;
	}

      }
    }
  }

  /* sanity check: make sure each variable is used! */
  memset( zero_deg_list, 0, sizeof(int)*total );

  for ( i=0; i<total; i++ ) {
    zero_deg_list[ pp->variable_ordering[i] ] = 1;
  }

  for ( i=0; i<total; i++ ) {
    if ( zero_deg_list[i] != 1 ) {
      wlog( 1, "WHOA! BAD VARIABLE ORDERING -- PART %d!\n", l_part );
      for ( i=0; i<total; i++ ) {
	wlog( 1, "%d ", pp->variable_ordering[i] );
      }
      wlog( 1, "\n" );
      abort();
    }
  }

/*   wlog( 1, "  %d: %d\n", l_part, misses ); */

  free( zero_deg_list );
  free( deg_cnt );

}

void reorder_states_within_partitions( world_t *w ) {
  int l_part;

  for ( l_part=0; l_part < w->num_local_parts; l_part++ ) {
    reorder_states_within_partition( w, l_part );
  }
}

/*
 * ----------------------------------------------------------------------------
 */

void compute_initial_partition_priorities( world_t *w ) {
  int l_part, g_part, l_state;
  int state_cnt;
  prec_t tmpheat, maxheat;

  l_part = -1;
  g_part = -1;
  while ( iterate_over_parts_seq( w, &l_part, &g_part ) ) {
    part_check_in( w, l_part );

    maxheat = 0;
    state_cnt = w->parts[ l_part ].num_states;

    for ( l_state = 0; l_state < state_cnt; l_state++ ) {
      tmpheat = get_heat( w, l_part, l_state );

      maxheat = MAX( maxheat, tmpheat );
    }

    /* we need to update the initial heat of this partition */
    w->parts[ l_part ].heat = maxheat;
    w->parts[ l_part ].primary_heat = maxheat;

  }
}

/*
 * ----------------------------------------------------------------------------
 */

prec_t value_sum( world_t *w ) {
  int part, state, scnt;
  prec_t total;
  part_t *pa;

  total = 0;
  for ( part=0; part<w->num_local_parts; part++ ) {
    pa = &( w->parts[ part ] );
    scnt = pa->num_states; 
    for ( state=0; state<scnt; state++ ) {
      total += pa->values->elts[ state ];
    }
  }

  return total;
}

/* washes over the partition once */ // - This does it just once - Anuj
//Now it does it till the partiton is completely solved.
prec_t value_iterate_partition( world_t *w, int l_part )
{
    part_t *pp;
    int l_state, state_cnt, l_sub_part;
    float max_heat, delta, part_internal_heat, tmp;
    med_hash_t *dep_part_hash;
    int numPartitionIters = 0;
    
    int g_end_ext_partition, g_end_ext_state, l_end_ext_state, index1 = 0, index2 = 0;
    val_t *val_state_action;
/*   FILE *fp; */

  /* make sure that the data is in the odcd cache! */
    part_check_in( w, l_part );

    max_heat = 0;
    pp = &( w->parts[ l_part ] );
    state_cnt = pp->num_states;


    dep_part_hash = w->parts[l_part].my_ext_parts_states;
    //Iterate over all external states grouped by partitions they belong to.
    //Load their values from their respective partition arrays.
    while ( med_hash_hash_iterate( dep_part_hash, &index1, &index2,
                                  &g_end_ext_partition, &l_end_ext_state, &val_state_action ))
    {
        val_state_action->d = w->parts[g_end_ext_partition].values->elts[l_end_ext_state];   //Setting the value of that external state.
    }
    //First iteration.
    for ( l_state = 0; l_state < state_cnt; l_state++ )
    {
        delta = value_update( w, l_part, l_state );
        max_heat = MAX( fabs( delta ), max_heat );
    }
    
    if ((w->state_to_subpartnum != NULL) && (max_heat > heat_epsilon) )
    {
        empty_queue(w->parts[l_part].sub_part_queue);
        init_sub_part_queue(w, l_part);
    
        while (sub_part_available_to_process(w, l_part))
        {
            l_sub_part = get_next_sub_part(w, l_part);
            tmp = value_iterate_sub_partition( w, l_part, l_sub_part);
            if (tmp > heat_epsilon)
            {
                add_sub_partition_deps_for_eval(w, l_part, l_sub_part);
                max_heat = tmp;
            }
        }
    }
    else if (max_heat > heat_epsilon)
    {
        //This is equivalent to while(true) as we don't change max_heat in the while loop.
        //If max_heat == 0 we don't need to enter this loop as partition is already cold.
        while(max_heat > 0)
        {
            //part_internal_heat initialized to 0 at beginning of each iteration.
            //It attains value of max heat in that iteration and keeps on reducing with every iteration.
            //It signifies that we are making progress within the partition.
            part_internal_heat = 0;
            for ( l_state = 0; l_state < state_cnt; l_state++ )
            {
                delta = value_update_iters( w, l_part, l_state );
                part_internal_heat = MAX( fabs( delta ), part_internal_heat );
            }
            w->parts[ l_part ].washes++;
            numPartitionIters++;
            if (part_internal_heat < heat_epsilon) //excluding (numPartitionIters > MAX_ITERS_PP) ||
            {
                //if (numPartitionIters > 1)
                //if (numPartitionIters >= 20)
                //  if ( verbose ) { wlog( 1, "Partition %d was processed %d number of times. Part Internal Heat is: %.6f. Max heat to begin with is: %.6f \n", l_part, numPartitionIters, part_internal_heat, max_heat ); }
                break;
            }
        }
    }
    return max_heat;
}

prec_t value_iterate_sub_partition( world_t *w, int l_part, int l_sub_part)
{
    int state_cnt, l_sub_state;
    float delta, max_change, sub_part_internal_heat;
    state_cnt = w->parts[l_part].sub_parts[l_sub_part].num_states_sub;
    
    max_change = 0;
    while(1)
    {
        //part_internal_heat initialized to 0 at beginning of each iteration.
        //It attains value of max heat in that iteration and keeps on reducing with every iteration.
        //It signifies that we are making progress within the partition.
        sub_part_internal_heat = 0;
        for ( l_sub_state = 0; l_sub_state < state_cnt; l_sub_state++ )
        {
            delta = value_update_iters( w, l_part, w->parts[l_part].sub_parts[l_sub_part].sub_parts_states[l_sub_state] );
            sub_part_internal_heat = MAX( fabs( delta ), sub_part_internal_heat );
            if (sub_part_internal_heat > max_change)
                max_change = sub_part_internal_heat;
        }
//        numPartitionIters++;
        if (sub_part_internal_heat < heat_epsilon) //excluding (numPartitionIters > MAX_ITERS_PP) ||
        {
            //if (numPartitionIters > 1)
            //if (numPartitionIters >= 20)
            //  if ( verbose ) { wlog( 1, "Partition %d was processed %d number of times. Part Internal Heat is: %.6f. Max heat to begin with is: %.6f \n", l_part, numPartitionIters, part_internal_heat, max_heat ); }
            
            
            break;
        }
    }
    return max_change;
}


/* washes over all of the partitions */
prec_t value_iterate( world_t *w ) {
  int l_part;
  prec_t maxheat, tmp;

  maxheat = 0;
    
    while (part_available_to_process(w))
    {
        l_part = get_next_part(w);
        tmp = value_iterate_partition( w, l_part );
        if (tmp > heat_epsilon)
        {
            add_partition_deps_for_eval(w, l_part);
            maxheat = tmp;
        }
    }

/*  for ( l_part = 0; l_part < w->num_local_parts; l_part++ ) {

    tmp = value_iterate_partition( w, l_part );

    if ( tmp > maxheat ) {
      maxheat = tmp;
    }
  }
*/
  return maxheat;
}

/*
 * ----------------------------------------------------------------------------
 */

prec_t value_update( world_t *w, int l_part, int l_state ) {
  int action, max_action, nacts;
  prec_t tmp, max_value, cval;

  cval = w->parts[ l_part ].values->elts[ l_state ];

  max_action = 0;
  max_value = reward_or_value( w, l_part, l_state, 0 );

  /* remember that there is an action bias! */
  nacts = w->parts[ l_part ].states[ l_state ].num_actions;
  for (action=1; action<nacts; action++) {
    tmp = reward_or_value( w, l_part, l_state, action );
    if ( tmp > max_value ) {
      max_value = tmp;
      max_action = action;
    }
  }

/*   fprintf( stderr, "  %d-%d\n", l_part, l_state ); */
/*   fprintf( stderr, "  %.5f-%.5f=%.5f\n", max_value, cval, max_value-cval ); */

  if ( max_value < 0 ) {
    fprintf( stderr, "WARGH!\n" );
    exit( 0 );
  }

  w->parts[ l_part ].values->elts[ l_state ] = max_value;       //Update the V(s,a) for this state.

  w->num_value_updates++;

  return max_value - cval;
}



//ANUJ - This is called in all iterations after the 1st one.
//Only difference from the function called in first iteration is that it calls reward_or_values_iters.
//This version of the reward_or_values, does not compute the values of external dependencies, just uses the cached value.
prec_t value_update_iters( world_t *w, int l_part, int l_state ) {
    int action, max_action, nacts;
    prec_t tmp, max_value, cval;
    
    cval = w->parts[ l_part ].values->elts[ l_state ];
    
    max_action = 0;
    max_value = reward_or_value_iters( w, l_part, l_state, 0 );     //ANUJ - Calling the iters version.
    
    /* remember that there is an action bias! */
    nacts = w->parts[ l_part ].states[ l_state ].num_actions;
    for (action=1; action<nacts; action++) {
        tmp = reward_or_value_iters( w, l_part, l_state, action );
        if ( tmp > max_value ) {
            max_value = tmp;
            max_action = action;
        }
    }
    
    /*   fprintf( stderr, "  %d-%d\n", l_part, l_state ); */
    /*   fprintf( stderr, "  %.5f-%.5f=%.5f\n", max_value, cval, max_value-cval ); */
    
    if ( max_value < 0 ) {
        fprintf( stderr, "WARGH!\n" );
        exit( 0 );
    }
    
    w->parts[ l_part ].values->elts[ l_state ] = max_value;       //Update the V(s,a) for this state.
    
    w->num_value_updates_iters++;
    
    return max_value - cval;
}




/*
 * ----------------------------------------------------------------------------
 */

int get_policy( world_t *w, int l_part, int l_state ) {
  int action, max_action, nacts;
  prec_t tmp, max_value;

  max_action = 0;
  max_value = reward_or_value( w, l_part, l_state, 0 );

  nacts = w->parts[ l_part ].states[ l_state ].num_actions;
  for (action=1; action<nacts; action++) {

    tmp = reward_or_value( w, l_part, l_state, action );

    if ( tmp > max_value ) {
      max_value = tmp;
      max_action = action;
    }
  }

  return max_action;
}

/*
 * ----------------------------------------------------------------------------
 */

int get_max_deps( state_t *st ) {
  int a, max_deps;

  max_deps = 0;
  for ( a=0; a<st->num_actions; a++ ) {
    if ( st->tps[a].int_deps > max_deps ) {
      max_deps = st->tps[a].int_deps;
    }
  }

  return max_deps;
}

/* this doesn't count diagonal elements */
int get_max_deps_nd( state_t *st, int l_state ) {
  int a, i, cnt, total, max;

  max = 0;

  for ( a=0; a<st->num_actions; a++ ) {

    total = 0;
    cnt = st->tps[a].int_deps;
    for ( i=0; i<cnt; i++ ) {
      if ( st->tps[a].entries[i].col != l_state ) {
	total++;
      }
    }

    if ( total > max ) {
      max = total;
    }
  }

  return max;
}


/*
 * ----------------------------------------------------------------------------
 */

//#ifdef USE_MPI

prec_t get_val( world_t *w, int g_state ) {
  prec_t f = 0;
#ifdef USE_MPI
#ifdef PREC_IS_FLOAT
  med_hash_get_float( w->foreign_state_val_hash, g_state, &f );
#else
  med_hash_get_double( w->foreign_state_val_hash, g_state, &f );
#endif
#endif
  return f;
}

//#endif

prec_t get_heat( world_t *w, int l_part, int l_state ) {
  if ( heat_metric == HEAT_STD ) {
    return std_diff_heat( w, l_part, l_state );
  } else {
    return max_val_heat( w, l_part, l_state );
  }
}

/* t is an array of transitions */
/* this is the H2 metric */
prec_t max_val_heat( world_t *w, int l_part, int l_state ) {
  int action, nacts;
  prec_t cur, could_be, tmp;

  cur = w->parts[ l_part ].values->elts[ l_state ];

  could_be = reward_or_value( w, l_part, l_state, 0 );

  nacts = w->parts[ l_part ].states[ l_state ].num_actions;
  for ( action=1; action<nacts; action++ ) {
    tmp = reward_or_value( w, l_part, l_state, action );
    could_be = MAX( could_be, tmp );
  }

/*   if ( could_be < cur ) { */
/*     wlog( 1, "WARGH: %d-%d: %.6f %.6f\n", l_part, l_state, could_be, cur ); */
/*     exit( 0 ); */
/*   } */

  if ( could_be - cur > heat_epsilon ) {
    return could_be;
  }

  return 0;
}

/* t is an array of transitions */
/* this is the H1 metric */
prec_t std_diff_heat( world_t *w, int l_part, int l_state ) {
  int action, nacts;
  prec_t cur, could_be, heat, tmp;

  cur = w->parts[ l_part ].values->elts[ l_state ];
  could_be = reward_or_value( w, l_part, l_state, 0 );
  heat = could_be - cur;

  nacts = w->parts[ l_part ].states[ l_state ].num_actions;
  for ( action=1; action<nacts; action++ ) {
    could_be = reward_or_value( w, l_part, l_state, action );
    tmp = could_be - cur;
    heat = MAX( tmp, heat );
  }

/*   return fabs(heat); */
  return heat;
}

prec_t reward_or_value( world_t *w, int l_part, int l_state, int a ) {
  prec_t value, tmp;
  state_t *st;
//    double t_start, t_end;
    
//    t_start = when();

  st = &( w->parts[ l_part ].states[ l_state ] );

  /* compute the internal values */
  value = entries_vec_mult( st->tps[ a ].entries,
			    st->tps[ a ].int_deps,
			    w->parts[ l_part ].values );

  /* add in external deps! */

  /*#error This needs to grok the global vs. local state distinction.  Wow.  How could it possibly not do that??? */

  tmp = get_remainder( w, l_part, l_state, a );     //Getting external dependencies.
  
  st->external_dep_vals[a] = tmp;      //Cache the values of the external deps in the state.
                                    //This won't need to be computed in any of the remaining iterations of this partition.

/*   if ( l_part == 0 && l_state == 0 ) { */
/*     fprintf( stderr, "  %d-%d (%d) %d: %.5f + %.5f + ", */
/* 	     l_part, l_state, st->global_state_index, a, value, tmp ); */
/*   } */

  value += tmp;

  /* we have to do this because we negated the A matrix! */
  value = -value + st->tps[ a ].reward;

/*   if ( l_part == 0 && l_state == 0 ) { */
/*     fprintf( stderr, "%.5f = %.5f\n", st->tps[a].reward, value ); */
/*   } */
    
//    t_end = when();
//    w->reward_or_value_updatetime += t_end - t_start;
  return value;
}



//ANUJ - using this function for the iterations of the partition after the 1st.
//This doesn't compute the external dependencies but just uses the cached value from the 1st iteration.
prec_t reward_or_value_iters( world_t *w, int l_part, int l_state, int a ) {
    prec_t value;//, tmp;
    state_t *st;
//    double t_start, t_end;
    
//    t_start = when();
    
    st = &( w->parts[ l_part ].states[ l_state ] );
    
    /* compute the internal values */
    value = entries_vec_mult( st->tps[ a ].entries,
                             st->tps[ a ].int_deps,
                             w->parts[ l_part ].values );
    /*#error This needs to grok the global vs. local state distinction.  Wow.  How could it possibly not do that??? */
    value += st->external_dep_vals[a];      //External deps
    
    /* we have to do this because we negated the A matrix! */
    value = -value + st->tps[ a ].reward;

    //    t_end = when();
//    w->reward_or_value_updatetime += t_end - t_start;
    return value;
}


/*
 * ----------------------------------------------------------------------------
 */

int iterate_over_parts_seq( world_t *w, int *local_part, int *global_part ) {
  *local_part += 1;

#ifdef USE_MPI
  if ( *local_part >= w->num_local_parts ) {
    return 0;
  }
  *global_part = lpi_to_gpi( w, *local_part );

#else
  if ( *local_part >= w->num_global_parts ) {
    return 0;
  }
  *global_part = *local_part;
#endif

  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

/*
 With this function, we create the datastructures necessary to manage
 the dependencies between partitions and states.
*/

void add_dep( world_t *w,
	      int g_start_state, int l_start_state,
	      int g_start_part, int l_start_part,
	      int g_end_state, int g_end_part ) {

  int l_end_part, r;

  /* no matter what, g_start_part depends on g_end_part, so
     it g_start_part needs a heat link to g_end_part */
#ifdef PREC_IS_FLOAT
//  r = med_hash_add_float( w->parts[ l_start_part ].heat_links, g_end_part, 0 );
#else
//  r = med_hash_add_double( w->parts[ l_start_part ].heat_links, g_end_part, 0 );
#endif
//  assert( r == MH_ADD_OK || r == MH_ADD_REPLACED );

#ifdef USE_MPI
  if ( is_partition_local( w, g_end_part ) ) {
#endif
    /* it's a local partition */

    l_end_part = gpi_to_lpi( w, g_end_part );

    /* g_start_state (which resides in g_start_part) depends on
       g_end_state (which resides in g_end_part).
       so, g_end_part needs to have a dependent which is l_start_part. */

  med_hash_set_add( w->parts[ l_end_part ].my_local_dependents,
		      g_start_part, l_start_state );

#ifdef USE_MPI
  } else {
    /* it's a foreign partition. */

    /* globally, we need to say that g_start_part depends on
       g_end_part this is so that we can do a reverse lookup when we
       get a message about g_end_part having changed.  So, we map
       g_end_part to g_start_part. */
    med_hash_set_add( w->endpart_to_startpart,
		      g_end_part, g_start_part );

    /* Within l_start_part, we need to map g_end_part -> g_start_state */
    med_hash_set_add( w->parts[ l_start_part ].endpart_to_startstate,
		      g_end_part, l_start_state );

    /* We need to inform the foreign processor that we are interested
       in the value of end_state, and that we should be notified when
       it changes.  So, we map proc -> g_end_state (we can throw this
       hash away once we've coordinated dependencies). */
    med_hash_set_add( w->fproc_data,
		      w->part_to_proc[ g_end_part ].owning_processor,
		      g_end_state );  
  }

#endif /* defined USE_MPI */
}

void add_part_ext_dep_states( world_t *w,
                             int g_start_part, int l_start_part,
                             int l_start_state, int l_end_state, int g_end_part)
{

    med_hash_set_add( w->parts[ l_start_part ].my_ext_parts_states,
                     g_end_part, l_end_state );
    
}


void add_cache_states(world_t *w,
                      int g_start_part, int l_start_part,
                      int l_start_state, int l_end_state, int g_end_part, val_t **arrayValptrs, int indexVal )
{
    val_t *val_address;
    int r;
    
    val_t v2;
    med_hash_t *m2;
    
    med_hash_get( w->parts[ l_start_part ].my_ext_parts_states, g_end_part, &v2);
    m2 = (med_hash_t *)v2.vptr;
    r = med_hash_getp(m2, l_end_state, &val_address);
    if (r == MH_FOUND)
        arrayValptrs[indexVal] = val_address;       //The state, action array points to the value of the external state.
    else
        wlog(1, "Strange!!! just added Values in Cache not found!!!");
}

/*
 * ----------------------------------------------------------------------------
 */

void switch_to_policy_normal( world_t *w, int l_part, int l_state,
			      int new_pol ) {
  part_t *p;
  int cur_pol;

  p = &( w->parts[ l_part ] );

  cur_pol = p->states[ l_state ].policy;
  p->states[ l_state ].policy = new_pol;

  /* update the RHS with the new reward */
  p->rhs->elts[ l_state ] = p->states[ l_state ].tps[ new_pol ].reward;

  /* switch the rows in the matrix around. */
  /* make sure that certain variables are updated correctly */

  p->cur_pol_matrix->rows[ l_state] .entries =
    p->states[ l_state ].tps[ new_pol ].entries;

  p->cur_pol_matrix->rows[ l_state ].colcnt =
    p->states[ l_state ].tps[ new_pol ].int_deps;

  p->cur_pol_matrix->total_nz -= p->states[ l_state ].tps[ cur_pol ].int_deps;
  p->cur_pol_matrix->total_nz += p->states[ l_state ].tps[ new_pol ].int_deps;

}

void switch_to_policy( world_t *w, int l_part, int l_state, int new_pol ) {

#ifdef USE_AZTEC
  if ( is_aztec_solver( w->solver ) ) {
    switch_to_policy_aztec( w, l_part, l_state, new_pol );
  } else {
    switch_to_policy_normal( w, l_part, l_state, new_pol );
  }

#else
  switch_to_policy_normal( w, l_part, l_state, new_pol );

#endif

}

/*
 * ----------------------------------------------------------------------------
 */

void part_matrix_init( world_t *w, int l_part ) {
  int cnt;
  part_t *p;

  p = &( w->parts[ l_part ] );

  cnt = p->num_states;

  /* allocate vectors */
  p->values = vec_create( cnt );
  p->rhs = vec_create( cnt );
    
    p->size_values = sizeof(vec_t);
    p->size_values += sizeof(prec_t)* cnt;
    p->size_rhs = sizeof(vec_t);
    p->size_rhs += sizeof(prec_t)* cnt;
    
    w->size_parts[4] += p->size_rhs;  w->size_parts[5] += p->size_values;

  if ( p->rhs == NULL ) {
    wlog( 1, "Error allocating partition rhs/value!\n" );
    exit( 0 );
  }

  /* allocate cur_pol_matrix */
  p->cur_pol_matrix = (matrix_t *)malloc( sizeof(matrix_t) );
  if ( p->cur_pol_matrix == NULL ) {
    wlog( 1, "Error allocating partition matrix!\n" );
    exit( 0 );
  }

  p->cur_pol_matrix->nrows = cnt;
  p->cur_pol_matrix->ncols = cnt;
  p->cur_pol_matrix->add_identity = 1;

  p->cur_pol_matrix->rows = (rowcol_t *)malloc( sizeof(rowcol_t) * cnt );
  if ( p->cur_pol_matrix->rows == NULL ) {
    wlog( 1, "Error allocating partition matrix rows!\n" );
    exit( 0 );
  }
  p->cur_pol_matrix->__row_entries = NULL;

  /* setup the initial xestimate values */
  /* the initial matrix / reward vector will be setup in
     setup_initial_policy() */

  const_vec( p->values, 0 );

}

void setup_initial_policy( world_t *w ) {
  int l_part, l_state, nzcnt, cnt;
  part_t *p;

  for ( l_part=0; l_part<w->num_local_parts; l_part++ ) {

    p = &( w->parts[ l_part ] );
    cnt = p->num_states;
    nzcnt = 0;

    for ( l_state=0; l_state<cnt; l_state++ ) {

      /* action 0 is the default policy */
      p->states[ l_state ].policy = 0;
      switch_to_policy( w, l_part, l_state, 0 );

      nzcnt += p->states[ l_state ].tps[ 0 ].int_deps;
    }

    p->cur_pol_matrix->total_nz = nzcnt;

  }
}

void initialize_partitions( world_t *w ) {
  int l_part;

  for ( l_part=0; l_part<w->num_local_parts; l_part++ ) {

    /* create the dependency hashes for this partition */
    w->parts[ l_part ].my_local_dependents = med_hash_create( 4 );
    w->parts[ l_part ].my_ext_parts_states = med_hash_create( 4 );


    if ( //w->parts[ l_part ].heat_links == NULL ||
	 w->parts[ l_part ].my_local_dependents == NULL ||
     w->parts[ l_part ].my_ext_parts_states  == NULL)
    {
      wlog( 1, "Error creating dependency hashes for part %d!\n", l_part );
      exit( 0 );
    }

    /* create all of the matrix stuff necessary for our library */
    part_matrix_init( w, l_part );

  }
}

void initialize_sub_partitions( world_t *w )
{
    int l_part, sub_part;
    
    if (w->state_to_subpartnum == NULL)
        return;

    for ( l_part=0; l_part<w->num_local_parts; l_part++ )
    {
        for (sub_part = 0; sub_part < w->parts[l_part].num_sub_parts; sub_part++)
        {
        
            /* create the dependency hashes for this partition */
            w->parts[ l_part ].sub_parts[sub_part].my_local_dependents = med_hash_create( 4 );
//            w->parts[ l_part ].sub_parts[sub_part].my_ext_parts_states = med_hash_create( 4 );
        
            if (w->parts[ l_part ].sub_parts[sub_part].my_local_dependents == NULL
                //|| w->parts[ l_part ].my_ext_parts_states  == NULL
                )
            {
                wlog( 1, "Error creating dependency hashes for part %d and sub-part %d!\n", l_part, sub_part );
                exit( 0 );
            }
//            part_matrix_init( w, l_part );
        }
    }
}


/*
 * ----------------------------------------------------------------------------
 */

int state_to_partnum( world_t *w, int g_state ) {
  if ( w->state_to_partnum == NULL ) {
    return 0;
  }
  return w->state_to_partnum[ g_state ];
}

int lsi_to_gsi( world_t *w, int l_part, int l_state ) {
  if ( odcd_enabled() ) {
    assert( w->parts[ l_part ].odcd_elem.in_cache == 1 );
  }
  return w->parts[ l_part ].states[ l_state ].global_state_index;
}

int gsi_to_lsi( world_t *w, int g_state ) {
  return w->gsi_to_lsi[ g_state ];
}

/*
 * ----------------------------------------------------------------------------
 */

int odcd_enabled( void ) {
  if ( odcd_cache_size == -1 ) {
    return 0;
  }

  return 1;
}

int attractors_enabled( void ) {
  if ( num_attractors == 0 ) {
    return 0;
  }

  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void save_resulting_vector( world_t *w, char *fn ) {
  FILE *fp;
  int l_part, g_state, l_state, cnt, policy;
  state_t *st;
  prec_t val;

  if ( fn == NULL ) {
    fp = stdout;
  } else {
    fp = fopen( fn, "wb" );
    if ( fp == NULL ) {
      wlog( 1, "Error opening %s!\n", fn );
      return;
    }
  }

  for ( l_part = 0; l_part < w->num_local_parts; l_part++ ) {

    part_check_in( w, l_part );

/* #ifdef USE_AZTEC */
/*     if ( run_type == RUN_PI ) { */
/*       copy_values_to_svhash( w, l_part ); */
/*     } */
/* #endif */

    cnt = w->parts[ l_part ].num_states;
    for ( l_state = 0; l_state < cnt; l_state++ ) {

      st = &( w->parts[ l_part ].states[ l_state ] );
      policy = get_policy( w, l_part, l_state );
      g_state = st->global_state_index;
      val = w->parts[ l_part ].values->elts[ l_state ];

      if ( w->parts[ l_part ].visits == 0 ) {
	policy = -1;
      }

      fprintf( fp, "%d %d %.4f\n", g_state, policy, val );
    }
  }

  if ( save_fn != NULL ) {
    fclose( fp );
  }

}

void check_movie( world_t *w ) {
  int frame;

  if ( make_movie != MM_YES ) {
    return;
  }

  switch( run_type ) {
  case RUN_VI:
    if ( w->vi_sweeps % every_nth_frame == 0 ) {
      frame = w->vi_sweeps;
    } else {
      return;
    }
    break;
  case RUN_PVI:
    if ( w->parts_processed % every_nth_frame == 0 ) {
      frame = w->parts_processed;
    } else {
      return;
    }
    break;
  case RUN_PI:
  case RUN_PPI:
    if ( w->pi_sweeps % every_nth_frame == 0 ) {
      frame = w->pi_sweeps;
    } else {
      return;
    }
    break;

  default:
    return;
  }

  wlog( 1, "FRAME: %d\n", frame );
  sprintf( movie_fn, movie_format, frame );
  save_resulting_vector( w, movie_fn );

}

/*
 * ---------------------------------------------------------------------------
 */

/*

A matrix is initially made up of all global state indices.  This
function translates all of those global indices into local indices in
the partition.

*/

void translate_and_negate_all( world_t *w ) {
  int l_part;

  for ( l_part=0; l_part<w->num_local_parts; l_part++ ) {
    translate_to_local_matrix( w, l_part );
    negate_matrix( w, l_part );
  }
}

void translate_to_local_matrix( world_t *w, int l_part ) {
  int l_state, nacts, a, ndeps, i, tmp;
  state_t *st;

  for ( l_state=0; l_state < w->parts[l_part].num_states; l_state++ ) {
    st = &( w->parts[ l_part ].states[ l_state ] );
    nacts = st->num_actions;

    for ( a=0; a<nacts; a++ ) {
      ndeps = st->tps[a].int_deps;

      for ( i=0; i<ndeps; i++ ) {
       tmp = gsi_to_lsi( w, st->tps[a].entries[i].col );
       st->tps[a].entries[i].col = tmp;
      }

    }
  }
}


/*

This function computes the external (to the partition) value
dependency of a state.

*/

prec_t get_remainder( world_t *w, int l_part, int l_state, int action ) {
    int i, dep_cnt;
  trans_t *tt;
  entry_t *ext_et;
    prec_t val_hash, val_hash2;

    //    int e_g_p, e_l_p, e_l_state, e_g_state;
    //  prec_t val;
//    val_t v2;
//  val = 0;
    val_hash2 = 0;
    

  tt = &( w->parts[ l_part ].states[ l_state ].tps[ action ] );
  dep_cnt = tt->ext_deps;
  ext_et = &( tt->entries[ tt->int_deps ] );    //* point to the first external entry

  for ( i=0; i<dep_cnt; i++ )
  {
          val_hash2 += ext_et[ i ].entry * (w->parts[l_part].states[l_state].external_state_vals[action][i]->d);

#ifdef NO_CACHE_HASH
      if ( is_partition_local( w, e_g_p ) ) {
      /* e_g_p is in a partition that's local to this processor. */
      e_l_state = gsi_to_lsi( w, e_g_state );
      e_l_p = gpi_to_lpi( w, e_g_p );
      val += ext_et[ i ].entry * w->parts[ e_l_p ].values->elts[ e_l_state ];

    } else {
      /* e_g_p is in a partition that's foreign.  A value for it
	 should be housed in the foreign state val hash. */
      val += ext_et[ i ].entry * get_val( w, e_g_state );
    }

#endif
  }

//      return val;
  return val_hash2;
}


/* this folds the external dependency into the RHS vector */

void fold( world_t *w, int l_part ) {
  _fold( w, l_part, 1 );
}
void unfold( world_t *w, int l_part ) {
  _fold( w, l_part, 0 );
}

void _fold( world_t *w, int l_part, int add_or_sub ) {
  int l_state;
  prec_t val;
  state_t *st;

  if ( add_or_sub ) {

    for ( l_state=0; l_state<w->parts[ l_part ].num_states; l_state++ ) {
      st = &( w->parts[ l_part ].states[ l_state ] );
      val = get_remainder( w, l_part, l_state, st->policy );
      w->parts[ l_part ].rhs->elts[ l_state ] -= val;
    }

  } else {

    for ( l_state=0; l_state<w->parts[ l_part ].num_states; l_state++ ) {
      st = &( w->parts[ l_part ].states[ l_state ] );
      val = get_remainder( w, l_part, l_state, st->policy );
      w->parts[ l_part ].rhs->elts[ l_state ] += val;
    }


  }
}

/*
 * ----------------------------------------------------------------------------
 * Policy iteration!
 * ----------------------------------------------------------------------------
 */

int policy_improvement( world_t *w, int l_part ) {
  int l_state, cnt, policy, changed;
  state_t *st;

  changed = 0;
  cnt = w->parts[ l_part ].num_states;
  for ( l_state=0; l_state<cnt; l_state++ ) {

    st = &( w->parts[ l_part ].states[ l_state ] );

    policy = get_policy( w, l_part, l_state );

    if ( policy != st->policy ) {
      switch_to_policy( w, l_part, l_state, policy );
      changed++;
    }
  }

  return changed;
}

/*
 * ----------------------------------------------------------------------------
 */

prec_t policy_evaluate_normal( world_t *w, int l_part, int *iters ) {
  int r;
  prec_t error;
  vec_t *residual;

  /* krylov subspace size / GMRES restart parameter */

  residual = new_vec_from_matrix( w->parts[ l_part ].cur_pol_matrix );

  switch( w->solver ) {
  case RICHARDSON:
    r = richardson( w->parts[ l_part ].cur_pol_matrix,
		    w->parts[ l_part ].values,
		    w->parts[ l_part ].rhs,
		    residual,
		    w->max_iters,
		    w->tol,
		    &error,
		    iters );
    break;
    
  case STEEPEST_DESCENT:
    r = steepest_descent( w->parts[ l_part ].cur_pol_matrix,
			  w->parts[ l_part ].values,
			  w->parts[ l_part ].rhs,
			  residual,
			  w->max_iters,
			  w->tol,
			  &error,
			  iters );
    break;

  case MINIMUM_RESIDUAL:
    r = minimum_residual( w->parts[ l_part ].cur_pol_matrix,
			  w->parts[ l_part ].values,
			  w->parts[ l_part ].rhs,
			  residual,
			  w->max_iters,
			  w->tol,
			  &error,
			  iters );
    break;

  case RESIDUAL_NORM_SD:
    r = residual_norm_sd( w->parts[ l_part ].cur_pol_matrix,
			  w->parts[ l_part ].values,
			  w->parts[ l_part ].rhs,
			  residual,
			  w->max_iters,
			  w->tol,
			  &error,
			  iters );
    break;

  case GMRES:
    r = gmres( w->parts[ l_part ].cur_pol_matrix,
	       w->parts[ l_part ].values,
	       w->parts[ l_part ].rhs,
	       residual,
	       w->max_iters,
	       w->tol,
	       &error,
	       iters,
	       w->kss_size
	       );
    break;

  case GAUSSIAN_ELIM:
    r = direct_gaussian_solver( w->parts[ l_part ].cur_pol_matrix,
				w->parts[ l_part ].values,
				w->parts[ l_part ].rhs,
				residual,
				w->max_iters,
				w->tol,
				&error,
				iters
				);
    break;


  case CONJUGATE_GRAD:
    r = cg( w->parts[ l_part ].cur_pol_matrix,
	    w->parts[ l_part ].values,
	    w->parts[ l_part ].rhs,
	    residual,
	    w->max_iters,
	    w->tol,
	    &error,
	    iters );
    break;

  case CONJUGATE_GRAD_NR:
    r = cgnr( w->parts[ l_part ].cur_pol_matrix,
	      w->parts[ l_part ].values,
	      w->parts[ l_part ].rhs,
	      residual,
	      w->max_iters,
	      w->tol,
	      &error,
	      iters );
    break;

  case CONJUGATE_GRAD_NE:
    r = cgne( w->parts[ l_part ].cur_pol_matrix,
	      w->parts[ l_part ].values,
	      w->parts[ l_part ].rhs,
	      residual,
	      w->max_iters,
	      w->tol,
	      &error,
	      iters );
    break;

  }

  vec_free( residual );

/* #ifdef PREC_IS_FLOAT */
/*   wlog( 1, "  %d %.4f\n", iters, error ); */
/* #else */
/*   wlog( 1, "  %d %.4lf\n", iters, error ); */
/* #endif */

  return error;

}

/*
 * ----------------------------------------------------------------------------
 */

prec_t policy_evaluate( world_t *w, int l_part, int *iters ) {
  prec_t r;

  /* We need to fold! */
  fold( w, l_part );

#ifdef USE_AZTEC

  if ( is_aztec_solver( w->solver ) ) {
    r = policy_evaluate_aztec( w, l_part, iters );
  } else {
    r = policy_evaluate_normal( w, l_part, iters );
  }

#else
  r = policy_evaluate_normal( w, l_part, iters );

#endif

  /* we need to unfold! */
  unfold( w, l_part );

  return r;

}

/*
 * ----------------------------------------------------------------------------
 */

/* we do this for a somewhat complicated reason.
   We actually want to solve x = Ax + b.
   But, the solver expects an equation of the form Ax=b.
   No problem; we can just factor things around a little
   bit to generate  (-A+I)x=b.  That's why we have to
   negate A and add the identity.  Note: you can't do
   it the other way, and solve (A-I)x=-b, because then
   you end up with iterations of the form x=-Ax-b!  It
   diverges pretty quickly, because you're actually taking
   the negative of the gradient...
*/

void negate_matrix( world_t *w, int l_part ) {
  int cnt, l_state, nd, nacts, a, i;
  state_t *st;

  cnt = w->parts[ l_part ].num_states;

  for ( l_state=0; l_state<cnt; l_state++ ) {

    st = &( w->parts[ l_part ].states[ l_state ] );
    nacts = st->num_actions;

    for ( a=0; a<nacts; a++ ) {
      nd = st->tps[a].int_deps + st->tps[a].ext_deps;
      for ( i=0; i<nd; i++ ) {
	st->tps[a].entries[i].entry *= -1;
      }
    }
  }

}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 *
 * Aztec stuff
 *
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

#ifdef USE_AZTEC

prec_t policy_evaluate_aztec( world_t *w, int l_part, int *iters ) {

  AZ_solve(
	   w->parts[ l_part ].values->elts,
	   w->parts[ l_part ].rhs->elts,
	   w->az_options,
	   w->az_params,
	   NULL,
	   w->parts[ l_part ].bindx,
	   NULL,
	   NULL,
	   NULL,
	   w->parts[ l_part].val,
	   w->parts[ l_part].data_org,
	   w->az_status,
	   w->az_proc_config
	   );

  *iters = w->az_status[AZ_its];

/*   iters = w->az_status[AZ_its]; */
/*   error = w->az_status[AZ_rec_r]; */

/*   wlog( 1, "  %d %.4lf\n", iters, error ); */

/*   fprintf( stderr, "     AZ_its: %.0f\n", w->az_status[AZ_its] ); */
/*   fprintf( stderr, "     AZ_why: %.0f (%s)\n", */
/* 	   w->az_status[AZ_why], */
/* 	   az_why( w->az_status[AZ_why] ) ); */
/*   fprintf( stderr, "       AZ_r: %.5f\n", w->az_status[AZ_r] ); */
/*   fprintf( stderr, "AZ_scaled_r: %.5f\n", w->az_status[AZ_scaled_r] ); */
/*   fprintf( stderr, "   AZ_rec_r: %.5f\n", w->az_status[AZ_rec_r] ); */

  return w->az_status[AZ_r];

}

/*
 * ----------------------------------------------------------------------------
 */

void switch_to_policy_aztec( world_t *w, int l_part, int l_state,
			     int new_pol ) {
  part_t *p;
  int cur_pol, i, cnt, ndeps, start, end, idx;
  entry_t *et;

  p = &( w->parts[ l_part ] );

  cur_pol = p->states[ l_state ].policy;
  p->states[ l_state ].policy = new_pol;

  /* update the RHS with the new reward */
  p->rhs->elts[ l_state ] = p->states[ l_state ].tps[ new_pol ].reward;


  /* swap matrix rows around */

  start = p->bindx[ l_state ];
  end = p->bindx[ l_state+1 ];
  cnt = end - start;

  ndeps = p->states[ l_state ].tps[ new_pol ].int_deps;
  et = p->states[ l_state ].tps[ new_pol ].entries;

  /* ixnay the diagonal */
  p->val[ l_state ] = 0.0;

  idx = 0;
  for ( i=0; i<ndeps; i++ ) {
    if ( et[i].col == l_state ) {
      /* this is a diagonal element */
      p->val[ l_state ] = et[i].entry;
    } else {
      p->bindx[ start + idx ] = et[i].col;
      p->val[ start + idx ] = et[i].entry;
      idx++;
    }
  }

  /* we allocate the maximum number of deps for each row.  if this
     action results in fewer deps, zero out the extra entries.
   */
  for ( i=idx; i<cnt; i++ ) {
    p->bindx[ start + i ] = 0;
    p->val[ start + i ] = 0;
  }

  /* we have to add the identity element */
  /* as above: we're solving (-A+I)x = b */
  p->val[ l_state ] += 1.0;

}

/*
 * ----------------------------------------------------------------------------
 */

char *az_why( int why ) {
  switch( why ) {
  case AZ_normal:    return "normal";
  case AZ_param:     return "param";
  case AZ_breakdown: return "breakdown";
  case AZ_loss:      return "loss";
  case AZ_ill_cond:  return "ill conditioned";
  case AZ_maxits:    return "max iterations";
  default: return "unknown";
  }
}

/*
 * ----------------------------------------------------------------------------
 */

void count_nz( world_t *w, int l_part, int *tsize ) {
  int i, cnt;

  *tsize = 0;
  cnt = w->parts[ l_part ].num_states;

  for ( i=0; i<cnt; i++ ) {
    *tsize = *tsize + get_max_deps_nd( &( w->parts[ l_part ].states[ i ]), i );
  }
}

/*
 * ----------------------------------------------------------------------------
 */

/* here, we only set up the first part of the bindx array.  the rest
   of it will be set up in setup_initial_policy when we plug in our
   initial policy. */

void initial_matrix_to_aztec( world_t *w, int l_part ) {
  part_t *p;
  state_t *st;
  int l_state, cnt, spot, max_nd;

  p = &( w->parts[ l_part ] );

  cnt = p->num_states;
  spot = cnt + 1;

  p->bindx[0] = spot;

  for ( l_state=0; l_state<cnt; l_state++ ) {

    st = &( p->states[l_state] );

    max_nd = get_max_deps_nd( st, l_state );

    spot += max_nd;

    p->bindx[ l_state+1 ] = spot;
  }
}

/*
 * ----------------------------------------------------------------------------
 */

void allocate_aztec( world_t *w, int l_part ) {
  int rowcnt, ncdsize;
  part_t *p;

  /* allocate stuff */

  p = &( w->parts[ l_part ] );

  rowcnt = p->num_states;
  count_nz( w, l_part, &ncdsize );

  p->bindx = (int *)malloc( (ncdsize + rowcnt + 1)*sizeof(int) );
  assert( p->bindx != NULL );
  memset( p->bindx, 0, (ncdsize + rowcnt + 1)*sizeof(int) );

  p->val = (double *)malloc( (ncdsize + rowcnt + 1)*sizeof(double) );
  assert( p->val != NULL );
  memset( p->val, 0, (ncdsize + rowcnt + 1)*sizeof(double) );

  initial_matrix_to_aztec( w, l_part );

  memset( p->data_org, 0, sizeof(int) * AZ_COMM_SIZE );

  p->data_org[AZ_matrix_type] = AZ_MSR_MATRIX;
  p->data_org[AZ_N_internal] = rowcnt;
  p->data_org[AZ_N_int_blk] = rowcnt;
  p->data_org[AZ_name] = l_part;
}

/*
 * ----------------------------------------------------------------------------
 */

void deallocate_aztec( world_t *w, int l_part ) {
  part_t *p;

  p = &( w->parts[ l_part ] );

  free( p->bindx );
  free( p->val );

  p->bindx = NULL;
  p->val = NULL;
}

/*
 * ----------------------------------------------------------------------------
 */

void setup_aztec_params( world_t *w ) {
  int l_part;

  AZ_defaults( w->az_options, w->az_params );

  /*
   *    What solver do we use?
   */

  /* AZ_cg (SPD only!), AZ_gmres (the default),
     AZ_cgs, AZ_tfqmr, AZ_bicgstab, AZ_lu */

  switch( w->solver ) {
  case AZ_GMRES:             w->az_options[AZ_solver] = AZ_gmres; break;
  case AZ_CONJUGATE_GRAD:    w->az_options[AZ_solver] = AZ_cg; break;
  case AZ_CONJUGATE_GRAD_SQ: w->az_options[AZ_solver] = AZ_cgs; break;
  case AZ_TFQMR:             w->az_options[AZ_solver] = AZ_tfqmr; break;
  case AZ_BICGSTAB:          w->az_options[AZ_solver] = AZ_bicgstab; break;
  case AZ_LU:                w->az_options[AZ_solver] = AZ_lu; break;
  default: fprintf( stderr, "Unknown aztec solver!\n" ); abort();
  }

  /*
   *    Do we scale / precondition?
   */

  /*
    AZ_none,
    AZ_Jacobi, AZ_Neumann, AZ_ls, AZ_sym_GS,  (also set options[AZ_poly_ord])
    AZ_dom_decomp
      options[AZ_overlap]
      options[AZ_subdomain_solve]
      options[AZ_reorder]
   */

  w->az_options[AZ_scaling] = AZ_none;
  w->az_options[AZ_precond] = AZ_none;


  /*
   *    Tune GMRES
   */

  /* size of GMRES' krylov subspace */
  w->az_options[AZ_kspace] = w->kss_size;

  /* orthogonalization method */
  w->az_options[AZ_orthog] = AZ_classic;  /* this is the default */
  /* modified is slower, but more numerically stable */
  /* w->az_options[AZ_orthog] = AZ_modified; */


  /*
   *    How do we determine when we're done?
   */

  w->az_options[AZ_max_iter] = w->max_iters;
  w->az_params[AZ_tol] = w->tol;

  /* AZ_r0 (default), AZ_rhs, AZ_Anorm, AZ_noscaled, AZ_sol, AZ_weighted */
  /* max norm */
  w->az_options[AZ_conv] = AZ_sol;
  /* standard two-norm */
  /*  w->az_options[AZ_conv] = AZ_noscaled; */

  /*
   *    How verbose should we be?
   */

  /*    w->az_options[AZ_output] = AZ_all; */
  /*    w->az_options[AZ_output] = 10000; */
  /*    w->az_options[AZ_output] = AZ_warnings; */
  /*    w->az_options[AZ_output] = AZ_none; */

  if ( verbose ) {
    w->az_options[AZ_output] = AZ_none;
  } else {
    w->az_options[AZ_output] = AZ_none;
  }

  /*
   *    Any MPI-ish stuff we need to worry about?
   */

  AZ_set_proc_config( w->az_proc_config, AZ_NOT_MPI );


  /*
   *    Initialize all of the local aztec matrices
   */

  for ( l_part=0; l_part < w->num_local_parts; l_part++ ) {
    allocate_aztec( w, l_part );
  }

}

#endif /* def USE_AZTEC */

/*
 * ----------------------------------------------------------------------------
 */

/* void copy_values_to_svhash( world_t *w, int l_part ) { */
/*   int cnt, l_state, r; */
/*   prec_t *tmpf; */
/*   state_t *st; */

/*   cnt = w->parts[ l_part ].num_states; */
/*   for ( l_state=0; l_state<cnt; l_state++ ) { */
/*     st = &( w->parts[ l_part ].states[ l_state ] ); */
/* #ifdef PREC_IS_FLOAT */
/*     r = med_hash_get_floatp( w->state_val_hash, */
/* 			     st->global_state_index, */
/* 			     &tmpf ); */
/* #else */
/*     r = med_hash_get_doublep( w->state_val_hash, */
/* 			      st->global_state_index, */
/* 			      &tmpf ); */
/* #endif */
/*     assert( r == MH_FOUND ); */
/*     *tmpf = w->parts[ l_part ].values->elts[ l_state ]; */
/*   } */

/* } */


/*
 * ----------------------------------------------------------------------------
 * The end.
 * ----------------------------------------------------------------------------
 */
