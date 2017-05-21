
#include "part_stuff.h"

/*
 * ----------------------------------------------------------------------------
 */

int pick_part_and_wash_it( world_t *w ) {
  int l_part_num;
  float maxheat;

  l_part_num = pick_partition( w );
  if ( l_part_num == -1 ) {
    /* either there was an error, or there isn't any more heat. */
    return 0;
  }

  w->parts_processed++;
  w->parts[ l_part_num ].visits++;

/*   wlog( 1, "-- %.4f\n", part_heat( w, l_part_num ) ); */

/*   fprintf( stderr, "WORKING IN PART %d\n", l_part_num ); */
/*   fprintf( stderr, "-------------------\n" ); */

  do {
    maxheat = value_iterate_partition( w, l_part_num );
/*     fprintf( stderr, "%.4f\n", maxheat ); */
  } while ( fabs(maxheat) > heat_epsilon );

  /* value_iterate_partition will inform foreign processors for us,
     but we need to process incoming messages */
#ifdef USE_MPI
  process_incoming_msgs( w, PROCESS_DONT_BLOCK );
#endif

  update_partition_potentials( w, l_part_num );

#ifdef PART_PQ
  /* when we picked the partition, we pulled it off the heap.
     now that its heat is 0, put it back */
  heap_add( w->part_heap, l_part_num );
#endif

  return 1;
}

int part_available_to_process(world_t *w)
{
    return queue_has_items(w->part_queue);
}

int get_next_part(world_t *w)
{
    int next_partition;
    queue_pop(w->part_queue, &next_partition);
    return next_partition;
}

void add_partition_deps_for_eval(world_t *w, int l_part_changed)
{
    int g_part_changed, g_start_part, l_start_part;
    med_hash_t *dep_part_hash;
    int index1;
    val_t *v;
    
    g_part_changed = lpi_to_gpi( w, l_part_changed );
    
    dep_part_hash = w->parts[ l_part_changed ].my_local_dependents;
    index1 = 0;
    while ( med_hash_iterate( dep_part_hash, &index1, &g_start_part, &v ) )
    {
        l_start_part = gpi_to_lpi( w, g_start_part );
        queue_add(w->part_queue, l_start_part);
    }
    
}


/*
 * ----------------------------------------------------------------------------
 */

void update_partition_potentials( world_t *w, int l_part_changed ) {
  int l_start_state, g_start_state;
  int g_part_changed, g_start_part, l_start_part;
  float *part_heat, tmpheat;
  med_hash_t *dep_part_hash, *dep_state_hash;
  int index1, index2;
  val_t *v;

  /* my heat is now zero (by definition) */
  clear_partition_heat( w, l_part_changed );

  /* this is the global partition number of the partition we just
     updated. */
  g_part_changed = lpi_to_gpi( w, l_part_changed );

  /* We have to process every state that depends on us.  We'll clear
     all the dependent partitions that have a heat link to us.  Then, as
     we're processing each state, we always increment the max.
     Remember, my_local_dependents is a hash_set, and it maps
     g_start_part's to g_start_state's
  */

  dep_part_hash = w->parts[ l_part_changed ].my_local_dependents;

  index1 = 0;
  while ( med_hash_iterate( dep_part_hash, &index1, &g_start_part, &v ) ) {

    l_start_part = gpi_to_lpi( w, g_start_part );

/*     wlog( 1, "  Part %d depends on me\n", l_start_part ); */

    /* Grab a pointer to the  g_start_part -> g_part_changed heatlink */
//    med_hash_get_floatp( w->parts[ l_start_part ].heat_links,
//			 g_part_changed, &part_heat );

    /* Reset the heat link from g_start_part to g_part_changed */
    *part_heat = 0;

    /* make sure that the data is in the odcd cache! */
#ifndef NO_NON_INFO_FRONTIER
    part_check_in( w, l_start_part );
#endif

    /* iterate over all of the states in g_start_part that depend on
       something in g_part_changed */
    dep_state_hash = (med_hash_t *)(v->vptr);
    index2 = 0;
    while ( med_hash_iterate( dep_state_hash, &index2,
			      &l_start_state, &v ) ) {

      g_start_state = lsi_to_gsi( w, l_start_part, l_start_state );

      tmpheat = get_heat( w, l_start_part, l_start_state );

/*       if ( tmpheat != 0 && l_part_changed == 7 ) { */
/* 	wlog( 1, "    Heat of state %d is %.2f\n", l_start_state, tmpheat ); */
/*       } */

      if ( tmpheat > *part_heat ) {
	*part_heat = tmpheat;
      }

    }

    /* 'kay. now we've updated the g_start_part -> g_part_changed heatlink.
       we need to recompute the heat of this partition. */
    compute_part_heat( w, l_start_part );

  }

}

/*
 * ----------------------------------------------------------------------------
 */

void clear_partition_heat( world_t *w, int l_part_num ) {
  int index;
  med_hash_t *m;
  int key;
  float *val;

  /* my heat is zero */
  w->parts[ l_part_num ].heat = 0;
  w->parts[ l_part_num ].primary_heat = 0;

  /* All of my states that depend on other partitions
     have a heat of zero.  Each heat link is therefore zero as well. */
  index = 0;
//  m = w->parts[ l_part_num ].heat_links;
//  while ( med_hash_iterate_float( m, &index, &key, &val ) ) {
//    *val = 0;
//  }
}

