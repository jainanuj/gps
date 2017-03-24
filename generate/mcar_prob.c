
#define MCAR_PROB

#include "cont_world.h"

/*
 * ============================================================================
 * Problem-specific code here
 * ============================================================================
 */

double f2_dashed2( double x ) {
  double alpha = sqrt(1.0 + B * x * x);
  return( A / (alpha * alpha * alpha) );
}

/* This is the state dynamics.
Inputs :

    * tsk is the task (here HillCar)
    * state is the current (2d here) state
    * action is the control (+4 or -4)

Output :

    * f is the state dynamics vector (2d here)

*/
void hillcar_f( double *state, double action, double *f ) {
  double x, q, p, acc;

  x = state[0];
  q = f_dashed(x);
  p = 1.0 + (q * q);
  acc = (action / (MASS * sqrt(p))) - (GRAVITY * q / p);

  f[0] = state[1];
  f[1] = acc;
}

double hillcar_terminal_reinf( double *state ) {
  double x=state[0];
  double y=state[1];

  /* no reward on left-hand side */
  /* you only get a positive reward for a perfect exit */
  if (x<0) return 0;
  if (y<0) y=-y;
  if ( y - REWARD_EPSILON <= 0 ) {
    return 1.0;
  }
  return 0;

  /* Original reinforcement */
/*   if (y<0) y=-y; */
/*   if (x<0) return -1; */
/*   return 1.0- y/2.0; */
}

/*
 * ============================================================================
 * Problem must implement these methods
 * ============================================================================
 */

void initialize_space_desc( space_t *s ) {

  /* default discretization level */

  if ( dd[0] == 0 ) {
    dd[0] = DEF_POS_DIVS;
  }

  if ( dd[1] == 0 ) {
    dd[1] = DEF_VEL_DIVS;
  }

  /* describe pos */
  s->dim_desc[0].disc_type = DISC_T_STD;
  s->dim_desc[0].min = -1.0;
  s->dim_desc[0].max = 1.0;
  s->dim_desc[0].range = 2.0;
  s->dim_desc[0].size = ( 2.0 / (dd[0]-1) );
  s->dim_desc[0].divs = dd[0];
  s->dim_desc[0].options = DIM_OPT_CLIP;

  /* describe vel */
  s->dim_desc[1].disc_type = DISC_T_STD;
  s->dim_desc[1].min = -4.0;
  s->dim_desc[1].max = 4.0;
  s->dim_desc[1].range = 8.0;
  s->dim_desc[1].size = ( 8.0 / (dd[1]-1) );
  s->dim_desc[1].divs = dd[1];
  s->dim_desc[1].options = DIM_OPT_CLIP;

  s->actions[0] = -4.0;
  s->actions[1] =  4.0;
}

/* void initialize_partitioning_desc( part_desc_t *pd ) { */
/*   pd[0].divs = PPOS_DIVS; */
/*   pd[1].divs = PVEL_DIVS; */
/* } */

/*
 * ----------------------------------------------------------------------------
 */

void clip_coords( space_t *s, coord_t *c ) {

  c->coords[0] = MIN( s->dim_desc[0].max, c->coords[0] );
  c->coords[0] = MAX( s->dim_desc[0].min, c->coords[0] );

  c->coords[1] = MIN( s->dim_desc[1].max, c->coords[1] );
  c->coords[1] = MAX( s->dim_desc[1].min, c->coords[1] );
}

int outside_or_boundary( space_t *s, coord_t *c ) {

  /* if position is on the boundary, it's outside */
  if ( c->coords[0] >= s->dim_desc[0].max ||
       c->coords[0] <= s->dim_desc[0].min ) {
    return 1;
  }

  /* if velocity is on the boundary, it's inside. see the clip
     function for the reason why */
  if ( c->coords[1] > s->dim_desc[1].max ||
       c->coords[1] < s->dim_desc[1].min ) {
    return 1;
  }

  return 0;
}

int tick( tick_interface_t *ti ) {

  double start[2], k1[2], k2[2], k3[2], k4[2], tmp[2];
  double action;
  float timestep;

  timestep = ti->timestep;

  /* run runge-kutta on the state dynamics */

  start[0] = ti->start_c->coords[0];
  start[1] = ti->start_c->coords[1];
  action = ti->s->actions[ ti->action ];

  hillcar_f( start, action, k1 );

  tmp[0] = start[0] + (timestep/2)*k1[0];
  tmp[1] = start[1] + (timestep/2)*k1[1];
  hillcar_f( tmp, action, k2 );

  tmp[0] = start[0] + (timestep/2)*k2[0];
  tmp[1] = start[1] + (timestep/2)*k2[1];
  hillcar_f( tmp, action, k3 );

  tmp[0] = start[0] + (timestep)*k3[0];
  tmp[1] = start[1] + (timestep)*k3[1];
  hillcar_f( tmp, action, k4 );

  tmp[0] = start[0] + (timestep/6)*( k1[0] + 2*k2[0] + 2*k3[0] + k4[0] );
  tmp[1] = start[1] + (timestep/6)*( k1[1] + 2*k2[1] + 2*k3[1] + k4[1] );

  ti->end_c->coords[0] = tmp[0];
  ti->end_c->coords[1] = tmp[1];

  clip_coords( ti->s, ti->end_c );

  tmp[0] = ti->end_c->coords[0];
  tmp[1] = ti->end_c->coords[1];

  if ( outside_or_boundary( ti->s, ti->end_c ) ) {
    *(ti->reward) = hillcar_terminal_reinf( tmp );
    return TICK_SPACE_EXIT;

  } else {
    *(ti->reward) = 0;
    return TICK_CONTINUE;
  }
}

/*
 * ============================================================================
 * End problem-specific code
 * ============================================================================
 */
