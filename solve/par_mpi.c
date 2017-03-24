
#include "par_mpi.h"

#ifdef USE_MPI

/*
 * ----------------------------------------------------------------------------
 */

void mpi_init( world_t *w, int *argc, char **argv[] ) {
  MPI_Init( argc, argv );
  MPI_Comm_size( MPI_COMM_WORLD, &(w->num_procs) );
  MPI_Comm_rank( MPI_COMM_WORLD, &(w->my_proc_num) );
  MPI_Get_processor_name( w->proc_name, &(w->proc_namelen) );
}

/*
 * ----------------------------------------------------------------------------
 */

void randomly_assign_partitions( world_t *w, int *lpc ) {
  int i, num_local_parts, owning_proc;

  // initialize the partition-to-processor mapping
  // this could be either a modulus sort of thing, or a random thing.
  // here, we select a random thing.

  srand( 42 );
  num_local_parts = 0;

  for ( i=0; i<w->num_global_parts; i++ ) {
    // assign this partition to a processor
    // generate a random number between 0 and num_procs-1, inclusive.
    owning_proc = (int)( (float)(w->num_procs) * (rand() / (RAND_MAX+1.0)) );
    w->part_to_proc[i].owning_processor = owning_proc;

    // if it's assigned to us, keep track of a local array index
    if ( owning_proc == w->my_proc_num ) {
      w->part_to_proc[i].local_part_num = num_local_parts;
      num_local_parts++;
    } else {
      w->part_to_proc[i].local_part_num = -1;
    }
  }

  *lpc = num_local_parts;
}

/* void attractor_assign_partitions( world_t *w, int *lpc ) { */
/*   int n, i, d, index; */
/*   int num_local_parts, owning_proc; */
/*   grid_coord_t *attractors, pc; */
/*   int g_part_num, mindist, tmpdist, tie_cnt; */

/*   attractors = (grid_coord_t *)malloc( w->num_procs * */
/*                                               num_attractors * */
/*                                        sizeof(grid_coord_t) ); */
/*   if ( attractors == NULL ) { */
/*     wlog( 1, "Couldn't allocate attractors!\n" ); */
/*     exit( 0 ); */
/*   } */

/*   srand( 242 ); */

/*   // Generate some attractors.  We have num_attractors per process. */

/*   index = 0; */
/*   for ( n=0; n<w->num_procs; n++ ) { */
/*     for ( i=0; i<num_attractors; i++ ) { */
/*       /\*       wlog( 1, "Attractor %d: ", index ); *\/ */
/*       for ( d=0; d<DIMENSIONS; d++ ) { */
/*         // generate a number between 0 and the number of partition */
/*         // divs in this dimension */
/*         attractors[ index ].coords[d] = */
/*           (int)( (float)(w->pd[d].divs) * (rand() / (RAND_MAX+1.0)) ); */
/*         /\* wlog( 1, "%d ", attractors[ index ].coords[d] ); *\/ */
/*       } */
/*       /\*       wlog( 1, "\n" ); *\/ */
/*       index++; */
/*     } */
/*   } */

/*   // initialize the partition-to-processor mapping */
/*   // this could be either a modulus sort of thing, or a random thing. */
/*   // here, we select a random thing. */

/*   num_local_parts = 0; */
/*   memset( &pc, 0, sizeof(grid_coord_t) ); */

/*   do { */

/*     g_part_num = partcoord_to_partnum( w->pd, &pc ); */

/*     // assign this partition to a processor */
/*     // figure out which attractor it is closest to. */

/*     mindist = 1999999999; */
/*     owning_proc = 0; */
/*     index = 0; */
/*     tie_cnt = 1; */

/*     /\*     wlog( 1, "For part %d:\n", g_part_num ); *\/ */

/*     for ( n=0; n<w->num_procs; n++ ) { */
/*       for ( i=0; i<num_attractors; i++ ) { */
/*         tmpdist = grid_manhattan_dist( &(w->s), &( attractors[index] ), &pc ); */
/*         /\* wlog( 1, " dist to attractor %d = %d (mindist=%d)\n", *\/ */
/*         /\*       index, tmpdist, mindist ); *\/ */

