
#ifndef _TYPES_H
#define _TYPES_H

#ifdef MCAR_PROB
#include "mcar_prob.h"
#endif

#ifdef SAP_PROB
#include "sap_prob.h"
#endif

#ifdef DAP_PROB
#include "dap_prob.h"
#endif

#ifdef TAP_PROB
#include "tap_prob.h"
#endif

#ifdef HOTPLATE_PROB
#include "hotplate_prob.h"
#endif



/*
  This is the outdegree of every state/action.  Kuhn triangulation
  requires D+1 vertices for every state, but a problem (like the
  hotplate) may override this value.  Through clever use of the tick
  interface and the tick() function, that means you can actually solve
  different kinds of problems, that don't use Kuhn triangulation.
 */

#ifndef D_PLUS_ONE
#define D_PLUS_ONE (DIMENSIONS+1)
#endif

/* #define SAVE_END_COORD */

/*
 * ----------------------------------------------------------------------------
 * Normal space structures
 * ----------------------------------------------------------------------------
 */

/* describe a dimension */
/* divs is the number of vertices per dimension. period.
   the size of a div can change based on what type of dimension
   it is. */
typedef struct dim_t {
  float min, max, range, halfrange, size, midpoint, unisize, midpt_shift;
  int divs;
  int disc_type;
  unsigned int options;
} dim_t;

/* dimension options */
#define DIM_OPT_WARP 1
#define DIM_OPT_CLIP 2

/* describe a space */
typedef struct space_t {
  float actions[NUM_ACTIONS];
  dim_t dim_desc[DIMENSIONS];
  int total_space_size;
  int total_states;
} space_t;

/* a general coordinate in our space */
/* The #ifndef construct is necessary because of some nasty
   problems with mutual dependencies.  This type is
   redefined in some of the problems.h */
#ifndef _COORD_T
#define _COORD_T
typedef struct coord_t {
  float coords[DIMENSIONS];
} coord_t;
#endif

/* a coordinate on the grid */
typedef struct grid_coord_t {
  int coords[DIMENSIONS];
} grid_coord_t;

/*
 * ----------------------------------------------------------------------------
 * Structures for representing our model
 * ----------------------------------------------------------------------------
 */

/* these structures need to be as small as possible */
#pragma pack(1)

/* barycentric coordinate type */
typedef struct bc_t {
  float coords[D_PLUS_ONE];
} bc_t;

/* kuhn triangle type */
/* if you change this, change the kt_sort_vertices function */
typedef struct kt_t {
  /* these represent global state indices */
  unsigned int vertices[D_PLUS_ONE];
} kt_t;

/* the results of taking an action from a state */
typedef struct trans_t {
  kt_t end_triangle;
  bc_t bary_coords;
  float reward, g_tothe_tau_time;
#ifdef SAVE_END_COORD
  coord_t end_coord;
#endif
} trans_t;

typedef struct state {
  trans_t tps[NUM_ACTIONS]; /* the results of each transition */
  /*  unsigned char max_action; */ /* ie, policy */
  int global_state_index;  /* to map l_state_t to g_state_t */
} state_t;

/* a type to help us compute the barycentric coordinates */
typedef struct cube_rel_t {
  coord_t c;
  int indices[DIMENSIONS];
} cube_rel_t;

#pragma pack()
/* now the structures can be any size */

/*
 * ----------------------------------------------------------------------------
 * Miscellaneous structures
 * ----------------------------------------------------------------------------
 */


/* this is a standardized way to interface with a new problem */

typedef struct tick_interface_t {

  /* we provide these */
  space_t *s;              /* this is the space we're operating in */
  coord_t *start_c;        /* starting coordinates (real coords) */
  grid_coord_t *start_gc;  /* starting coordinates (grid coords) */ 
  int action;              /* please perform this action */
  int tick_num;            /* we're on this tick ( 1-based counting ) */
  float timestep;          /* use this timestep */
			   
  /* you fill in these */
  coord_t *end_c;          /* place ending coordinates here */
  float *reward;           /* place the reward here */

  /* you can frob with this ONLY if you know what you're doing! */
  trans_t *t;
} tick_interface_t;

#endif
