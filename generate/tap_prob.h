
#ifndef _TAP_PROB_H
#define _TAP_PROB_H

/*
 * ----------------------------------------------------------------------------
 * Problem-specific header file stuff here
 * ----------------------------------------------------------------------------
 */

#include <math.h>
#include <stdlib.h>

#define TWOPI (double)6.2831853071795862
#define PI (TWOPI/2)

typedef struct tap_t {
  // currently unused.  we would need to translate these to a torque.
/*   float voltage; */
/*   float motor_constant; */
/*   float gear_constant; */
/*   float resistance; */

  // these are the lengths of the links
  float l0, l1, l2, l3;

  // these are the masses of the links
  float m0, m1, m2, m3;

  // gravity in y.
  float gy;

  // these are the frictions
  float b1, b2, b3;

} tap_t;

// a 3x3 matrix
typedef float m3x3_t[3][3];

#define DEF_THETA_1_DIVS 24
#define DEF_THETA_2_DIVS 24
#define DEF_THETA_3_DIVS 24
#define DEF_THETA_VEL_1_DIVS 15
#define DEF_THETA_VEL_2_DIVS 18
#define DEF_THETA_VEL_3_DIVS 24

/* #define PT1_DIVS  8 */
/* #define PT2_DIVS  8 */
/* #define PT3_DIVS  8 */
/* #define PTV1_DIVS 5 */
/* #define PTV2_DIVS 6 */
/* #define PTV3_DIVS 8 */

#define TAP_ASYM

// 8, 10, 15 seem to be reasonable

#define MIN_VEL_1 -8.0
#define MAX_VEL_1  8.0

#define MIN_VEL_2 -10.0
#define MAX_VEL_2  10.0

//#define MIN_VEL_3 -12.0
//#define MAX_VEL_3  12.0

#define MIN_VEL_3 -10.0
#define MAX_VEL_3  10.0

/*
 * ----------------------------------------------------------------------------
 * A problem must define these values for us
 * ----------------------------------------------------------------------------
 */

#define NUM_ACTIONS 2

#define DIMENSIONS 6

#define PROB_STR "TAP"

#ifndef TAP_PROB
#define TAP_PROB
#endif

#include "cont_world.h"

/*
 * ----------------------------------------------------------------------------
 */

void tap_tick( tap_t *tap, float torque, float *state, float *newstate,
	       float timestep );

void add_array( float *a, float *b, float *r, int n );
void mul_array( float *a, float c, float *r, int n );

void tap_sys_dyn( tap_t *tap, float torque, float *in, float *out );
int gaussian_eliminate( m3x3_t a, float *x );

void tap_clip_and_warp( float *state );

void tap_set_defaults( tap_t *d );

/*
 * ----------------------------------------------------------------------------
 * End problem-specific defines
 * ----------------------------------------------------------------------------
 */

#endif


