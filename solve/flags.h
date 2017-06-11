
#ifndef _FLAGS_H
#define _FLAGS_H

#include <stdio.h>
#include "small-matvec.h"

extern int verbose;
extern int do_sanity_checks;

extern char *mdp_fn;
extern char *stat_fn;
extern char *state_to_part_fn;
extern char *part_to_proc_fn;
extern char *save_fn;

#define HEAT_STD 0
#define HEAT_ABS 1
extern int heat_metric;

#define RUN_VI      0
#define RUN_PVI     1
#define RUN_PI      2
#define RUN_PPI     3
extern int run_type;

#define VOTE_YES 0
#define VOTE_NO  1
extern int use_voting;

#define MM_NO 0
#define MM_YES 1
extern int make_movie;

extern int every_nth_frame;

extern char *movie_format;
extern char movie_fn[256];

extern float heat_epsilon;

extern char *echo_string;

extern int num_attractors;

extern int odcd_cache_size;
extern char *odcd_cache_fn_format;

extern int solver;

#endif
