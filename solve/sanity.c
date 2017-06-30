
#include "sanity.h"

/*
 * ----------------------------------------------------------------------------
 */

void check_heap( world_t *w ) {
  int i, error, supposed_index, heap_index;

  error = 0;
  for ( i=0; i<w->num_local_parts; i++ ) {
    supposed_index = w->parts[ i ].my_heap_num;
    if ( supposed_index == -1 ) {
      continue;
    }

    heap_peek( w->part_heap, supposed_index, &heap_index );
    if ( heap_index != i ) {
      wlog( 1, "    Error: %d should be %d but is %d\n",
	       i, supposed_index, heap_index );
      error = 1;
    }
  }

  if (!error) {
    wlog( 1, "    Heap is ok!\n");
  }
}

/*
 * ----------------------------------------------------------------------------
 */

void check_heat( world_t *w ) {
  int i, heat_cnt;
  float maxheat;

/*   float cur, couldbe, diff;     */
/*   int action; */

  heat_cnt = 0;
  maxheat = w->parts[0].heat;

  for ( i=0; i<w->num_local_parts; i++ ) {
    if ( w->parts[ i ].heat != 0 ) {
      heat_cnt++;
    }
    maxheat = MAX( maxheat, w->parts[i].heat );
  }

  wlog( 1, "\nFound heat in %d partitions!\n", heat_cnt );
  wlog( 1, "  max heat found is %.20f\n", maxheat );

  /* XXX with ODCD, this is much harder! */

/*   heat_cnt = 0; */
/*   maxheat = 0; */
/*   for ( i=0; i<w->num_local_states; i++ ) { */

/*     cur = get_val( w, i ); */

/*     for ( action=0; action<NUM_ACTIONS; action++ ) { */
/*       couldbe = reward_or_value( w, i, action ); */
/*       diff = couldbe - cur; */

/*       if ( diff != 0 ) { */
/* 	heat_cnt++; */
/*       } */

/*       maxheat = MAX( maxheat, diff ); */
      
/*     } */
/*   } */

/*   wlog( 1, "\nFound heat in %d states/actions!\n", heat_cnt ); */
/*   wlog( 1, "  max heat found is %.20f\n", maxheat ); */
}

/*
 * ----------------------------------------------------------------------------
 */

void check_parts( world_t *w ) {
#ifdef ANUJ_CHECK_PARTS
  int l_part_num, index;
  med_hash_t *m;
  float *tmpf;
  int key;

  for (l_part_num=0; l_part_num<w->num_local_parts; l_part_num++) {
    index = 0;
//    m = w->parts[ l_part_num ].heat_links;
//    while ( med_hash_iterate_float( m, &index, &key, &tmpf ) ) {
//      if ( *tmpf != 0.0 ) {
//	wlog( 1, "WHOA! non-zero heat at %d,%d!\n",
//		 l_part_num, key );
//      }
//    }
  }
#endif
}

void check_bcoords( world_t *w ) {
/*   int s, a, v; */
/*   trans_t *t; */
/*   float sum, maxg; */

/*   maxg = 0; */

  /* XXX with ODCD, this is broken! */

/*   for (s=0; s<w->num_local_states; s++) { */
/*     for (a=0; a<NUM_ACTIONS; a++) { */
/*       t = &( w->states[ s ].tps[a] ); */

/*       if ( t->g_tothe_tau_time < 0 ) { */
/* 	// skip things that exit the space. */
/* 	continue; */
/*       } */

/*       if ( t->g_tothe_tau_time > maxg ) { */
/* 	maxg = t->g_tothe_tau_time; */
/*       } */

/*       sum = 0; */
/*       for (v=0; v<D_PLUS_ONE; v++) { */
/* 	sum += t->bary_coords.coords[v]; */
/*       } */

/*       if ( sum > 1.00001 || sum < 0.99999 ) { */
/* 	wlog( 1, "    WHOA! - bc sum for %d,%d is %.10f!\n", */
/* 		 s, a, sum ); */
/* 	wlog( 1, "      reward: %.2f, tau_time: %.10f\n", */
/* 		 t->reward, t->g_tothe_tau_time ); */
/* 	for (v=0; v<D_PLUS_ONE; v++) { */
/* 	  wlog( 1, "      %.10f\n", t->bary_coords.coords[v] ); */
/* 	} */
/*       } */
/*     } */
/*   } */

/*   wlog( 1, "SANITY: Max tau time: %.6f\n", maxg ); */
}