/*         /\* if there's a tie, we need to roll a die.  Ties happen a */
/*            lot, and if we don't distribute, processor 0 will get */
/*            highly favored *\/ */
/*         if ( tmpdist == mindist ) { */
/*           tie_cnt++; */
/*           if ( rand()/(float)(RAND_MAX-1) <= 1.0/((float)tie_cnt) ) { */
/*             owning_proc = n; */
/*           } */

/*         } else if ( tmpdist < mindist ) { */
/*           mindist = tmpdist; */
/*           owning_proc = n; */
/*           tie_cnt = 1; */
/*         } */
/*         index++; */
/*       } */
/*     } */

/*     /\*     wlog( 1, " -> assigned to %d\n", owning_proc ); *\/ */

/*     w->part_to_proc[ g_part_num ].owning_processor = owning_proc; */

/*     // if it's assigned to us, keep track of a local array index */
/*     if ( owning_proc == w->my_proc_num ) { */
/*       w->part_to_proc[ g_part_num ].local_part_num = num_local_parts; */
/*       num_local_parts++; */
/*     } else { */
/*       w->part_to_proc[ g_part_num ].local_part_num = -1; */
/*     } */

/*   } while ( iterate_over_parts( w->pd, &pc ) ); */

/*   *lpc = num_local_parts; */

/*   free( attractors ); */
/* } */

void assign_and_count_local_parts( world_t *w, int *lpc ) {
  
/*   if ( attractors_enabled() ) { */
/*     attractor_assign_partitions( w, lpc ); */
/*   } else { */
    randomly_assign_partitions( w, lpc );
/*   } */
}

/*
 * ----------------------------------------------------------------------------
 */

/* void count_local_states( world_t *w, int *lsc ) { */
/*   int l_part_num, num_local_states; */

/*   num_local_states = 0; */

/*   for ( l_part_num=0; l_part_num<w->num_local_parts; l_part_num++ ) { */
/*     num_local_states += num_states_in_part( w, l_part_num ); */
/*   } */

/*   *lsc = num_local_states; */
/* } */

/*
 * ----------------------------------------------------------------------------
 */

int is_partition_local( world_t *w, int g_part_num ) {
  if ( w->part_to_proc[ g_part_num ].owning_processor == w->my_proc_num ) {
    return 1;
  }
  return 0;
}

/*
 * ----------------------------------------------------------------------------
 */

// translate a global partition index to a local partition index
int gpi_to_lpi( world_t *w, int g_part_num ) {
  int r, op;

  r = w->part_to_proc[ g_part_num ].local_part_num;
  op = w->part_to_proc[ g_part_num ].owning_processor;

  if ( g_part_num < 0 ||
       g_part_num > w->num_global_parts ||
       w->my_proc_num != op ) {
    wlog ( 1, "WHOA! GPI_TO_LPI ERROR: checking %d, im %d, says %d, ret %d!\n",
	   g_part_num, w->my_proc_num, op, r );
  }

  return r;
}

