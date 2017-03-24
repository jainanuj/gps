
#include "sap_prob.h"

dap_t static_dap; /* yargh! a global var! */

/*
 * ============================================================================
 * Problem-specific code here
 * ============================================================================
 */

void _dap_calc_intermediate( dap_t * dap, double t1, double t2, double tv1,
			     double tv2, double v ) {
  double m0 = dap->mass_0;
  double m1 = dap->mass_1;
  double m2 = dap->mass_2;
  double l0 = dap->len_0;
  double l1 = dap->len_1;
  double l2 = dap->len_2;
  //  double gx = dap->gravity_x;
  double gy = dap->gravity_y;
  double gc = dap->gear_constant;
  double mc = dap->motor_constant;
  double r = dap->resistance;
  double f1 = dap->friction_1;
  double f2 = dap->friction_2;

  double ct = cos(t1 - t2);
  double st = sin(t1 - t2);
  double st1 = sin(t1);
  double st2 = sin(t2);

  double c1 = 0.5 * (m0 * l0 * l0 + (m1 + m2) * l1 * l1);
  double c2 = 0.5 * (m2 * l2 * l2);
  double c3 = m2 * l1 * l2;
  double c4 = gy * (m0 * l0 - (m1 + m2) * l1);
  double c5 = -gy * m2 * l2;
  double c6 = (gc * mc) / r;
  double c7 = c6 * c6 * r;

  double den = 4.0 * c1 * c2 - c3 * c3 * ct * ct;
  double term_1 = (-f1 * tv1 + c6 * v) -
    c7 * tv1 - c3 * tv2 * tv2 * st - c4 * st1;
  double term_2 = (-f2 * (tv2 - tv1) + c3 * tv1 * tv1 * st) - c5 * st2;

  dap->cos_t1t2 = ct;
  dap->sin_t1t2 = st;
  dap->sin_t1 = st1;
  dap->sin_t2 = st2;

  dap->c1 = c1;
  dap->c2 = c2;
  dap->c3 = c3;
  dap->c4 = c4;
  dap->c5 = c5;
  dap->c6 = c6;
  dap->c7 = c7;

  dap->den = den;
  dap->term_1 = term_1;
  dap->term_2 = term_2;
}

double _dap_f1( dap_t *dap ) {
  // Make sure you call the calc_intermediate function before calling
  // this one.
  return (2.0 * dap->c2 * dap->term_1 +
	  -dap->c3 * dap->cos_t1t2 * dap->term_2) / dap->den;
}

double _dap_f2( dap_t *dap ) {
  // Make sure you call the calc_intermediate function before calling
  // this one.
  return (2.0 * dap->c1 * dap->term_2 +
	  -dap->c3 * dap->cos_t1t2 * dap->term_1) / dap->den;
}

void dap_tick( dap_t *dap ) {
  int i;
  int n;
  // Runge-Kutta
  // It's complicated.  Don't ask.

  double x[4];
  double xnew[4];
  double k1[4];
  double k2[4];
  double k3[4];
  double k4[4];

  x[0] = dap->theta_1;
  x[1] = dap->theta_2;
  x[2] = dap->theta_vel_1;
  x[3] = dap->theta_vel_2;

  k1[0] = dap->sample_time * dap->theta_vel_1;
  k1[1] = dap->sample_time * dap->theta_vel_2;

  // Get ready to call f1 and f2
  _dap_calc_intermediate(
			 dap,
			 dap->theta_1,
			 dap->theta_2,
			 dap->theta_vel_1,
			 dap->theta_vel_2,
			 dap->voltage
			 );
  k1[2] = dap->sample_time * _dap_f1( dap );
  k1[3] = dap->sample_time * _dap_f2( dap );

  k2[0] = dap->sample_time * (dap->theta_vel_1 + k1[2] / (double)2);
  k2[1] = dap->sample_time * (dap->theta_vel_2 + k1[3] / (double)2);

  // Get ready for f1 and f2 with different values:
  _dap_calc_intermediate(
			 dap,
			 dap->theta_1 + k1[0] / (double)2,
			 dap->theta_2 + k1[1] / (double)2,
			 dap->theta_vel_1 + k1[2] / (double)2,
			 dap->theta_vel_2 + k1[3] / (double)2,
			 dap->voltage
			 );
  k2[2] = dap->sample_time * _dap_f1( dap );
  k2[3] = dap->sample_time * _dap_f2( dap );

  k3[0] = dap->sample_time * (dap->theta_vel_1 + k2[2] / (double)2);
  k3[1] = dap->sample_time * (dap->theta_vel_2 + k2[3] / (double)2);

  // Get ready again
  _dap_calc_intermediate(
			 dap,
			 dap->theta_1 + k2[2] / (double)2,
			 dap->theta_2 + k2[1] / (double)2,
			 dap->theta_vel_1 + k2[2] / (double)2,
			 dap->theta_vel_2 + k2[3] / (double)2,
			 dap->voltage
			 );
  k3[2] = dap->sample_time * _dap_f1( dap );
  k3[3] = dap->sample_time * _dap_f2( dap );

  k4[0] = dap->sample_time * (dap->theta_vel_1 + k3[2]);
  k4[1] = dap->sample_time * (dap->theta_vel_2 + k3[3]);

  // Get ready again
  _dap_calc_intermediate(
			 dap,
			 dap->theta_1 + k3[0],
			 dap->theta_2 + k3[1],
			 dap->theta_vel_1 + k3[2],
			 dap->theta_vel_2 + k3[3],
			 dap->voltage
			 );
  k4[2] = dap->sample_time * _dap_f1( dap );
  k4[3] = dap->sample_time * _dap_f2( dap );

  // Now we perform the actual Runge Kutta calculation.
  for (i = 0; i<4; ++i) {
    xnew[i] = x[i] +
      (k1[i] + (double)2 * k2[i] + (double)2 * k3[i] + k4[i]) / (double)6;
  }

  // Change the internal dap values to reflect the change.
  dap->theta_1 = xnew[0];
  dap->theta_2 = xnew[1];
  dap->theta_vel_1 = xnew[2];
  dap->theta_vel_2 = xnew[3];

  n = (int)(dap->theta_1 / TWOPI);
  dap->theta_1 -= (double)n * TWOPI;
  n = (int)(dap->theta_2 / TWOPI);
  dap->theta_2 -= (double)n * TWOPI;
}