/*
 * ----------------------------------------------------------------------------
 */

/* void dep_stats( world_t *w ) { */
/*   int fs_cnt, max_fs, min_fs, i, found; */
/*   float avg_fs; */

/*   avg_fs = 0; */
/*   max_fs = 0; */
/*   min_fs = NUM_STATES; */
/*   found = 0; */

/*   for (i=0; i<NUM_PARTITIONS; i++) { */

/*     fs_cnt = w->parts[ i ].foreign_state_cnt; */
/*     if ( fs_cnt > max_fs ) max_fs = fs_cnt; */
/*     if ( fs_cnt < min_fs ) min_fs = fs_cnt; */
/*     avg_fs += w->parts[ i ].foreign_state_cnt; */

/*     if ( w->parts[ i ].primary_heat ) { */
/*       found = 1; */
/*       //      wlog( 1, "Partition %d contains primary heat (%f, %f)\n", i, */
/*       //	       w->parts[ i ].heat, w->parts[ i ].primary_heat ); */
/*     } */
/*   } */

/*   avg_fs /= NUM_PARTITIONS; */

/*   wlog( 1, "\nDependency stats (max,min,avg): %d, %d, %.2f\n\n", */
/* 	   max_fs, min_fs, avg_fs ); */

/*   if ( found == 0 ) { */
/*     wlog( 1, "\n\n\nWhoa! No primary rewards found!!\n\n\n" ); */
/*     exit(0); */
/*   } */
/* } */

/*
 * ----------------------------------------------------------------------------
 */

void compute_tc_wc_unv( world_t *w, float *tc, float *wc, int *unv ) {
  int i, total_u;
  long total_w, total_v;

  total_w = 0;
  total_v = 0;
  total_u = 0;

  for (i=0; i<w->num_local_parts; i++) {
    total_v += w->parts[ i ].visits;
    total_w += w->parts[ i ].washes;
    // this is the count of unvisited partitions
    if ( w->parts[ i ].visits == 0 ) {
      total_u ++;
    }
  }

  *tc = (float)total_v / (float)(w->num_local_parts);
  *wc = (float)total_w / (float)(w->num_local_parts);
  *unv = total_u;
}

/* void compute_dep_stats( world_t *w, float *sdp, float *pdp ) { */
/*   long total_sdp, total_pdp; */
/*   int i; */

/*   total_sdp = 0; */
/*   total_pdp = 0; */

/*   for (i=0; i<w->num_local_parts; i++) { */
/*     total_sdp += w->parts[ i ].foreign_state_cnt; */
/*     total_pdp += w->parts[ i ].foreign_part_cnt; */
/*   } */

/*   *sdp = (float)total_sdp / (float)(w->num_local_parts); */
/*   *pdp = (float)total_pdp / (float)(w->num_local_parts); */
/* } */

