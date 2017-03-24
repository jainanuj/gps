#ifndef HOTPLATE_PROB__H
#define HOTPLATE_PROB__H

#define NUM_ACTIONS 1
#define DIMENSIONS 2
#define D_PLUS_ONE 5 /* This is the number of dependencies for each node */
#define PROB_STR "HOTPLATE"
#define DEFAULT_VALUE .5

#ifndef HOTPLATE_PROB
#define HOTPLATE_PROB
#endif

/* This must come AFTER the above definitions */
#include "cont_world.h"

/* Problem specific stuff */
#define HP_PART_DIVS 128
#define HP_SIZE 768

#define HP_BOUND_WEIGHT 0.125
#define HP_CENTER_WEIGHT 0.50

#define HP_EPSILON 0.001
#define HP_HITEMP 1.0
#define HP_LOTEMP 0.0
#define HP_DEFTEMP DEFAULT_VALUE

#define HP_BOTTOM 0
#define HP_TOP (HP_SIZE-HP_BOTTOM-1)
#define HP_LEFT 0
#define HP_RIGHT (HP_SIZE-HP_LEFT-1)

#define HP_IS_LO(x,y) ((x)==HP_LEFT || (y) == HP_TOP || (x) == HP_RIGHT)
#define HP_IS_HI(x,y) ( \
        (y)==HP_BOTTOM \
        || ((y)==400 && (x)<=330) \
        || ((y)==200 && (x)==500) \
        )
#define HP_IS_FIXED(x,y) (HP_IS_HI(x,y) || HP_IS_LO(x,y))

#endif
