
#include "flags.h"

int verbose = 0;
int do_sanity_checks = 0;

char *mdp_fn = NULL;
char *state_to_part_fn = NULL;
char *part_to_proc_fn = NULL;
char *save_fn = NULL;

int heat_metric = HEAT_STD;
int run_type = RUN_VI;
int use_voting = VOTE_NO;

int make_movie = MM_NO;
int every_nth_frame = 1;
char *movie_format = "/misc/movies/MCAR-%06d";
char movie_fn[256];

float heat_epsilon = 0.00001;

char *echo_string = NULL;

int num_attractors = 0;

int odcd_cache_size = -1;
char *odcd_cache_fn_format = "/tmp/odcd_cache-%d";

int solver = RICHARDSON;
