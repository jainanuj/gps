
#include "tap_prob.h"

tap_t static_tap; /* yargh! a global var! */

/*
 * ============================================================================
 *
 *  Problem-specific code here
 *
 * ============================================================================
 */

/*

  We're using Runge-Kutta.

  Here's the high-level version of what we're doing:
    
  k1 = func( state, action )
  k2 = func( state + (timestep/2)*k1, action )
  k3 = func( state + (timestep/2)*k2, action )
  k4 = func( state + (timestep * k3), action )

  return state + (timestep/6)(k1 + 2*k2 + 2*k3 + k4)
*/

void tap_tick( tap_t *tap, float torque, float *state, float *newstate,
	       float timestep ) {
  float k1[6], k2[6], k3[6], k4[6];
  float tmp[6];

  // euler.
/*   tap_sys_dyn( tap, torque, state, k1 );   */
/*   mul_array( k1, timestep, tmp, 6 ); */
/*   add_array( state, tmp, newstate, 6 ); */

/*   return; */

  // runge-kutta

  // compute k1
  tap_sys_dyn( tap, torque, state, k1 );

  // compute k2
  mul_array( k1, timestep/2, tmp, 6 );
  add_array( state, tmp, tmp, 6 );
  tap_sys_dyn( tap, torque, tmp, k2 );

  // compute k3
  mul_array( k2, timestep/2, tmp, 6 );
  add_array( state, tmp, tmp, 6 );
  tap_sys_dyn( tap, torque, tmp, k3 );

  // compute k4
  mul_array( k3, timestep, tmp, 6 );
  add_array( state, tmp, tmp, 6 );
  tap_sys_dyn( tap, torque, tmp, k4 );

  // compute final answer

  mul_array( k2, 2, k2, 6 );
  mul_array( k3, 2, k3, 6 );

  add_array( k4, k3, tmp, 6 );
  add_array( k2, tmp, tmp, 6 );
  add_array( k1, tmp, tmp, 6 );

  mul_array( tmp, timestep/6, tmp, 6 );

  add_array( state, tmp, newstate, 6 );

  // the final answer is now in newstate
}

void add_array( float *a, float *b, float *r, int n ) {
  int i;
  for ( i=0; i<n; i++ ) {
    r[i] = a[i] + b[i];
  }
}

void mul_array( float *a, float c, float *r, int n ) {
  int i;
  for ( i=0; i<n; i++ ) {
    r[i] = c * a[i];
  }
}

