#!/usr/bin/perl

$cmd = "./gps "
    . "--verbose=1 "
    . "--epsilon=0.0001 "
    . "--odcd_cache_fn_format=/tmp/odcd_cache-%d "
    . "--save_fn=results "

#    . "--make_movie "
#    . "--every_nth_frame=1 "
#    . "--movie_format=/misc/movies/MCAR-%06d "

#    . "--echo_string=$cs "
#    . "--num_attractors=10 "
    . " ";

print "MDP filename (mdp1): ";
$mdp_fn = <>;
chomp $mdp_fn;
if ( $mdp_fn eq "" ) { $mdp_fn = "mdp1"; };
$cmd .= "--mdp_fn=$mdp_fn ";

print "Partitioning filename (-no-default-): ";
$stp_fn = <>;
chomp $stp_fn;
if ( $stp_fn ne "" ) {
    $cmd .= "--stp_fn=$stp_fn ";
}

print "Run type [part|vi_std|vi_fast|pi|ppi] (pi): ";
$run_type = <>;
chomp $run_type;
if ( $run_type eq "" ) { $run_type = "pi"; };
$cmd .= "--run_type=$run_type ";

print "Heat metric [std|abs] (abs): ";
$heat_metric = <>;
chomp $heat_metric;
if ( $heat_metric eq "" ) { $heat_metric = "abs"; };
$cmd .= "--heat_metric=$heat_metric ";

print "Use voting [yes|no] (yes): ";
$use_voting = <>;
chomp $use_voting;
if ( $use_voting eq "" ) { $use_voting = "yes"; };
if ( $use_voting eq "yes" ) {
    $cmd .= "--use_voting ";
}

print "ODCD cache size [-1|0-numparts] (-1): ";
$cs = <>;
chomp $cs;
if ( $cs eq "" ) { $cs = "-1"; };
$cmd .= "--odcd_cache_size=$cs ";

print "Running: $cmd\n";
system( $cmd );

