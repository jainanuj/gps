
#ifndef _DAP_PROB_H
#define _DAP_PROB_H

/*
 * ----------------------------------------------------------------------------
 * Problem-specific header file stuff here
 * ----------------------------------------------------------------------------
 */

#include <math.h>
#include <stdlib.h>

#define TWOPI (double)6.2831853071795862
#define PI (TWOPI/2)

// SIMPLE HACK
#define DEF_THETA_1_DIVS 400
#define DEF_THETA_VEL_1_DIVS 400

/* #define PT1_DIVS  20 */
/* #define PTV1_DIVS 20 */

//#define DAP_ASYM

/* both of these go from -15.0 to +15.0 */
#define MIN_VEL_1 -15.0
#define MIN_VEL_2 -15.0
#define MAX_VEL_1  15.0
#define MAX_VEL_2  15.0

/* 0.01745 is 1 degree */

// SUPERTINY (for the new reward structure)
// we actually want it to be exactly at 0,0, but it doesn't happen
// due to round-off error.  that's what these are for.
#define REWARD_THETA_EP 0.0001
#define REWARD_VEL_EP   0.001

// EASY (20 degrees)
//#define REWARD_THETA_EP (20*0.01745)
//#define REWARD_VEL_EP   0.1
// HARD (1 degree)
//#define REWARD_THETA_EP (1*0.01745)
//#define REWARD_VEL_EP   0.1

typedef struct dap_t {
    double voltage;
    double motor_constant;
    double gear_constant;
    double resistance;
    double theta_1;
    double theta_2;
    double theta_vel_1;
    double theta_vel_2;
    double len_0;
    double len_1;
    double len_2;
    double mass_0;
    double mass_1;
    double mass_2;
    double sample_time;
    double gravity_x;
    double gravity_y;
    double friction_1;
    double friction_2;

    double c1;
    double c2;
    double c3;
    double c4;
    double c5;
    double c6;
    double c7;
    double den;
    double term_1;
    double term_2;

    double cos_t1t2;
    double sin_t1t2;
    double sin_t1;
    double sin_t2;
} dap_t;

void dap_init( dap_t *dap );

void _dap_calc_intermediate( dap_t *dap, double t1, double t2, double tv1,
			     double tv2, double v );

double _dap_f1( dap_t* dap );
double _dap_f2( dap_t* dap );

void dap_tick( dap_t* dap );

/*
 * ----------------------------------------------------------------------------
 * A problem must define these values for us
 * ----------------------------------------------------------------------------
 */

#define NUM_ACTIONS 2

#define DIMENSIONS 2

#define PROB_STR "SAP"

#ifndef SAP_PROB
#define SAP_PROB
#endif

#include "cont_world.h"

/*
 * ----------------------------------------------------------------------------
 */

/* see comment in cont_world.h */
#ifndef _COORD_T
#define _COORD_T
typedef struct coord_t {
  float coords[DIMENSIONS];
} coord_t;
#endif


void dap_set_defaults( dap_t *d );
void dap_clip_and_warp( dap_t *d );
void dap_to_coord( dap_t *d, coord_t *c );
void coord_to_dap( coord_t *c, dap_t *d );
float dap_calc_reward( dap_t *d );

void dap_randomize( dap_t *d );
void dap_barely_randomize( dap_t *d );
void dap_balanced( dap_t *d );
void dap_straight_down( dap_t *d );

/*
 * ----------------------------------------------------------------------------
 * End problem-specific defines
 * ----------------------------------------------------------------------------
 */

#endif