void tap_sys_dyn( tap_t *tap, float torque, float *in, float *out ) {
  float x[3], l;
  m3x3_t a;

  float t1, t2, t3;      /* theta */
  float td1, td2, td3;   /* theta dot */
  float b1, b2, b3;      /* resistances */
  float l0, l1, l2, l3;  /* link lengths */
  float m0, m1, m2, m3;  /* masses */
  float gy;              /* gravity in y */

  float sin_t1, sin_t2, sin_t3;
  float sin_t1_t2, sin_t1_t3, sin_t2_t3;
  float cos_t1_t2, cos_t1_t3, cos_t2_t3;

  // these are the angles (theta)
  t1 = in[0];
  t2 = in[1];
  t3 = in[2];

  // these are the velocities of the angles (theta dot)
  td1 = in[3];
  td2 = in[4];
  td3 = in[5];

  gy = tap->gy;

/*   printf( "TP: gy:%.2f t1: %.2f\t t2: %.2f\t t3: %.2f\t v1: %.2f\t v2: %.2f\t v3: %.2f\n", */
/* 	  gy, in[0], in[1], in[2], in[3], in[4], in[5] ); */

  // tap descriptors
  l0 = tap->l0;
  l1 = tap->l1;
  l2 = tap->l2;
  l3 = tap->l3;

  b1 = tap->b1;
  b2 = tap->b2;
  b3 = tap->b3;

  m0 = tap->m0;
  m1 = tap->m1;
  m2 = tap->m2;
  m3 = tap->m3;

  sin_t1 = sin( t1 );
  sin_t2 = sin( t2 );
  sin_t3 = sin( t3 );

  sin_t1_t2 = sin( t1-t2 );
  sin_t1_t3 = sin( t1-t3 );
  sin_t2_t3 = sin( t2-t3 );

  cos_t1_t2 = cos( t1-t2 );
  cos_t1_t3 = cos( t1-t3 );
  cos_t2_t3 = cos( t2-t3 );


  l = 1.0;

  x[0] =
    -b1*td1
    -2 * (td2*td2) * sin_t1_t2
    -(td3*td3) * sin_t1_t3
    -(3/l)*gy*sin_t1
    -torque;

  x[1] =
    -b2*td2
    +2 * (td1*td1) * sin_t1_t2
    -(td3*td3) * sin_t2_t3
    -(2/l)*gy*sin_t2;

  x[2] =
    -b3*td3
    +(td1*td1)*sin_t1_t3
    +(td2*td2)*sin_t2_t3
    -(1/l)*gy*sin_t3;

  a[0][0] = 3;
  a[0][1] = 2*cos_t1_t2;
  a[0][2] = cos_t1_t3;

  a[1][0] = 2*cos_t1_t2;
  a[1][1] = 2;
  a[1][2] = cos_t2_t3;

  a[2][0] = cos_t1_t3;
  a[2][1] = cos_t2_t3;
  a[2][2] = 1;



/*   x[0] = */
/*     -b1*td1 */
/*     -(m2+m3)*l1*l2* (td2*td2) * sin_t1_t2 */
/*     -m3*l1*l3* (td3*td3) *sin_t1_t3 */
/*     //    +gy*sin_t1*((m1+m2+m3)*l1 - m0*l0 ) */
/*     //    +torque; */
/*     -gy*sin_t1*((m1+m2+m3)*l1 - m0*l0 ) */
/*     -torque; */

/*   x[1] = */
/*     -b2*td2 */
/*     //    -(m2+m3)*l1*l2*(td1*td1)*sin_t1_t2 */
/*     +(m2+m3)*l1*l2*(td1*td1)*sin_t1_t2 */
/*     -m3*l2*l3*(td3*td3)*sin_t2_t3 */
/*     -gy*sin_t2*(m2+m3)*l2 ; */

/*   x[2] = */
/*     -b3*td3 */
/*     +m3*l1*l3*(td1*td1)*sin_t1_t3 */
/*     +m3*l2*l3*(td2*td2)*sin_t2_t3 */
/*     -gy*sin_t3*m3*l3; */


/*   a[0][0] = m0*l0*l0+(m1+m2+m3)*l1*l1; */
/*   a[0][1] = (m1+m3)*l1*l2*cos_t1_t2; */
/*   a[0][2] = m3*l1*l3*cos_t1_t3; */

/*   a[1][0] = (m2+m3)*l1*l2*cos_t1_t2; */
/*   a[1][1] = (m2+m3)*l2*l2; */
/*   a[1][2] = m3*l2*l3*cos_t2_t3; */

/*   a[2][0] = m3*l1*l3*cos_t1_t3; */
/*   a[2][1] = m3*l2*l3*cos_t2_t3; */
/*   a[2][2] = m3*l3*l3; */


  // to find tdd1, tdd2, and tdd3, we need to solve
  // (a^-1)x.  We'll just use dumb gaussian elimination to find it.

  if ( !gaussian_eliminate( a, x ) ) {
    fprintf( stderr, "Whoa! Singular matrix!\n" );
  }

  // change in position is the velocity
  out[0] = in[3];
  out[1] = in[4];
  out[2] = in[5];

  // change in velocity is the acceleration, which
  // are the newly calculated tdd's
  out[3] = x[0];
  out[4] = x[1];
  out[5] = x[2];
}