int lpi_to_gpi( world_t *w, int l_part_num ) {
  return w->parts[ l_part_num ].g_part_num;
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

int send_msg( world_t *w, void *data, int length, int proc ) {
  mpi_req_stuff *mrs;
  MPI_Status s;

  //  wlog( 1, "s" );

  mrs = &( w->mpi_reqs[ proc ] );
  
  if ( mrs->in_use == 1 ) {
    //    wlog( 1, "  waiting..." );
    MPI_Wait( &(mrs->mpi_req), &s );
    //    wlog( 1, " done.\n" );
  }

  // 'kay.  we need to treat this buffer as volatile.
  if ( mrs->buf_size < length ) {
    if ( mrs->buf != NULL ) {
      free( mrs->buf );
    }
    mrs->buf = (char *)malloc( length );
    if ( mrs->buf == NULL ) {
      wlog( 1, "Whoa! Error allocating send buffer!\n" );
      exit( 0 );
    }
  }

  memcpy( mrs->buf, data, length );

  MPI_Isend( mrs->buf, length, MPI_BYTE, proc, ZETAG, MPI_COMM_WORLD,
	     &(mrs->mpi_req) );
  w->messages_sent++;
  w->message_size += length;

  mrs->in_use = 1;

  //  wlog( 1, "done\n" );

  return 1;
}


int are_msgs_waiting( void ) {
  int flag;
  MPI_Status status;

  MPI_Iprobe( MPI_ANY_SOURCE, ZETAG, MPI_COMM_WORLD, &flag, &status );
  if ( !flag ) {
    // no message is waiting.
    return 0;
  }
  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void process_incoming_msgs( world_t *w, int block ) {
  int i;

  // process up to num_procs messages
  for ( i=0; i<w->num_procs; i++ ) {
    if ( !process_mpi_msgs( w, block ) ) {
      // no messages waiting.
      break;
    }
    // we might block on the first process_mpi_msgs, but we don't
    // want to block after that.
    block = PROCESS_DONT_BLOCK;
  }
}

int process_mpi_msgs( world_t *w, int block ) {
  char *c;
  int i;
  MPI_Status status;

  if ( block == PROCESS_DONT_BLOCK ) {
    // if you combine these two statements, the else isn't right. 
    if ( !are_msgs_waiting() ) {
      return 0;
    }
/*   } else { */
/*     wlog( 1, "About to enter blocking receive...\n" ); */
  }

  MPI_Recv( w->recv_buf, w->max_recv_size, MPI_BYTE, MPI_ANY_SOURCE,
	    ZETAG, MPI_COMM_WORLD, &status );

  //  wlog( 1, "r" );

  switch ( w->recv_buf[0] ) {
  case MSG_PART_UPDATE_MSG:
    process_partition_update( w, (part_update_msg_t *)(w->recv_buf) );
    break;

  case MSG_LTFSD:
    process_ltfsd_msg( w, (ltfsd_msg_t *)(w->recv_buf) );
    break;

  case MSG_LTFSD_DONE:
    process_ltfsd_done_msg( w );
    break;

  case MSG_LTFSD_GO:
    wlog( 1, "Got ltfsd go msg!\n" );
    w->ltfsd_myturn = 1;
    break;

  case MSG_LTFSD_TERMINATE:
    process_ltfsd_terminate_msg( w );
    break;

  case MSG_PROC_DONE:
    process_proc_done_msg( w );
    break;

  case MSG_PROC_UNDONE:
    process_proc_undone_msg( w );
    break;

  case MSG_PROC_TERMPING:
    process_termping_msg( w );
    break;

  case MSG_PROC_TERMINATE:
    process_terminate_msg( w );
    break;

  default:
    wlog( 1, "Unknown message %d!\n", w->recv_buf[0] );
    c = (char *)(&( w->recv_buf[0] ));
    for ( i=0; i<20; i++ ) {
      wlog( 1, "  c[%d]=%d\n", i, c[i] );
    }
  }

  return 1;
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void send_proc_done_msg( world_t *w ) {
  int msg;

  msg = MSG_PROC_DONE;

/*   wlog( 1, "I think I'm done!\n" ); */

  // we always send done messages to processor 0!

  if ( w->my_proc_num == 0 ) {
    // some MPI implementations don't support messages to self.
    process_proc_done_msg( w );

  } else {
    send_msg( w, &msg, sizeof(msg), 0 );
  }
}

void process_proc_done_msg( world_t *w ) {
  int i;

  // this should only be executing on processor 0.
  w->processing_done_cnt++;

/*   wlog( 1, "Someone is done: processing_done_cnt is %d\n", */
/* 	w->processing_done_cnt ); */

  if ( w->processing_done_cnt != w->num_procs ) {
    // not everyone is done.  That's fine.  Nothing more needs to happen.
    return;
  }

  // Ok.  we're in a state where everyone claims that they are done.

  wlog( 1, "Everyone says they're done.  Term pinging...\n" );

  for ( i=1; i<w->num_procs; i++ ) {
    send_termping_msg( w, i );
  }

  w->term_state = 1;
  w->term_ping = 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void send_proc_undone_msg( world_t *w ) {
  int msg;

  msg = MSG_PROC_UNDONE;

/*   wlog( 1, "I think I'm undone!\n" ); */

  // we always send done messages to processor 0!

  if ( w->my_proc_num == 0 ) {
    // some MPI implementations don't support messages to self. :(
    process_proc_undone_msg( w );

  } else {
    send_msg( w, &msg, sizeof(msg), 0 );
  }
}

void process_proc_undone_msg( world_t *w ) {
  w->processing_done_cnt--;
  w->term_state = 0; 
  w->term_ping = 0;

/*   wlog( 1, "Someone is undone: processing_done_cnt is %d\n", */
/* 	w->processing_done_cnt ); */

  if ( w->processing_done_cnt < 0 ) {
    wlog( 1, "Whoa! bad done_cnt!\n" );
    w->processing_done_cnt = 0;
  }
}

/*
 * ----------------------------------------------------------------------------
 */

void send_terminate_msg( world_t *w, int proc ) {
  int msg;
  msg = MSG_PROC_TERMINATE;
  wlog( 1, "Telling %d to terminate!\n", proc );
  send_msg( w, &msg, sizeof(msg), proc );
}

void process_terminate_msg( world_t *w ) {
  wlog( 1, "Received terminate\n" );
  w->terminate = 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void send_termping_msg( world_t *w, int proc ) {
  int msg;

  wlog( 1, "Sending termping to %d\n", proc );

  msg = MSG_PROC_TERMPING;
  send_msg( w, &msg, sizeof(msg), proc );
}

void process_termping_msg( world_t *w ) {
  int msg, i;

  wlog( 1, "Received termping message\n" );

  if ( w->my_proc_num != 0 ) {
    wlog( 1, "Sending termping to 0\n" );
    msg = MSG_PROC_TERMPING;
    send_msg( w, &msg, sizeof(msg), 0 );
    return;
  }

  // this can happen if the termping is late.
  if ( w->term_state != 1 ) {
    return;
  }

  w->term_ping++;

  if ( w->term_ping < w->num_procs ) {
    // not everyone has responded to the ping.
    return;
  }

  // everyone has responded affirmatively to the termping.
  // really kill everyone.

  wlog( 1, "Everyone responded to the ping.  Terminating...\n" );

  for ( i=1; i<w->num_procs; i++ ) {
    send_terminate_msg( w, i );
  }

  w->terminate = 1;
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void coordinate_dependencies( world_t *w ) {
  int index1, imdone;

  if ( w->my_proc_num == 0 ) {
    w->ltfsd_myturn = 1;
  }

  imdone = 0;
  index1 = 0;

  while ( 1 ) {
    if ( w->ltfsd_done == 1 ) {
      break;
    }

    // we only want to block when we're done.
    process_mpi_msgs( w, imdone );

    if ( w->ltfsd_myturn && !imdone ) {
      if ( !send_ltfsd_msg( w, &index1 ) ) {
	imdone = 1;
	send_ltfsd_done_msg( w );
      }
    }

  }
}

/*
 * ----------------------------------------------------------------------------
 */

// ltfsd = local to foreign state dependency

int send_ltfsd_msg( world_t *w, int *index1 ) {
  int i, index2;
  int foreign_proc, state, elts_todo, max_eltcnt;
  int *states, cstate, msgsize;
  val_t *v;
  med_hash_t *m;
  ltfsd_msg_t *msg;

  if ( ! med_hash_iterate( w->fproc_data,
			   index1, &foreign_proc, &v ) ) {
    return 0;
  }

  m = (med_hash_t *)v->vptr;
  wlog( 1, "Sending ltfsd: tell processor %d about %d states\n",
	foreign_proc, m->nelts );

  elts_todo = m->nelts;
  max_eltcnt = 4096;

  msgsize = sizeof(ltfsd_msg_t) + (max_eltcnt * sizeof(int));
  msg = (ltfsd_msg_t *)malloc( msgsize );
  if ( msg == NULL ) {
    wlog( 1, "WARGH! couldn't allocate statemsg!\n" );
    exit( 0 );
  }

  states = (int *)(((char *)msg) + sizeof(ltfsd_msg_t));
  index2 = 0;

  // here, we chunk the message into sets of max_eltcnt.
  // That's because MPI has some limitations on the size of
  // messages it can send / receive.

  while ( elts_todo > max_eltcnt ) {

    msg->msg_type = MSG_LTFSD;
    msg->originating_proc = w->my_proc_num;
    msg->state_count = max_eltcnt;

    cstate = 0;
    for ( i=0; i<max_eltcnt; i++ ) {
      if ( !med_hash_iterate( m, &index2, &state, &v ) ) {
	wlog( 1, "  Error in counting! Couldn't iterate!\n" );
	break;
      }
      states[cstate] = state;
      cstate++;
    }

    // actually send the message!
    send_msg( w, msg, msgsize, foreign_proc );

    elts_todo -= max_eltcnt;
  }

  msg->msg_type = MSG_LTFSD;
  msg->originating_proc = w->my_proc_num;
  msg->state_count = elts_todo;

  cstate = 0;
  for ( i=0; i<elts_todo; i++ ) {
    if ( !med_hash_iterate( m, &index2, &state, &v ) ) {
      wlog( 1, "  Error in counting! Couldn't iterate!\n" );
      break;
    }
    states[cstate] = state;
    cstate++;
  }

  msgsize = sizeof(ltfsd_msg_t) + (elts_todo * sizeof(int));

  // actually send the message!
  send_msg( w, msg, msgsize, foreign_proc );

  free( msg );

  return 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void process_ltfsd_msg( world_t *w, ltfsd_msg_t *msg ) {
  int foreign_proc, num_states, i, *states;
  int g_state_num, g_part_num, l_part_num;

  // so.  we just received a big statemsg.
  foreign_proc = msg->originating_proc;
  num_states = msg->state_count;
  states = (int *)(((char *)msg) + sizeof(ltfsd_msg_t));

  wlog( 1, "Received ltfsd: %d is telling me about %d states\n",
	foreign_proc, num_states );

  for ( i=0; i<num_states; i++ ) {
    g_state_num = states[i];
    g_part_num = state_to_partnum( w, g_state_num );
    l_part_num = gpi_to_lpi( w, g_part_num );

/*     wlog( 1, "  %d (%d) in lpart %d has a dep\n", */
/* 	  g_state_num, l_part_num ); */

    med_hash_set_add( w->parts[ l_part_num ].my_foreign_dependents,
		      foreign_proc, g_state_num );
  }
}

/*
 * ----------------------------------------------------------------------------
 */

void send_ltfsd_done_msg( world_t *w ) {
  int msg;

  msg = MSG_LTFSD_DONE;

  wlog( 1, "Sending ltfsd done msg to proc 0\n" );

  if ( w->my_proc_num == 0 ) {
    // some MPI implementations don't support messages to self.  
    process_ltfsd_done_msg( w );

  } else {
    send_msg( w, &msg, sizeof(msg), 0 );
  }

  if ( w->my_proc_num < w->num_procs-1 ) {

    msg = MSG_LTFSD_GO;

    wlog( 1, "Sending ltfsd go msg to proc %d\n",
	  w->my_proc_num+1 );

    send_msg( w, &msg, sizeof(msg), w->my_proc_num+1 );
  }
}

void process_ltfsd_done_msg( world_t *w ) {
  int i;

  // this should only be running on processor 0!

  w->ltfsd_done_cnt += 1;
  wlog( 1, "Received ltfsd done msg: ltfsd_done_cnt = %d\n",
	w->ltfsd_done_cnt );

  if ( w->ltfsd_done_cnt != w->num_procs ) {
    return;
  }

  wlog( 1, "Everyone says they're done; terminating ltfsd\n" );

  for ( i=1; i<w->num_procs; i++ ) {
    send_ltfsd_terminate_msg( w, i );
  }

  w->ltfsd_done = 1;
}

/*
 * ----------------------------------------------------------------------------
 */

void send_ltfsd_terminate_msg( world_t *w, int foreign_proc ) {
  int msg;

  msg = MSG_LTFSD_TERMINATE;
  wlog( 1, "Sending ltfsd terminate msg: telling proc %d\n", foreign_proc );
  send_msg( w, &msg, sizeof(msg), foreign_proc );
}

void process_ltfsd_terminate_msg( world_t *w ) {
  wlog( 1, "Processing ltfsd terminate msg\n" );
  w->ltfsd_done = 1;
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void send_partition_update( world_t *w, int l_part_num ) {
  int g_part_num, foreign_proc, sc_count, sc_index;
  int hashindex1, hashindex2, msgsize;
  int g_state_num;
  val_t *v;
  med_hash_t *m1, *m2;
  part_update_msg_t *msg;
  state_changed_t *sc;

  // alright.  We've just finished updating l_part_num.  We need to
  // tell all of the foreign processors that depend on us about the
  // changes.

  g_part_num = lpi_to_gpi( w, l_part_num );

/*   wlog( 1, "Sending partition update: part=%d\n", g_part_num ); */

  // send a message to every processor that cares about this partition

  hashindex1 = 0;
  m1 = w->parts[ l_part_num ].my_foreign_dependents;
  while ( med_hash_iterate( m1, &hashindex1, &foreign_proc, &v ) ) {

    // Send a msg to foreign_proc.  Tell it the new value of all of the
    // states that it cares about.
    m2 = (med_hash_t *)v->vptr;
    sc_count = m2->nelts;

/*     wlog( 1, "  -> proc=%d, sc=%d\n", foreign_proc, sc_count ); */

    msgsize = sizeof(part_update_msg_t) +
      sc_count * sizeof(state_changed_t);

    msg = (part_update_msg_t *)malloc( msgsize );
    if ( msg == NULL ) {
      wlog( 1, "WARGH! Couldn't allocate part_update_msg_t!\n" );
      exit( 0 );
    }

    // fill out the message header

    msg->msg_type = MSG_PART_UPDATE_MSG;
    msg->originating_proc = w->my_proc_num;
    msg->g_part_changed = g_part_num;
    msg->state_changed_count = sc_count;

    sc = (state_changed_t *)( ((char *)msg) + sizeof(part_update_msg_t) );

    sc_index = 0;
    hashindex2 = 0;
    while ( med_hash_iterate( m2, &hashindex2, &g_state_num, &v ) ) {
      sc[ sc_index ].global_state_num = g_state_num;
      sc[ sc_index ].new_val = get_val( w, g_state_num );
      sc_index++;
    }

    // send the message
    send_msg( w, msg, msgsize, foreign_proc );

    // clean up
    free( msg );
  }

}

/*
 * ----------------------------------------------------------------------------
 */

void process_partition_update( world_t *w, part_update_msg_t *msg ) {
  int index, g_start_part, l_start_part, r;
  int g_start_state, l_start_state;
  val_t *vptr;
  val_t v;
  med_hash_t *part_hash, *state_hash;
  int part_hashindex, state_hashindex;
  float tmpheat, *maxheat, *newvalp;
  state_changed_t *changed_states;
  int num_states, g_part_changed;

  // unpack the message

  g_part_changed = msg->g_part_changed;
  num_states = msg->state_changed_count;
  changed_states =
    (state_changed_t *)( ((char *)msg) + sizeof(part_update_msg_t) );

  //  wlog( 1, "Received partition update: proc=%d, part=%d, sc=%d\n",
  //	msg->originating_proc, g_part_changed, num_states );

  // ok.  g_part_changed is the global index of the partition that
  // has changed.  first, update the values of all of the states that
  // have changed.

  for ( index=0; index<num_states; index++ ) {
    // we need the value of all of these states.
    r = med_hash_get_floatp( w->foreign_state_val_hash,
			     changed_states[index].global_state_num,
			     &newvalp );
    assert( r == MH_FOUND );
    *newvalp = changed_states[index].new_val;
  }

  // This loop will iterate over all of the partitions that contain
  // any state that depends on g_part_changed.

  r = med_hash_get( w->endpart_to_startpart, g_part_changed, &v );
  assert( r == MH_FOUND );

  part_hash = (med_hash_t *)v.vptr;
  part_hashindex = 0;

  while ( med_hash_iterate( part_hash, &part_hashindex,
			    &g_start_part, &vptr ) ) {

    l_start_part = gpi_to_lpi( w, g_start_part );

    // Grab a pointer to the  g_start_part -> g_part_changed heatlink
    r = med_hash_get_floatp( w->parts[ l_start_part ].heat_links,
			     g_part_changed, &maxheat );
    assert( r == MH_FOUND );

    // Reset the heat link from g_start_part to g_part_changed
    *maxheat = 0;

    // make sure that the data is in the odcd cache!
#ifndef NO_NON_INFO_FRONTIER
    part_check_in( w, l_start_part );
#endif

    // Inside of g_part_changed there are many states that changed
    // value.  Within each partition we have a mapping which tells us
    // which states within the partition depended upon any state in
    // the partition that changed.

    med_hash_get( w->parts[ l_start_part ].endpart_to_startstate,
		  g_part_changed, &v );

    state_hash = (med_hash_t *)(v.vptr);
    state_hashindex = 0;
    while ( med_hash_iterate( state_hash, &state_hashindex,
			      &l_start_state, &vptr ) ) {

      // so.  l_start_state depends upon some value in g_part_changed.
      // figure out the heat of l_start_state, and update the
      // heatlink between l_start_part and g_part_changed.

      g_start_state = lsi_to_gsi( w, l_start_part, l_start_state );

      //      tmpheat = get_heat( w, g_start_state,
      //			  w->parts[ l_start_part ].states[l_start_state].tps );

      tmpheat = get_heat( w, l_start_part, l_start_state );

      if ( tmpheat > *maxheat ) {
	*maxheat = tmpheat;
      }

    }

    // ok.  We've updated all of the states in the fsc, and we've
    // updated the heatlink between l_start_part and g_part_changed.
    // Recompute the heat of l_start_part, and then reprioritize it.

    compute_part_heat( w, l_start_part );
  }

  // done. whew!
}

/*
 * ----------------------------------------------------------------------------
 */

void compute_max_mesg_size( world_t *w ) {
  int max_mesg_size, foreign_proc, index1;
  val_t *v;
  med_hash_t *m;

  // Receiving partition updates happens a lot.  We don't want to
  // malloc and free all of the time, and we know what the size
  // of the largest message we'll ever have to deal with is.
  // So, we just statically allocate it.

  index1 = 0;
  max_mesg_size = 0;
  while( med_hash_iterate( w->fproc_data,
			   &index1, &foreign_proc, &v ) ) {
    m = (med_hash_t *)v->vptr;
    if ( m->nelts > max_mesg_size ) {
      max_mesg_size = m->nelts;
    }
  }

  max_mesg_size *= 2;

  wlog( 1, "Max mesg size = %d\n", max_mesg_size );

  w->max_recv_size = max_mesg_size * sizeof(state_changed_t) +
    sizeof(part_update_msg_t);

  w->recv_buf = (int *)malloc( w->max_recv_size );
  if ( w->recv_buf == NULL ) {
    wlog( 1, "Couldn't allocate receive buffer!\n" );
    exit( 0 );
  }

}

/*
 * ----------------------------------------------------------------------------
 */

#endif // defined USE_MPI