/*
 * ----------------------------------------------------------------------------
 */

void dap_set_defaults( dap_t *d ) {
  memset( d, 0, sizeof( dap_t ) );

  d->len_0 = 0.28;  /* centimeters */
  d->len_1 = 0.30;
  d->len_2 = 0.20;

  d->mass_0 = 0.40;  /* 0.40 */ /* decagrams? */
  d->mass_1 = 0.50;
  d->mass_2 = 0.01;

  d->motor_constant = 1.0;
  d->gear_constant = 1.5;
  d->resistance = 50; /* ohms? */

  d->sample_time = base_timestep; /* seconds */

  d->gravity_x = 0; /* met / sec**2? */
  d->gravity_y = 9.8;

  d->friction_1 = 0.001;
  d->friction_2 = 0.0005; 
}

void dap_clip_and_warp( dap_t *d ) {
  while ( d->theta_1 < 0 ) {
    d->theta_1 += TWOPI;
  }  

  while ( d->theta_2 < 0 ) {
    d->theta_2 += TWOPI;
  }  

  while ( d->theta_1 >= TWOPI ) {
    d->theta_1 -= TWOPI;
  }

  while ( d->theta_2 >= TWOPI ) {
    d->theta_2 -= TWOPI;
  }

/*    if ( fabs( d->theta_vel_1 ) > MAX_VEL_1 ) { */
/*      fprintf( stderr, "Wargh! Clipped theta_vel_1!\n" ); */
/*    } */
/*    if ( fabs( d->theta_vel_2 ) > MAX_VEL_2 ) { */
/*      fprintf( stderr, "Wargh! Clipped theta_vel_2!\n" ); */
/*    } */

  d->theta_vel_1 = MIN( MAX_VEL_1, d->theta_vel_1 );
  d->theta_vel_2 = MIN( MAX_VEL_2, d->theta_vel_2 );

  d->theta_vel_1 = MAX( MIN_VEL_1, d->theta_vel_1 );
  d->theta_vel_2 = MAX( MIN_VEL_2, d->theta_vel_2 );
}

void dap_to_coord( dap_t *d, coord_t *c ) {
  // SIMPLE HACK
  c->coords[0] = d->theta_1;
  c->coords[1] = d->theta_vel_1;
}

void coord_to_dap( coord_t *c, dap_t *d ) {
  // SIMPLE HACK
  d->theta_1 = c->coords[0];
  d->theta_vel_1 = c->coords[1];
  d->theta_2 = 0;
  d->theta_vel_2 = 0;
}

float dap_calc_reward( dap_t *d ) {
  // SIMPLE HACK
  if (
      (
       ( d->theta_1 >= 0 && d->theta_1 < REWARD_THETA_EP )
       ||
       ( d->theta_1 <= TWOPI && d->theta_1 > TWOPI-REWARD_THETA_EP )
      )

      && (d->theta_vel_1 > -REWARD_VEL_EP) && (d->theta_vel_1 < REWARD_VEL_EP)
      ) {

    //    fprintf( stdout, "  R: %.12f\t%.12f\t**** 1.0 ****\n", d->theta_1, d->theta_vel_1 );

    return 1;
  }

  //  fprintf( stdout, "  R: %.12f\t%.12f\t0\n", d->theta_1, d->theta_vel_1 );

  return 0;
}