int gaussian_eliminate( m3x3_t a, float *x ) {
   int i,j,k,maxrow;
   double tmp;

   for ( i=0; i<3; i++ ) {

      /* Find the row with the largest first value */
      maxrow = i;
      for ( j=i+1; j<3; j++ ) {
         if (fabs(a[i][j]) > fabs(a[i][maxrow]))
            maxrow = j;
      }

      /* Swap the maxrow and ith row */
      for ( k=i; k<3+1; k++ ) {
         tmp = a[k][i];
         a[k][i] = a[k][maxrow];
         a[k][maxrow] = tmp;
      }

      /* Singular matrix? */
      if ( fabs(a[i][i]) < 0.00001 ) {
         return 0;
      }

      /* Eliminate the ith element of the jth row */
      for ( j=i+1; j<3; j++ ) {
         for ( k=3; k>=i; k-- ) {
            a[k][j] -= a[k][i] * a[i][j] / a[i][i];
         }
      }
   }

   return 1;
}

/*
 * ---------------------------------------------------------------------------
 */

void tap_clip_and_warp( float *state ) {
  int i;

  // clip the angles
  for ( i=0; i<3; i++ ) {
    while ( state[i] < 0 ) {
      state[i] += TWOPI;
    }  

    while ( state[i] >= TWOPI ) {
      state[i] -= TWOPI;
    }
  }

  // clip the velocities
  if ( state[3] > MAX_VEL_1 ) {
    state[3] = MAX_VEL_1;
  }
  if ( state[3] < MIN_VEL_1 ) {
    state[3] = MIN_VEL_1;
  }

  if ( state[4] > MAX_VEL_2 ) {
    state[4] = MAX_VEL_2;
  }
  if ( state[4] < MIN_VEL_2 ) {
    state[4] = MIN_VEL_2;
  }

  if ( state[5] > MAX_VEL_3 ) {
    state[5] = MAX_VEL_3;
  }
  if ( state[5] < MIN_VEL_3 ) {
    state[5] = MIN_VEL_3;
  }

}

/*
 * ---------------------------------------------------------------------------
 */

void tap_set_defaults( tap_t *t ) {
  memset( t, 0, sizeof( tap_t ) );

  t->l0 = 0.30;  /* centimeters */
  t->l1 = 0.30;
  t->l2 = 0.30;
  t->l3 = 0.30;

  t->m0 = 0.10;  /* 0.40 */ /* decagrams? */
  t->m1 = 0.10;
  t->m2 = 0.10;
  t->m3 = 0.10;

  t->gy = -9.8;

  // friction
  t->b1 = 0.2;
  t->b2 = 0.2;
  t->b3 = 0.2;
}

/*
 * ============================================================================
 *
 *  Problem must implement these methods
 *
 * ============================================================================
 */

