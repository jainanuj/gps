
#ifndef _FLAGS_H
#define _FLAGS_H

#include <stdio.h>

extern int verbose;

#define VT_NO  0
#define VT_YES 1
extern int use_variable_timestep;

extern char *out_fn;
extern char *in_fn;

extern int flag_plot_vf;
extern int flag_plot_part;

extern float discount_factor;
extern float base_timestep;

/* these are "dimension divisions" */
#define MAX_DD 64
extern int dd[MAX_DD];

#endif