/*
 * ----------------------------------------------------------------------------
 */

void dap_randomize( dap_t *d ) {
  /* these two generate between 0 and TWOPI */
  d->theta_1 = TWOPI * (rand()/(RAND_MAX+1.0));
  d->theta_2 = TWOPI * (rand()/(RAND_MAX+1.0));
  /* these two lines generate random numbers between MIN_VEL and MAX_VEL */
  d->theta_vel_1 = MIN_VEL_1 + (MAX_VEL_1-MIN_VEL_1) * (rand()/(RAND_MAX+1.0));
  d->theta_vel_2 = MIN_VEL_2 + (MAX_VEL_2-MIN_VEL_2) * (rand()/(RAND_MAX+1.0));

  d->voltage = 0;

  dap_clip_and_warp( d );
}

void dap_barely_randomize( dap_t *d ) {
  /* these two generate between -0.2 - 0.2 */
  d->theta_1 = 0.4 * (rand()/(RAND_MAX+1.0)) - 0.2;
  d->theta_2 =  0.4 * (rand()/(RAND_MAX+1.0)) - 0.2;
  d->theta_vel_1 = 0;
  d->theta_vel_2 = 0;

  d->voltage = 0;

  dap_clip_and_warp( d );
}

void dap_balanced( dap_t *d ) {
  d->theta_1 = 0;
  d->theta_2 = 0;
  d->theta_vel_1 = 0;
  d->theta_vel_2 = 0;
  d->voltage = 0;
}

void dap_straight_down( dap_t *d ) {
  d->theta_1 = PI;
  d->theta_2 = PI;
  d->theta_vel_1 = 0;
  d->theta_vel_2 = 0;
  d->voltage = 0;
}

/*
 * ============================================================================
 * Problem must implement these methods
 * ============================================================================
 */

void initialize_space_desc( space_t *s ) {

  /* default discretization level */

  if ( dd[0] == 0 ) {
    dd[0] = DEF_THETA_1_DIVS;
  }

  if ( dd[1] == 0 ) {
    dd[1] = DEF_THETA_VEL_1_DIVS;
  }

  // SIMPLE HACK
  /* describe theta_1 */
#ifdef DAP_ASYM
  s->dim_desc[0].disc_type = DISC_T_ASYM_MID;
#else
  s->dim_desc[0].disc_type = DISC_T_STD;
#endif

  s->dim_desc[0].min = 0;
  s->dim_desc[0].max = TWOPI;
  s->dim_desc[0].midpt_shift = 0;
  s->dim_desc[0].divs = dd[0];
  s->dim_desc[0].options = DIM_OPT_WARP;

  /* describe theta_vel_1 */
#ifdef DAP_ASYM
  s->dim_desc[1].disc_type = DISC_T_ASYM_EDGE;
#else
  s->dim_desc[1].disc_type = DISC_T_STD;
#endif
  s->dim_desc[1].min = MIN_VEL_1;
  s->dim_desc[1].max = MAX_VEL_1;
  s->dim_desc[1].midpt_shift = 0;
  s->dim_desc[1].divs = dd[1];
  s->dim_desc[1].options = DIM_OPT_CLIP;

  s->actions[0] = -10.0;
  s->actions[1] =  10.0;

  /* initialize our global variable */
  dap_set_defaults( &static_dap );
}

/* void initialize_partitioning_desc( part_desc_t *pd ) { */
/*   // SIMPLE HACK */
/*   pd[0].divs = PT1_DIVS; */
/*   pd[1].divs = PTV1_DIVS; */
/* } */

/*
 * ----------------------------------------------------------------------------
 */

int tick( tick_interface_t *ti ) {

  if ( ti->tick_num == 1 &&
       ti->start_gc->coords[0] == 0 &&
       ti->start_gc->coords[1] == ((int)(dd[1])/2) ) {
    *(ti->reward) = 1;
    return TICK_SPACE_EXIT;
  }

  *(ti->reward) = 0;

  static_dap.sample_time = ti->timestep;
  coord_to_dap( ti->start_c, &static_dap );
  static_dap.voltage = ti->s->actions[ ti->action ];


  dap_tick( &static_dap );


  dap_clip_and_warp( &static_dap );
  dap_to_coord( &static_dap, ti->end_c );

  return TICK_CONTINUE;


  // THE OLD WAY
  //
  //  *reward = dap_calc_reward( &static_dap );
  //
  // uncomment this if you want it to be able to "exit" the state space.
  // be careful, however, since you may get boundary conditions on the
  // edge of the reward boundary.
  // remember that if there are cycles, the value function will get
  // larger than this number (it can get as high as R/(1-gamma) )

  //  if ( *reward == 1 ) {
  //    return TICK_SPACE_EXIT;
  //  }
  //  return TICK_CONTINUE;
}

/*
 * ============================================================================
 * End problem-specific code
 * ============================================================================
 */
