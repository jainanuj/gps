#!/usr/bin/perl

#
# This script will generate a partitioning based on
# regular divisions in the state space.  It assumes
# that states were generated according to a regular
# grid, and that partitions are also generated on
# a regular grid.
#

$dims = shift || die usage();
$sdivs = shift || die usage();
$pdivs = shift || die usage();

$stotal = $sdivs ** $dims;
$ptotal = $pdivs ** $dims;

$s_per_part = $sdivs / $pdivs;

do {

    # generate the partition coordinates

    for ( $i=0; $i<$dims; $i++ ) {
	$pc[$i] = int( $svec[$i] / $s_per_part );
	if ( $pc[$i] >= $pdivs ) {
	    $pc[$i] = $pdivs-1;
	}
    }

    # compute partition number, based on coords

    $pnum = 0;
    for ( $i=$dims-1; $i>=0; $i-- ) {
	$pnum *= $pdivs;
	$pnum += $pc[$i];
    }

    if ( $pnum >= $ptotal || $pnum < 0 ) {
	die "Whoa!\n";
    }

    print "$pnum\n";
} while ( iterate_over_states( \@svec ) );

sub iterate_over_states {
    my $svec_r = shift;

    for ( $i=0; $i<$dims; $i++ ) {
	$$svec_r[$i] = $$svec_r[$i] + 1;
	if ( $$svec_r[$i] == $sdivs ) {
	    $$svec_r[$i] = 0;
	} else {
	    return 1;
	}
    }

    return 0;
}

sub usage {
    print "usage: quadpart.pl dims sdivs pdivs\n";
}
