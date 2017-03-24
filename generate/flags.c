
#include "flags.h"

int verbose = 0;
int use_variable_timestep = VT_YES;

float discount_factor = 0.6;
float base_timestep = 0.001;

int flag_plot_vf = 0;
int flag_plot_part = 0;

char *out_fn = NULL;
char *in_fn = NULL;

int dd[MAX_DD];