void initialize_space_desc( space_t *s ) {

  /* default discretization level */

  if ( dd[0] == 0 ) {
    dd[0] = DEF_THETA_1_DIVS;
  }

  if ( dd[1] == 0 ) {
    dd[1] = DEF_THETA_2_DIVS;
  }

  if ( dd[2] == 0 ) {
    dd[2] = DEF_THETA_3_DIVS;
  }

  if ( dd[3] == 0 ) {
    dd[3] = DEF_THETA_VEL_1_DIVS;
  }

  if ( dd[4] == 0 ) {
    dd[4] = DEF_THETA_VEL_2_DIVS;
  }

  if ( dd[5] == 0 ) {
    dd[5] = DEF_THETA_VEL_3_DIVS;
  }

  /* describe theta_1 */
#ifdef TAP_ASYM
  s->dim_desc[0].disc_type = DISC_T_ASYM_MID;
#else
  s->dim_desc[0].disc_type = DISC_T_STD;
#endif
  s->dim_desc[0].min = 0;
  s->dim_desc[0].max = TWOPI;
  s->dim_desc[0].divs = dd[0];
  s->dim_desc[0].options = DIM_OPT_WARP;

  /* describe theta_2 */
#ifdef TAP_ASYM
  s->dim_desc[1].disc_type = DISC_T_ASYM_MID;
#else
  s->dim_desc[1].disc_type = DISC_T_STD;
#endif
  s->dim_desc[1].min = 0;
  s->dim_desc[1].max = TWOPI;
  s->dim_desc[1].divs = dd[1];
  s->dim_desc[1].options = DIM_OPT_WARP;

  /* describe theta_3 */
#ifdef TAP_ASYM
  s->dim_desc[2].disc_type = DISC_T_ASYM_MID;
#else
  s->dim_desc[2].disc_type = DISC_T_STD;
#endif
  s->dim_desc[2].min = 0;
  s->dim_desc[2].max = TWOPI;
  s->dim_desc[2].divs = dd[2];
  s->dim_desc[2].options = DIM_OPT_WARP;




  /* describe theta_vel_1 */
#ifdef TAP_ASYM
  s->dim_desc[3].disc_type = DISC_T_ASYM_EDGE;
#else
  s->dim_desc[3].disc_type = DISC_T_STD;
#endif
  s->dim_desc[3].min = MIN_VEL_1;
  s->dim_desc[3].max = MAX_VEL_1;
  s->dim_desc[3].divs = dd[3];
  s->dim_desc[3].options = DIM_OPT_CLIP;

  /* describe theta_vel_2 */
#ifdef TAP_ASYM
  s->dim_desc[4].disc_type = DISC_T_ASYM_EDGE;
#else
  s->dim_desc[4].disc_type = DISC_T_STD;
#endif
  s->dim_desc[4].min = MIN_VEL_2;
  s->dim_desc[4].max = MAX_VEL_2;
  s->dim_desc[4].divs = dd[4];
  s->dim_desc[4].options = DIM_OPT_CLIP;

  /* describe theta_vel_3 */
#ifdef TAP_ASYM
  s->dim_desc[5].disc_type = DISC_T_ASYM_EDGE;
#else
  s->dim_desc[5].disc_type = DISC_T_STD;
#endif
  s->dim_desc[5].min = MIN_VEL_3;
  s->dim_desc[5].max = MAX_VEL_3;
  s->dim_desc[5].divs = dd[5];
  s->dim_desc[5].options = DIM_OPT_CLIP;


  s->actions[0] = -10.0;
  s->actions[1] =  10.0;

  /* initialize our global variable */
  tap_set_defaults( &static_tap );

}

/*
 * ---------------------------------------------------------------------------
 */

/* void initialize_partitioning_desc( part_desc_t *pd ) { */
/*   pd[0].divs = PT1_DIVS; */
/*   pd[1].divs = PT2_DIVS; */
/*   pd[2].divs = PT3_DIVS; */
/*   pd[3].divs = PTV1_DIVS; */
/*   pd[4].divs = PTV2_DIVS; */
/*   pd[5].divs = PTV3_DIVS; */
/* } */

/*
 * ---------------------------------------------------------------------------
 */

int tick( tick_interface_t *ti ) {
  float state[6], newstate[6], torque;
  int i;

  if ( ti->tick_num == 1 &&
       ti->start_gc->coords[0] == 0 &&
       ti->start_gc->coords[1] == 0 &&
       ti->start_gc->coords[2] == 0 &&
       ti->start_gc->coords[3] == ((int)(dd[3])/2) &&
       ti->start_gc->coords[4] == ((int)(dd[4])/2) &&
       ti->start_gc->coords[5] == ((int)(dd[5])/2)
       ) {
    *(ti->reward) = 1;
    return TICK_SPACE_EXIT;
  }

  *(ti->reward) = 0;

  for ( i=0; i<6; i++ ) {
    state[i] = ti->start_c->coords[i];
  }

  torque = ti->s->actions[ ti->action ];

  tap_tick( &static_tap, torque, state, newstate, ti->timestep );

  tap_clip_and_warp( newstate );

  for ( i=0; i<6; i++ ) {
    ti->end_c->coords[i] = newstate[i];
  }

  return TICK_CONTINUE;
}

/*
 * ============================================================================
 *
 *  End of code
 *
 * ============================================================================
 */
