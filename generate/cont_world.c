
#include "cont_world.h"

/*
 * ----------------------------------------------------------------------------
 */

void init_stuff( space_t *s ) {
  initialize_space_desc( s );
  compute_dim_info( s );
}

/*
 * ----------------------------------------------------------------------------
 */

void compute_dim_info( space_t *s ) {
  int i, divs, size, total_states;
  dim_t *d;

  size = 1;
  total_states = 1;

  for ( i=0; i<DIMENSIONS; i++ ) {
    d = &( s->dim_desc[i] );

    if ( d->disc_type < 0 || d->disc_type > NUM_DISC_TYPES ) {
      wlog( 1, "Bad discretization type for dim %d!\n", i );
      exit( 0 );
    }

    divs = d->divs;

    if ( d->options & DIM_OPT_CLIP ) {
      divs--;
    }

    d->range = d->max - d->min;
    d->halfrange = d->range / 2;
    d->size = d->range / divs;
    d->unisize = 1.0 / (float)divs;
    d->midpoint = d->min + d->halfrange;

    size *= d->divs;
    total_states *= d->divs;

    if ( verbose ) {
      wlog( 1, "Dim %d:\n", i );
      wlog( 1, "     Divs: %d\n", d->divs );
      wlog( 1, "Disc type: %s\n", disc_names[ d->disc_type ] );
      wlog( 1, "      Max: %f\n", d->max );
      wlog( 1, "      Min: %f\n", d->min );
      wlog( 1, "      Mid: %f\n", d->midpoint );
      wlog( 1, "       HR: %f\n", d->halfrange );
      wlog( 1, "     Size: %f\n", d->size );
      wlog( 1, "  Unisize: %f\n", d->unisize );
    }
  }

  // this number is only used for the state_to_grid function.  We want it
  // to represent the "volume" of all but the last (i.e., 0...n-2) dimensions.
  size /= d->divs;

  s->total_space_size = size;
  s->total_states = total_states;
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

/* Kuhn triangulation methods */

/* we always store the vertex list for a kuhn triangle in sorted order.
   to determine if two triangles are equal, we simply check the each v
   in the vertex list. */
int kt_equal( kt_t *a, kt_t *b ) {
  int i;

  for ( i=0; i<D_PLUS_ONE; i++ ) {
    if ( a->vertices[i] != b->vertices[i] ) {
      return 0;
    }
  }

  return 1;
}

void kt_swap( kt_t *kt, bc_t *bc, int a, int b ) {
  int tmp_v;
  float tmp_c;

  if ( a == b ) {
    return;
  }

  tmp_v = kt->vertices[a];
  tmp_c = bc->coords[a];

  kt->vertices[a] = kt->vertices[b];
  bc->coords[a] = bc->coords[b];

  kt->vertices[b] = tmp_v;
  bc->coords[b] = tmp_c;
}

/* we have to sort the vertex and the barycentric coordinate arrays
   in parallel */
void kt_sort_vertices( kt_t *t, bc_t *b ) {
  int i, j;
  unsigned int max_index, max_val;

  /* just a dumb selection sort */
  for ( i=0; i<D_PLUS_ONE; i++ ) {

    max_val = t->vertices[i];
    max_index = i;

    for ( j=i+1; j<D_PLUS_ONE; j++ ) {
      if ( t->vertices[j] > max_val ) {
	max_val = t->vertices[j];
	max_index = j;
      }
    }

    /* swap i with max_index */
    kt_swap( t, b, i, max_index );
  }
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

/* The following functions allow us to iterate over a grid.  The most common
   is the basic version, called "iterate_over_grid".  It simply starts in
   the coordinates specified by ll and iterates up to ur.  Note that
   each element of UR must be greater than each element of LL, otherwise this
   won't do what you want it to.  You pass in a structure which keeps track
   of where you are in the iteration.  Typically, that should be initialized
   to whatever is in ll.

   Oh. ll = "lower left", and ur = "upper right".
*/

int iterate_over_grid( grid_coord_t *ll, grid_coord_t *ur, grid_coord_t *t ) {
  int i;

  for ( i=0; i<DIMENSIONS; i++ ) {

    t->coords[i]++;

    if ( t->coords[i] > ur->coords[i] ) {
      t->coords[i] = ll->coords[i];
    } else {
      return 1;
    }
  }

  return 0;
}

/*
 * ----------------------------------------------------------------------------
 */

/*

ALL OF THE FOLLOWING ROUTINES EXPECT GLOBAL STATE / PARTITION INDICES!

*/

/* this takes a grid coordinate and warps / clips each coordinate.
   it's handy if you took a grid coordinate known to be in the space,
   and then (for example) added one to each of the coordinates.  That
   operation may have resulted in some grid coordinates that were 
   out of bounds.  This function gets it back in-bounds. */

void grid_warp( space_t *s, grid_coord_t *g ) {
  int i, tmp_coord;

  for ( i=0; i<DIMENSIONS; i++ ) {

    tmp_coord = g->coords[i];

    /* out of range? that may be ok.  some of our dimensions
       wrap around (such as an angle dimension ), but some of
       them need to be clipped. */

    if ( tmp_coord >= s->dim_desc[i].divs ) {

      if ( s->dim_desc[i].options & DIM_OPT_WARP ) {
	/* warp it */
	while ( tmp_coord >= s->dim_desc[i].divs ) {
	  tmp_coord -= s->dim_desc[i].divs;
	}
      } else {
	/* clip it */
	tmp_coord = s->dim_desc[i].divs - 1;
      }

    } else if ( tmp_coord < 0 ) {

      if ( s->dim_desc[i].options & DIM_OPT_WARP ) {
	/* warp it */
	while ( tmp_coord < 0 ) {
	  tmp_coord += s->dim_desc[i].divs;
	}
      } else {
	/* clip it */
	tmp_coord = 0;
      }

    }

    g->coords[i] = tmp_coord;
  }
}

int grid_to_state( space_t *s, grid_coord_t *g ) {
  int i, state;

  /* this makes sure that the grid coords are in-bounds */
  grid_warp( s, g );

  state = 0;

  for ( i=DIMENSIONS-1; i>=0; i-- ) {
    state *= s->dim_desc[i].divs;
    state += g->coords[i];
  }

  return state;
}

void state_to_grid( space_t *s, int g_state, grid_coord_t *g ) {
  int i, tmp, size;

  size = s->total_space_size;

  for ( i=DIMENSIONS-1; i>0; i-- ) {
    tmp = g_state / size;
    g->coords[i] = tmp;

    g_state -= tmp * size;
    size /= s->dim_desc[i-1].divs;
  }

  g->coords[0] = g_state;
}

int coord_to_state( space_t *s, coord_t *c ) {
  grid_coord_t g;

  coord_to_grid( s, c, &g );

  return grid_to_state( s, &g );
}

void coord_to_grid( space_t *s, coord_t *c, grid_coord_t *g ) {
  int i, d_t;

  for ( i=0; i<DIMENSIONS; i++ ) {
    d_t = s->dim_desc[i].disc_type;
    g->coords[i] = (*disc_funcs[ d_t ])( s, i, c );

    //    g->coords[i] = (int)(( c->coords[i] - s->dim_desc[i].min ) /
    //			 s->dim_desc[i].size );
  }
}

void grid_to_coord( space_t *s, grid_coord_t *g, coord_t *c ) {
  int i, d_t;

  for ( i=0; i<DIMENSIONS; i++ ) {
    d_t = s->dim_desc[i].disc_type;
    c->coords[i] = (*undisc_funcs[ d_t ])( s, i, g );

    //  c->coords[i] = s->dim_desc[i].min + g->coords[i] * s->dim_desc[i].size;
  }
}

int coord_outside_space( space_t *s, coord_t *c ) {
  int i;

  for ( i=0; i<DIMENSIONS; i++ ) {
    if ( c->coords[i] > s->dim_desc[i].max ||
	 c->coords[i] < s->dim_desc[i].min ) {
      return 1;
    }
  }

  return 0;
}

int grid_coord_cmp( grid_coord_t *a, grid_coord_t *b ) {
  return memcmp( a, b, sizeof(grid_coord_t) );
}

int grid_manhattan_dist( space_t *s, grid_coord_t *a, grid_coord_t *b ) {
  int i, result, tmp1, tmp2;

  result = 0;
  for ( i=0; i<DIMENSIONS; i++) {

    // for warp-able dimensions, the fastest way may be through
    // the warp.  Say you have an angular dimension.  What's the
    // distance between 0 and 6.0?  It's not 6.0, it's 0.2832

    if ( s->dim_desc[i].options & DIM_OPT_WARP ) {

      tmp1 = abs( a->coords[i] - b->coords[i] );
      if ( tmp1 == 0 ) { continue; }

      if ( a->coords[i] < b->coords[i] ) {
	tmp2 = abs( (a->coords[i] + s->dim_desc[i].divs) - b->coords[i] );
      } else {
	tmp2 = abs( a->coords[i] - (b->coords[i] + s->dim_desc[i].divs) );
      }

      result += MIN( tmp1, tmp2 );

    } else {
      result += abs( a->coords[i] - b->coords[i] );
    }

  }

  return result;
}

#ifdef USE_FUNKY_CACHE
grid_coord_t funky_save_a, funky_save_b;
int funky_save_result;
#endif

int grid_funky_dist( space_t *s, grid_coord_t *a, grid_coord_t *b ) {
  int i, result, tmp1, tmp2;

#ifdef USE_FUNKY_CACHE
  if ( memcmp(a, &funky_save_a, sizeof(grid_coord_t) ) == 0 &&
       memcmp(b, &funky_save_b, sizeof(grid_coord_t) ) == 0 ) {
    return funky_save_result;
  }
#endif

  result = 0;
  for ( i=0; i<DIMENSIONS; i++) {

    // for warp-able dimensions, the fastest way may be through
    // the warp.  Say you have an angular dimension.  What's the
    // distance between 0 and 6.0?  It's not 6.0, it's 0.2832!

    // In this metric, due to the lower-left bias of the grid,
    // certain coordinates get a bonus.  That's what all the +1
    // business is about.  Note that dist(a,b) != dist(b,a).

    // XXX I'm not sure the distances here are quite right
    // if you end up wrapping around.  They may be off by one.

    if ( s->dim_desc[i].options & DIM_OPT_WARP ) {

      if ( a->coords[i] < b->coords[i] ) {
	tmp1 = abs( b->coords[i] - a->coords[i] ); // distance going "right"
	tmp2 = s->dim_desc[i].divs - tmp1;         // distance going "left"

      } else {
	tmp2 = abs( b->coords[i] - a->coords[i] ); // distance going "left"
	tmp1 = s->dim_desc[i].divs - tmp2;         // distance going "right"
      }

      if ( tmp1 < tmp2 ) {
	result += tmp1;
      } else {
	if ( tmp2 > 0 ) {
	  result += tmp2 - 1;
	}
      }

    } else {

      tmp1 = abs( b->coords[i] - a->coords[i] );

      if ( (tmp1 > 0) && ( b->coords[i] < a->coords[i] ) ) {
	tmp1--;
      }

      result += tmp1;

    }

  }

/*    if ( abs( a->coords[1] - b->coords[1] ) > 2 ) { */

/*    wlog( 1, "funky from %d,%d - %d,%d == %d\n", */
/*  	   a->coords[0], a->coords[1], b->coords[0], b->coords[1], result ); */

/*    } */

#ifdef USE_FUNKY_CACHE
  memcpy( &funky_save_a, a, sizeof(grid_coord_t) );
  memcpy( &funky_save_b, b, sizeof(grid_coord_t) );
  funky_save_result = result;
#endif

  return result;
}


/*
 * ----------------------------------------------------------------------------
 */

void grid_diff( grid_coord_t *ur, grid_coord_t *ll, grid_coord_t *diff ) {
  int i;
  for (i=0; i<DIMENSIONS; i++) {
    diff->coords[i] = ur->coords[i] - ll->coords[i];
  }
}

/*
  The idea of this function is to treat one grid_coord_t (the
  "extents") as describing the extents of a box.  The coordinates are
  the length of each side of the hypercube.  Then, the other
  grid_coord_t (the "pos") represents a position within that
  hypercube.  Assuming that we were doing multi-dimensional array
  indexing, on an array whose sizes in each dimension were the
  "extents" box, what would the index of "pos" be?

  We mostly use this function to figure out where in a partition's
  state array a given grid coordinate is.
*/

//grid state to array index - ANUJ
int gen_grid_to_index( grid_coord_t *extents, grid_coord_t *pos ) {
  int i, index;

  index = 0;
  for ( i=DIMENSIONS-1; i>=0; i-- ) {
    index *= extents->coords[i];
    index += pos->coords[i];
  }

  return index;
}


//Amount to add to ll to find index for gc. - ANUJ
int grid_to_diffs_to_index( grid_coord_t *gc, grid_coord_t *ll,
			    grid_coord_t *extents ) {
  grid_coord_t rel_pos;
  grid_diff( gc, ll, &rel_pos );
  return gen_grid_to_index( extents, &rel_pos );
}

/*
 * ----------------------------------------------------------------------------
 */

float euclidean_dist( coord_t *a, coord_t *b ) {
  int i;
  float result, diff;

  result = 0;
  for ( i=0; i<DIMENSIONS; i++ ) {
    diff = a->coords[i] - b->coords[i];
    result += diff * diff;
  }

  result = sqrt( result );
  return result;
}

void get_short_coord( space_t *s, coord_t *start, coord_t *end,
		      coord_t *result ) {
  int i;
  float d1, d2, d3, tmp;

  memcpy( result, end, sizeof( coord_t ) );

  for ( i=0; i<DIMENSIONS; i++ ) {

    if ( s->dim_desc[i].options & DIM_OPT_WARP ) {

      tmp = end->coords[i];
      d1 = euclidean_dist( start, result );

      result->coords[i] = tmp + s->dim_desc[i].range;
      d2 = euclidean_dist( start, result );

      result->coords[i] = tmp - s->dim_desc[i].range;
      d3 = euclidean_dist( start, result );

      if ( d1 < d2 ) {

	if ( d3 < d1 ) {
	  result->coords[i] = tmp - s->dim_desc[i].range;
	} else {
	  result->coords[i] = tmp;
	}

      } else {

	if ( d3 < d2 ) {
	  result->coords[i] = tmp - s->dim_desc[i].range;
	} else {
	  result->coords[i] = tmp + s->dim_desc[i].range;
	}

      }

    }

  }
}

/* returns 1 if a is "to the left of" b */

int left_or_right( space_t *s, int i, float a, float b ) {
  float d1, d2, d3;

  if ( s->dim_desc[i].options & DIM_OPT_WARP ) {
    d1 = fabs( a - b );
    d2 = fabs( a + s->dim_desc[i].range - b );
    d3 = fabs( a - s->dim_desc[i].range - b );

    // this series of if's implements a 3-way minimum.
    if ( d1 < d2 ) {
      if ( d3 < d1 ) {
	a -= s->dim_desc[i].range;
      } else {
	// in this case, d1 is the smallest.  we don't need to do anything.
      }
    } else {
      if ( d3 < d2 ) {
	a -= s->dim_desc[i].range;
      } else {
	a += s->dim_desc[i].range;
      }
    }
  }

  if ( a < b ) {
    return 1;
  } else {
    return 0;
  }
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

/* compute the grid coordinates of the upper-right corner of the space */
void make_ur( space_t *s, grid_coord_t *g ) {
  int i;

  for (i=0; i<DIMENSIONS; i++) {
    g->coords[i] = s->dim_desc[i].divs-1;
  }
}

/* compute the grid coordinates of the upper-right corner of the
   space, minus one */
void make_ur_minus_one( space_t *s, grid_coord_t *g ) {
  int i;

  for (i=0; i<DIMENSIONS; i++) {
    g->coords[i] = s->dim_desc[i].divs-2;
  }
}

/*
 * ----------------------------------------------------------------------------
 */

/* given the grid coordinates of the lower-left vertex of a cube, what
   are the grid coordinates of the upper-right corner? the only tricky
   part here is the fact that the lower-left vertex might be on a
   boundary, which is why we have to warp. */

void make_cube_ur( space_t *s, grid_coord_t *ll, grid_coord_t *ur ) {
  int i;

  for ( i=0; i<DIMENSIONS; i++ ) {
    ur->coords[i] = ll->coords[i] + 1;
  }

  grid_warp( s, ur );
}

float get_coord_diff( space_t *s, int i, float a, float b ) {
  float d1, d2, d3;

  //  wlog( 1, "a=%.20f, b=%.20f\n", a, b );

  if ( s->dim_desc[i].options & DIM_OPT_WARP ) {
    d1 = fabs( b - a );
    d2 = fabs( (b + s->dim_desc[i].range) - a );
    d3 = fabs( (b - s->dim_desc[i].range) - a );

    //    wlog( 1, "  d1=%.20f, d2=%.20f, d3=%.20f\n", d1, d2, d3 );

    if ( d1 < d2 ) {
      return MIN( d3, d1 );
    } else {
      return MIN( d3, d2 );
    }

  } else {
    return fabs( b - a );
  }

}

/*
 * ----------------------------------------------------------------------------
 */

/* given a point and a cube , translate that point into relative
   coordinates of the point within a cube.  The point is assumed to
   fall within the cube (although nothing will break if it isn't;
   you'll just get numbers that are >1.0 or <0 ).  The cube is defined
   by specifying the lower-left and upper-right coordinates. */

void cube_rel_init( space_t *s, coord_t *c, coord_t *c_ll,
		    coord_t *c_ur, cube_rel_t *h ) {
  int i, j;
  float size;

  for ( i=0; i<DIMENSIONS; i++ ) {
    h->indices[i] = i;

    // we want to figure out what the size of this grid square is,
    // in each dimension.  It's not as easy as subtracting, since
    // we have warpable dimensions.  That's what the get_coord_diff
    // function is for.

    size = get_coord_diff( s, i, c_ur->coords[i], c_ll->coords[i] );

    if ( size < 0 ) {
      // this shouldn't ever happen.
      wlog( 1, "WARGH! negative size!\n" );
      for ( j=0; j<DIMENSIONS; j++ ) {
	wlog( 1, "  ur[%d]=%f, ll[%d]=%f\n",
		 j, c_ur->coords[j], j, c_ll->coords[j] );
      }
    }

    if ( size == 0 ) {
      /* this could also be 1.0. doesn't matter. I think. */
      h->c.coords[i] = 0.0;
    } else {
      h->c.coords[i] = ( c->coords[i] - c_ll->coords[i] ) / size;
    }
  }
}

/* the reason we're doing this is because we're basically sorting
   two arrays in parallel, based on the values of one of them */

void cube_rel_swap( cube_rel_t *h, int a, int b ) {
  int tmp_i;
  float tmp_c;

  if ( a == b ) {
    return;
  }

  tmp_c = h->c.coords[a];
  tmp_i = h->indices[a];

  h->c.coords[a] = h->c.coords[b];
  h->indices[a] = h->indices[b];

  h->c.coords[b] = tmp_c;
  h->indices[b] = tmp_i;
}

void cube_rel_sort( cube_rel_t *h ) {
  int i, j, max_index;
  float max_val;

  /* note that this implements highest-to-lowest sorting order */

  /* just a dumb selection sort */
  for ( i=0; i<DIMENSIONS; i++ ) {

    max_val = h->c.coords[i];
    max_index = i;

    for ( j=i+1; j<DIMENSIONS; j++ ) {
      if ( h->c.coords[j] > max_val ) {
	max_val = h->c.coords[j];
	max_index = j;
      }
    }

    /* swap i with max_index */
    cube_rel_swap( h, i, max_index );
  }
}

void coord_to_bcoords( space_t *s, coord_t *c, bc_t *bary_coords,
		       kt_t *triangle ) {
  int i;
  cube_rel_t h;
  float bary_val;
  grid_coord_t g_ll, g_ur;
  coord_t c_ll, c_ur;

  /* this computes the grid coordinates of the "lower left" vertex,
     because it "rounds off" to the lower-left vertex. */
  coord_to_grid( s, c, &g_ll );

  /* make the upper-right vertex of the cube */
  make_cube_ur( s, &g_ll, &g_ur );

  /* compute the real coordinates of the lower-left and upper-right */
  grid_to_coord( s, &g_ll, &c_ll );
  grid_to_coord( s, &g_ur, &c_ur );

  /* translate coordinates relative to the cube */
  cube_rel_init( s, c, &c_ll, &c_ur, &h );

  /* sort in binary decomposition order */
  cube_rel_sort( &h );

  triangle->vertices[0] = grid_to_state( s, &g_ll );
  bary_val = 1;

  /* I know this looks weird... you'll just have to trust me.
     you need to know a bit about kuhn triangulation to understand
     why this works.  basically, indices[i] tells us which dimension to
     increment in order to compute the next vertex of the enclosing
     kuhn triangle.  it works because we're using grid coordinates. */

  for ( i=0; i<DIMENSIONS; i++ ) {
    g_ll.coords[ h.indices[i] ]++;
    triangle->vertices[ i+1 ] = grid_to_state( s, &g_ll );

    bary_coords->coords[i] = bary_val - h.c.coords[i];
    bary_val = h.c.coords[i];
  }
  bary_coords->coords[i] = h.c.coords[i-1];

  /* make sure that the vertices of the defining triangle are sorted */

  // this is now obsolete, since we don't ever compare triangle vertices.
  // HEY! HEY! HEY!  SOMEDAY, THIS MAY BE NECESSARY AGAIN!
  //  kt_sort_vertices( triangle, bary_coords );
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

/* given the starting coordinates start_c and the action a, where do I
   end up?  what reward do I get? how many timesteps did it take?
   tell me the triangle and the barycentric coordinates (put
   the answer in the structures provided).  
*/

void tick_until_transition( space_t *s,
			    coord_t *start_c,
			    grid_coord_t *real_start_gc,
			    int action,
			    trans_t *t,
			    coord_t *end_c,
			    grid_coord_t *end_gc,
			    int *nt ) {
  tick_interface_t ti;
  int num_ticks, result, mdist;
  float tmp_reward;
  grid_coord_t tmp_gc;


  t->reward = 0;

  ti.s = s;
  ti.start_gc = real_start_gc;
  ti.action = action;

  ti.start_c = start_c;
  ti.end_c = end_c;
  ti.tick_num = 1;
  ti.timestep = base_timestep;
  ti.reward = &( t->reward );

  ti.t = t;

  //
  // FAKE TICK
  //
  // This tick doesn't count as part of our control tracking.
  // We dynamically scale the timestep here.
  // we want there to be about 2-3 timesteps until we enter a new grid cell.
  // since we use asymmetrical discretizations, the timestep may need to be
  // smaller or larger depending on the size of the region.  so, we take one
  // gigantic step, compute the resulting manhattan distance, and then
  // estimate the new timestep with BASE/(mdist*2).


  if ( use_variable_timestep == VT_YES ) {

    result = tick( &ti );

    if ( result == TICK_ONE_TICK ) {
      // if this is a one-tick world, we don't need any fancy timestep
      // processing.  just move on.

      coord_to_bcoords( s, end_c, &( t->bary_coords ), &( t->end_triangle ) );
      coord_to_grid( s, end_c, end_gc );
      t->g_tothe_tau_time = discount_factor; // gamma^1
      *nt = 1;
      return;

    } else if ( result == TICK_SIMPLE ) {
      *nt = 1;
      return;

    } else if ( result == TICK_SPACE_EXIT ) {
      // ummmm... well.... someday, I'll have to revisit this.  we might
      // want to decrease the timestep and try again, since we started off
      // with a large one.
      /*      t->tau_time = -1; */
      /*      *nt = 1; */
      /*      return; */

      ti.timestep /= 100;  // arbitrary... :(

    } else {

      coord_to_grid( s, end_c, &tmp_gc );
      mdist = grid_funky_dist( s, real_start_gc, &tmp_gc );
      if ( mdist > 0 ) {
	ti.timestep = ti.timestep / ((float)((mdist)*2));
      }

    }

  } // if use variable timestep


/*    wlog( 1, "a=[%d,%d], b=[%d,%d]\n", */
/*  	   real_start_gc->coords[0], real_start_gc->coords[1], */
/*  	   tmp_gc.coords[0], tmp_gc.coords[1] ); */
/*    wlog( 1, "  mdist=%d; using timestep of %.12f\n", */
/*  	   mdist, ti.timestep ); */


  memcpy( end_c, start_c, sizeof( coord_t ) );

  ti.start_c = end_c;
  ti.end_c = end_c;
  ti.reward = &tmp_reward;
  t->reward = 0;
  num_ticks = 0;

  do {
    num_ticks++;
    if ( num_ticks == MAX_TICKS ) { break; }
    ti.tick_num = num_ticks;

    result = tick( &ti );

    // "integrate" reward
    t->reward += tmp_reward;

    if ( result == TICK_SPACE_EXIT ) {
      t->g_tothe_tau_time = - pow( discount_factor, num_ticks * ti.timestep );
      *nt = num_ticks;
      return;

    } else if ( result == TICK_SIMPLE ) {
      // let the problem define the ending coordinates for us!
      // t->g_tothe_tau_time = discount_factor;
      *nt = 1;
      return;

    } else if ( result == TICK_ONE_TICK ) {
      // if this is a one-tick world, we don't need any fancy timestep
      // processing.  just move on.
      t->g_tothe_tau_time = discount_factor; // gamma^1
      *nt = 1;
      coord_to_bcoords( s, end_c, &( t->bary_coords ), &( t->end_triangle ) );
      coord_to_grid( s, end_c, end_gc );
      return;
    }

    coord_to_grid( s, end_c, &tmp_gc );

    mdist = grid_funky_dist( s, real_start_gc, &tmp_gc );

  } while ( mdist < 1 );


  //  memcpy( &(t->end_coord), end_c, sizeof( coord_t ) );

  t->g_tothe_tau_time = pow( discount_factor, num_ticks * ti.timestep );
  *nt = num_ticks;
  coord_to_bcoords( s, end_c, &( t->bary_coords ), &( t->end_triangle ) );
  memcpy( end_gc, &tmp_gc, sizeof(grid_coord_t) );

  //  wlog( 1, "%d\n", num_ticks);
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void compute_transitions( space_t *s, char *fn ) {
  int g_start_state, num_ticks;
  int i, progress_amount, progress, action;
  coord_t start_c, end_c;
  grid_coord_t ll, ur, tmpgc, end_gc;
  trans_t t;
  FILE *fp;

  if ( fn == NULL ) {
    fp = stdout;
  } else {
    fp = fopen( fn, "wb" );
    if ( fp == NULL ) {
      wlog( 1, "Error opening file %s!\n", fn );
      exit( 0 );
    }
  }

  fprintf( fp, "%d\n", s->total_states );

  progress_amount = s->total_states / 77;
  /* 80 is the typical number of columns in a terminal */

  if ( verbose ) {
    wlog( 1, "  (progress amount is %d)\n  ", progress_amount );
  }
  
  /* loop over every vertex in our grid, precomputing the transitions */

  progress = 0;

  make_ur( s, &ur );
  memset( &ll, 0, sizeof( grid_coord_t ) );
  memset( &tmpgc, 0, sizeof( grid_coord_t ) );

  /* iterate over states within the partition.
     we do this by iterating over the grid the partition defines
  */
  do {

    progress++;
    if ( verbose && progress_amount > 0 &&
	 ( progress % progress_amount == 0 ) ) {
      wlog( 1, "." );
      wlog_flush();
    }

    g_start_state = grid_to_state( s, &tmpgc );

    grid_to_coord( s, &tmpgc, &start_c );

    fprintf( fp, "%d %d\n", g_start_state, NUM_ACTIONS );

    for (action=0; action<NUM_ACTIONS; action++) {

      memset( &t, 0, sizeof(trans_t) );

      tick_until_transition( s, &start_c, &tmpgc, action,
			     &t, &end_c, &end_gc, &num_ticks );

      /* g_tothe_tau_time will be negative if the transition
	 exited the space (or is otherwise an absorbing state) */
      if ( t.g_tothe_tau_time < 0 ) {
	fprintf( fp, "%.2f 0\n", t.reward );
      } else {
	fprintf( fp, "%.2f %d", t.reward, D_PLUS_ONE );
	for ( i=0; i<D_PLUS_ONE; i++ ) {
	  fprintf( fp, " %d %.6f",
		   t.end_triangle.vertices[i],
		   t.bary_coords.coords[i] * t.g_tothe_tau_time );
	}
	fprintf( fp, "\n" );
      }

    } /* foreach action*/

  } while ( iterate_over_grid( &ll, &ur, &tmpgc ) );

  if ( fn != NULL ) {
    fclose( fp );
  }
}

/*
 * ----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 */

void plot_vf( space_t *s, char *in_fn, char *out_fn ) {
  int g_state, policy;
  coord_t c;
  grid_coord_t gc;
  FILE *infp, *outfp;
  char tbuf[512];
  float val, r, g, b, xc, yc, zc;

  if ( in_fn == NULL ) {
    infp = stdin;
  } else {
    infp = fopen( in_fn, "rb" );
    if ( infp == NULL ) {
      wlog( 1, "Error opening file %s!\n", in_fn );
      exit( 0 );
    }
  }

  if ( out_fn == NULL ) {
    outfp = stdout;
  } else {
    outfp = fopen( out_fn, "wb" );
    if ( outfp == NULL ) {
      wlog( 1, "Error opening file %s!\n", out_fn );
      exit( 0 );
    }
  }

  fprintf( outfp, "CMESH\n%d %d\n",
	   s->dim_desc[0].divs,
	   s->dim_desc[1].divs );

  while ( fgets( tbuf, 512, infp ) ) {

    sscanf( tbuf, "%d %d %f", &g_state, &policy, &val );

    state_to_grid( s, g_state, &gc );
    grid_to_coord( s, &gc, &c );

    /* print a line to the out_fn */

#ifdef SAP_PROB
    c.coords[0] -= ( s->dim_desc[0].range/2);
    while ( c.coords[0] < 0 ) {
      c.coords[0] += s->dim_desc[0].range;  // should be TWOPI
    }
    while ( c.coords[0] > s->dim_desc[0].range ) {
      c.coords[0] -= s->dim_desc[0].range;  // should be TWOPI
    }
#endif

    /* this just makes it into a nice box */
    xc = ( c.coords[0] - s->dim_desc[0].min ) / ( s->dim_desc[0].range );
    yc = ( c.coords[1] - s->dim_desc[1].min ) / ( s->dim_desc[1].range );
    zc = val * 0.5;

    fprintf( outfp, "%.4f %.4f %.4f ", xc, yc, zc );

    if ( policy == -1 ) {
      /* UNVISITED! */
      r = 0; g = 0; b = 0.5; /* dark blue */

    } else if ( policy == 0 ) {
      r = 1; g = 0; b = 0; /* red */

    } else if ( policy == 1 ) {
      r = 0; g = 1; b = 0; /* green */

    } else if ( policy == 2 ) {
      r = 0; g = 0; b = 1; /* blue */

    } else {
      r = 0; g = 1; b = 1; /* yellow */
    }

    fprintf( outfp, "%.1f %.1f %.1f 1\n", r, g, b );

  }

  if ( out_fn != NULL ) {
    fclose( outfp );
  }

  if ( in_fn != NULL ) {
    fclose( infp );
  }
}



void plot_partitioning( space_t *s, char *in_fn, char *out_fn ) {
  int g_state, part;
  coord_t c;
  grid_coord_t gc;
  FILE *infp, *outfp;
  char tbuf[512];
  float r, g, b, xc, yc, zc;

  if ( in_fn == NULL ) {
    infp = stdin;
  } else {
    infp = fopen( in_fn, "rb" );
    if ( infp == NULL ) {
      wlog( 1, "Error opening file %s!\n", in_fn );
      exit( 0 );
    }
  }

  if ( out_fn == NULL ) {
    outfp = stdout;
  } else {
    outfp = fopen( out_fn, "wb" );
    if ( outfp == NULL ) {
      wlog( 1, "Error opening file %s!\n", out_fn );
      exit( 0 );
    }
  }

  fprintf( outfp, "CMESH\n%d %d\n",
	   s->dim_desc[0].divs,
	   s->dim_desc[1].divs );

  g_state = 0;
  while ( fgets( tbuf, 512, infp ) ) {

    sscanf( tbuf, "%d", &part );

    state_to_grid( s, g_state, &gc );
    grid_to_coord( s, &gc, &c );

    /* print a line to the out_fn */

#ifdef SAP_PROB
    c.coords[0] -= ( s->dim_desc[0].range/2);
    while ( c.coords[0] < 0 ) {
      c.coords[0] += s->dim_desc[0].range;  // should be TWOPI
    }
    while ( c.coords[0] > s->dim_desc[0].range ) {
      c.coords[0] -= s->dim_desc[0].range;  // should be TWOPI
    }
#endif

    /* this just makes it into a nice box */
    xc = ( c.coords[0] - s->dim_desc[0].min ) / ( s->dim_desc[0].range );
    yc = ( c.coords[1] - s->dim_desc[1].min ) / ( s->dim_desc[1].range );
    zc = part / ( 900.0 * 2.0 );

    fprintf( outfp, "%.4f %.4f %.4f ", xc, yc, zc );

    r = g = b = 1.0;

    fprintf( outfp, "%.1f %.1f %.1f 1\n", r, g, b );

    g_state++;
  }

  if ( out_fn != NULL ) {
    fclose( outfp );
  }

  if ( in_fn != NULL ) {
    fclose( infp );
  }
}


/*
 * ----------------------------------------------------------------------------
 * The end.
 * ----------------------------------------------------------------------------
 */