void final_stats( world_t *w, float iter_time, float coord_time ) {
  float ram, tc, wc;
  int unv;
  //  float sdp, pdp;

  compute_tc_wc_unv( w, &tc, &wc, &unv );
  //  compute_dep_stats( w, &sdp, &pdp );

  ram = sizeof(world_t);
  ram +=
    sizeof(state_t) * w->num_local_states +
    sizeof(part_t) * w->num_local_parts;
    
/*   ram += sdp * sizeof(int); // state dependencies */
/*   ram += pdp * sizeof(f_part_t); // state dependencies */

  wlog( 1, "       States: %d\n", w->num_local_states );
  wlog( 1, "   Partitions: %d\n", w->num_local_parts );
  wlog( 1, "States / part: %f\n",
	  (float)(w->num_local_states) / (float)(w->num_local_parts));
  wlog( 1, "    Iter Time: %.6f\n", iter_time );
  wlog( 1, "   Coord Time: %.6f\n", coord_time );

#ifdef USE_MPI
  wlog( 1, "  Number of messages: %d\n", w->messages_sent );
  wlog( 1, "Total message volume: %d\n", w->message_size );
  wlog( 1, "Average message size: %.2f\n",
	(float)(w->message_size) / (float)(w->messages_sent) );
#endif


  wlog( 1, "           RAM: %f\n", ram );
  wlog( 1, "            Tc: %f\n", tc );
  wlog( 1, "            Wc: %f\n", wc );
  wlog( 1, "     Unvisited: %d (%.2f)\n", unv,
	(float)unv/(float)w->num_local_parts );

  if ( odcd_enabled() ) {
    wlog( 1, "ODCD Hit ratio: %.2f (%d/%d)\n",
	  100 * odcd_hit_ratio( &(w->odcd_cache) ),
	  w->odcd_cache.cache_hits, w->odcd_cache.cache_tries );
    wlog( 1, "ODCD recompute: %.2f (%d/%d)\n",
	  (float)(w->odcd_cache.cache_tries -
		  w->odcd_cache.cache_hits)/
	  (float)(w->num_local_parts),
	  w->odcd_cache.cache_tries - w->odcd_cache.cache_hits,
	  w->num_local_parts );
  }

/*   wlog( 1, "        |SDp|: %f\n", sdp ); */
/*   wlog( 1, "        |PDp|: %f\n", pdp ); */

}

/*
 * ----------------------------------------------------------------------------
 */

void my_heap_dump_r( world_t *w, heap *h, int pos, int depth ) {
  int lchild, rchild, i;

  lchild = pos*2 + 1;
  rchild = pos*2 + 2;
  if ( pos >= h->numitems ) { return; }

  for (i=0; i<depth; i++) {
    wlog( 1, "  " );
  }

  wlog( 1, "  %d: %d\t%.2f\n", pos, h->items[pos],
	   w->parts[ h->items[pos] ].heat );

  my_heap_dump_r( w, h, lchild, depth+1 );
  my_heap_dump_r( w, h, rchild, depth+1 );

}

void my_heap_dump( world_t *w, heap *h ) {
  my_heap_dump_r( w, h, 0, 0 );
}

/*
 * ----------------------------------------------------------------------------
 */

void sanity_checks( world_t *w ) {
  time_t start, end;

  if ( verbose ) {
    wlog( 1, "\n\n" );
    wlog( 1, "---------------------------------------------------\n" );
    wlog( 1, "Sanity checks:\n" );
  }
  start = time( NULL );

  if ( verbose ) {
    wlog( 1, "  Checking barycentric coordinates...\n" );
  }
  check_bcoords( w );

  if ( verbose ) {
    wlog( 1, "  Checking heap...\n" );
  }
  check_heap( w );

  if ( verbose ) {
    wlog( 1, "  Checking partitions...\n" );
  }
  check_parts( w );

  //  if ( verbose ) {
  //    wlog( 1, "  Checking foreign dependencies...\n" );
  //  }
  //  dep_stats( w );

  final_stats( w, 0, 0 );

  end = time( NULL );

  if ( verbose ) {
    wlog( 1, "Took %d seconds\n", (int)(end-start) );
    wlog( 1, "---------------------------------------------------\n" );
    wlog( 1, "\n\n" );
  }

}

/*
 * ----------------------------------------------------------------------------
 * The end.
 * ----------------------------------------------------------------------------
 */
