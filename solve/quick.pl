#!/usr/bin/perl

$mdp_fn = shift || "mdp1";
$stp_fn = shift || "part1";
$run_type = shift || "vi";
$heat_metric = shift || "abs";
$solver = shift || "r";
$voting = shift || "1";

$extra = join( " ", @ARGV );

# run_type should be vi, pvi, pi, ppi
# heat_metric should be std, abs

$cmd = "./gps "
    . "--epsilon=0.0001 "
    . "--odcd_cache_fn_format=/tmp/odcd_cache-%d "
    . "--odcd_cache_size=-1 "
    . "--mdp_fn=$mdp_fn "
    . "--stp_fn=$stp_fn "
    . "--run_type=$run_type "
    . "--heat_metric=$heat_metric "
    . "--solver=$solver "
    . "--use_voting=$voting "

    . "--save_fn=results "
    . "--verbose=1 "

#    . "--make_movie "
#    . "--every_nth_frame=1 "
#    . "--movie_format=/misc/movies/MCAR-%06d "

#    . "--echo_string=$cs "
#    . "--num_attractors=10 "
    . " $extra";

# print $cmd, "\n";
system( $cmd );
