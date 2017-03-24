
#ifndef _MCAR_PROB_H
#define _MCAR_PROB_H

/*
 * ----------------------------------------------------------------------------
 * Problem-specific header file stuff here
 * ----------------------------------------------------------------------------
 */

#include <math.h>

/* Definition of some constants and basic functions */
#define A 1.0
#define B 5.0
#define C 0.0
#define MASS 1.0
#define GRAVITY 9.81
#define f1(x) ((x) * ((x) + 1.0))
#define f1_dashed2(x) (2.0 * (x) + 1.0)
#define f2(x) (A * (x) / sqrt(1.0 + B * (x) * (x)))
#define f(x) (((x) < C) ? f1(x):f2(x))
#define f_dashed(x) (((x) < C) ? f1_dashed2(x) : f2_dashed2(x))

#define DEF_POS_DIVS 300
#define DEF_VEL_DIVS 300

/* #define PPOS_DIVS 30 */
/* #define PVEL_DIVS 30 */

#define REWARD_EPSILON 0.1

/*
 * ----------------------------------------------------------------------------
 * A problem must define these values for us
 * ----------------------------------------------------------------------------
 */

#define NUM_ACTIONS 2

#define DIMENSIONS 2

#define PROB_STR "MCAR"

/*
 * ----------------------------------------------------------------------------
 * End problem-specific defines
 * ----------------------------------------------------------------------------
 */

#endif


