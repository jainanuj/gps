
#include "cont_world.h"

disc_func_t disc_funcs[ NUM_DISC_TYPES ] = {
  disc_std,
  disc_asym_edge,
  disc_asym_mid
};

undisc_func_t undisc_funcs[ NUM_DISC_TYPES ] = {
  undisc_std,
  undisc_asym_edge,
  undisc_asym_mid
};

char *disc_names[ NUM_DISC_TYPES ] = {
  "Standard",
  "Asymmetric edge",
  "Asymmetric middle"
};

/* ----------------------------------------------------------------------- */
/* the standard regular grid discretization */

int disc_std( space_t *s, int i, coord_t *c ) {
  return (int)(( c->coords[i] - s->dim_desc[i].min ) / s->dim_desc[i].size );
}

float undisc_std( space_t *s, int i, grid_coord_t *g ) {
  return s->dim_desc[i].min + g->coords[i] * s->dim_desc[i].size;
}

/* ----------------------------------------------------------------------- */
/* symmetrical about the edges  */

// MIN = func( 0.0 )
// MAX = func( 1.0 )
#define F_EDGE_MIN 0.83333333333
#define F_EDGE_MAX 5.0
#define F_EDGE_RANGE (F_EDGE_MAX - F_EDGE_MIN)
#define F_EDGE_STEEPNESS 1.2

float edge_func( float f ) {
  //  return pow( f, 2.0 );
  return (1.0/(F_EDGE_STEEPNESS-f));
}

float edge_inv_func( float f ) {
  //  return pow( f, 2.0 );
  return (-1.0/f) + F_EDGE_STEEPNESS;
}

int disc_asym_edge( space_t *s, int i, coord_t *c ) {
  float rel, f;
  dim_t *d;

  d = &( s->dim_desc[i] );
  f = c->coords[i];

  /* XXX take out midpt shifting stuff!!!! */
/*    f += d->midpt_shift; */
/*    if ( f > d->max ) { */
/*      f -= d->range; */
/*    } */
/*    if ( f < d->min ) { */
/*      f += d->range; */
/*    } */

  if ( f > d->midpoint ) {
    f = 2*(d->midpoint) - f;
    rel = (f - d->min) / d->halfrange;
    rel = ( edge_func( rel ) - F_EDGE_MIN ) / F_EDGE_RANGE;
    rel = 2.0 - rel;
  } else {
    rel = (f - d->min) / d->halfrange;
    rel = ( edge_func( rel ) - F_EDGE_MIN ) / F_EDGE_RANGE;
  }

  rel /= 2;

  // ok. rel is now a number between 0 and 1.  Partition.

  return (int)( rel / d->unisize );
}

float undisc_asym_edge( space_t *s, int i, grid_coord_t *g ) {
  float rel;
  dim_t *d;

  d = &( s->dim_desc[i] );
  rel = (float)( g->coords[i] ) * d->unisize * 2;

  if ( rel > 1.0 ) {

    rel -= 2.0;
    rel = -rel;
    rel *= F_EDGE_RANGE;
    rel += F_EDGE_MIN;

    rel = edge_inv_func( rel );

    rel *= d->halfrange;
    rel += d->min - ( 2 * d->midpoint );

    rel = -rel;


  } else {

    rel *= F_EDGE_RANGE;
    rel += F_EDGE_MIN;

    rel = edge_inv_func( rel );

    rel *= d->halfrange;
    rel += d->min;
  }

/*    rel -= d->midpt_shift; */

/*    if ( rel > d->max ) { */
/*      rel -= d->range; */
/*    } */
/*    if ( rel < d->min ) { */
/*      rel += d->range; */
/*    } */


  return rel;
}

/* ----------------------------------------------------------------------- */
/* symmetrical about the midpoint */
/* this function is the exact opposite of the previous function. */

float mid_func( float f ) {
  return F_EDGE_MAX - (1.0/(F_EDGE_STEEPNESS - 1.0 + f)) + F_EDGE_MIN;
}

float mid_inv_func( float f ) {
  return ( 1 / ( -f + F_EDGE_MIN + F_EDGE_MAX ) ) - (F_EDGE_STEEPNESS - 1.0);
}

int disc_asym_mid( space_t *s, int i, coord_t *c ) {
  float rel, f;
  dim_t *d;

  d = &( s->dim_desc[i] );
  f = c->coords[i];

/*    f += d->midpt_shift; */
/*    if ( f > d->max ) { */
/*      f -= d->range; */
/*    } */
/*    if ( f < d->min ) { */
/*      f += d->range; */
/*    } */

  if ( f > d->midpoint ) {
    f = 2*(d->midpoint) - f;
    rel = (f - d->min) / d->halfrange;
    rel = ( mid_func( rel ) - F_EDGE_MIN ) / F_EDGE_RANGE;
    rel = 2.0 - rel;
  } else {
    rel = (f - d->min) / d->halfrange;
    rel = ( mid_func( rel ) - F_EDGE_MIN ) / F_EDGE_RANGE;
  }

  rel /= 2;

  // ok. rel is now a number between 0 and 1.  Partition.

  return (int)( rel / d->unisize );
}

float undisc_asym_mid( space_t *s, int i, grid_coord_t *g ) {
  float rel;
  dim_t *d;

  d = &( s->dim_desc[i] );
  rel = (float)( g->coords[i] ) * d->unisize * 2;

  if ( rel > 1.0 ) {

    rel -= 2.0;
    rel = -rel;
    rel *= F_EDGE_RANGE;
    rel += F_EDGE_MIN;

    rel = mid_inv_func( rel );

    rel *= d->halfrange;
    rel += d->min - ( 2 * d->midpoint );

    rel = -rel;


  } else {

    rel *= F_EDGE_RANGE;
    rel += F_EDGE_MIN;

    rel = mid_inv_func( rel );

    rel *= d->halfrange;
    rel += d->min;
  }

/*    rel -= d->midpt_shift; */

/*    if ( rel > d->max ) { */
/*      rel -= d->range; */
/*    } */
/*    if ( rel < d->min ) { */
/*      rel += d->range; */
/*    } */


  return rel;
}