/*
 * ----------------------------------------------------------------------------
 */

void compute_part_heat( world_t *w, int l_part_num ) {
  int index;
  float max_heat, *tmpf;
  med_hash_t *m;
  int key;
#ifdef PART_PQ
  int remove_result;
#endif

  /* my maximum heat is the maximum between me and any other partition.
     iterate over all of my heat links, testing the heat. */
  max_heat = 0;
  index = 0;
//  m = w->parts[ l_part_num ].heat_links;
//  while ( med_hash_iterate_float( m, &index, &key, &tmpf ) ) {
//    max_heat = MAX( max_heat, *tmpf );
//  }

  if ( max_heat == w->parts[ l_part_num ].heat ) {
    return;
  }

  w->parts[ l_part_num ].heat =
    MAX( max_heat, w->parts[ l_part_num ].primary_heat );

/*   wlog( 1, "    -> Heat of part %d is now %.6f  (BE = %.6f)\n", */
/* 	l_part_num, w->parts[ l_part_num ].heat, */
/* 	part_bellman_error( w, l_part_num ) */
/* 	); */

/*   if ( max_heat > 1.0 ) { */
/*     wlog( 1, "  WHOA: heat of part %d is now %.2f!\n", */
/* 	  l_part_num, w->parts[ l_part_num ].heat ); */
/*     wlog_flush(); */
/*   } */

#ifdef PART_PQ
  /* update the partition heap! */
  heap_remove( w->part_heap, w->parts[ l_part_num ].my_heap_num,
	       &remove_result );
  heap_add( w->part_heap, l_part_num );
#endif


  /* XXX super debug code */
  
}

prec_t part_bellman_error( world_t *w, int l_part ) {
  prec_t maxheat, tmpheat;
  int state_cnt, l_state;

  maxheat = 0;
  state_cnt = w->parts[ l_part ].num_states;
  for ( l_state = 0; l_state < state_cnt; l_state++ ) {
    tmpheat = std_diff_heat( w, l_part, l_state );
    maxheat = MAX( maxheat, tmpheat );
  }

  return maxheat;
}

/*
 * ----------------------------------------------------------------------------
 */

float part_heat( world_t *w, int l_part_num ) {
  return w->parts[ l_part_num ].heat;
}

/*
 * ----------------------------------------------------------------------------
 */

int pick_partition_seq( world_t *w ) {
  int i, mp;
  prec_t max_priority, tmp;

  max_priority = 0;
  mp = 0;

  for ( i=0; i<w->num_local_parts; i++ ) {
    tmp = part_heat( w, i );
    if ( tmp > max_priority ) {
      max_priority = tmp;
      mp = i;
    }
  }

  if ( max_priority < heat_epsilon ) {
    return -1;
  }

  return mp;
}

int pick_partition_pq( world_t *w ) {
  int l_part_num, result;
  float heat;

  /* the 0 here indicates the position of the element we want to
     peek at.  in this case, it's the head of the head. */
  result = heap_peek( w->part_heap, 0, &l_part_num );
  if ( !result ) {
    wlog( 1, "Whoa!  Error peeking the heap!\n" );
    return -1;
  }

  heat = part_heat( w, l_part_num );

  /*  wlog( 1, "Selected partition %d (%.2f)\n", l_part_num, heat ); */

  if ( verbose && (w->parts_processed % 7800 == 0) ) {
    wlog( 1, "\nPP: %d %.6f!\n", w->parts_processed, heat );
    wlog_flush();
  }
  if ( verbose && (w->parts_processed % 100 == 0) ) {
    wlog( 1, "." );
    wlog_flush();
  }

  if ( heat < heat_epsilon ) {
/*     if ( verbose ) { */
/*       wlog( 1, "\n\nNo more heat. All done.\n\n\n" ); */
/*     } */
    return -1;
  }

  result = heap_pop( w->part_heap, &l_part_num );
  if ( !result ) {
    wlog( 1, "Whoa!  Error popping the heap!\n" );
    return -1;
  }

  w->parts[ l_part_num ].my_heap_num = -1;

  part_check_in( w, l_part_num );

/*   if ( !heap_verify( w->part_heap ) ) { */
/*     my_heap_dump( w, w->part_heap ); */
/*     wlog( 1, "WHOA! HEAP ERROR!\n" ); */
/*     wlog( 1, "\n\n\n" ); */
/*   } */

  return l_part_num;
}

int pick_partition( world_t *w ) {

#ifdef PART_PQ
  return pick_partition_pq( w );
#endif

#ifdef PART_SEQ
  return pick_partition_seq( w );
#endif

}

/*
 * ----------------------------------------------------------------------------
 */

void part_check_in( world_t *w, int l_part_num ) {
  char *data;

  if ( !odcd_enabled() ) {
    return;
  }

  assert( l_part_num >= 0 && l_part_num < w->num_local_parts );

  data = odcd_cache_pull_in( &( w->odcd_cache ),
			     &( w->parts[ l_part_num ].odcd_elem ) );

  w->parts[ l_part_num ].states = (state_t *)data;
}

/*
 * ----------------------------------------------------------------------------
 */
