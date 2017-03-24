#!/usr/bin/perl

#
# Take a discrete MDP formatted file, and generate
# a file suitable for use by METIS.
#
# Once generated, the program "kmetis" can be run
# to generate the appropriate partitioning.
#

$infn = shift || die usage();
$outfn = shift || die usage();

# this is the type of edge criteria used
$ptype = shift || die usage();

open( MDP, "<$infn" ) || die "couldn't open $infn: $!\n";

$nstates = <MDP>;
chomp $nstates;

for ( $i=0; $i<$nstates; $i++ ) {
    $line = <MDP>;
    chomp $line;

    ($start_state, $nacts) = split( /\s+/, $line );
    $start_state = int( $start_state );

    for ( $a=0; $a<$nacts; $a++ ) {
	$line = <MDP>;

	@parts = split( /\s+/, $line );

	$reward = shift @parts;
	$ndeps = shift @parts;

	for ( $n=0; $n<$ndeps; $n++ ) {
	    $end_state = shift @parts;
	    $prob = shift @parts;

	    $end_state = int( $end_state );

	    # uniform cost
	    if ( $ptype eq "uni" ) {
		$graph{$start_state}{$end_state} = 1;
		$graph{$end_state}{$start_state} = 1;

            # maximum (over actions) edge weight
	    } elsif ( $ptype eq "max" ) {
		$prob = int( $prob * 10000 );
		$tw = $graph{$start_state}{$end_state};
		if ( $prob > 0 && $prob > $tw ) {
		    $graph{$start_state}{$end_state} = $prob;
		    $graph{$end_state}{$start_state} = $prob;
		}

            # average (over actions) edge weight	    
	    } elsif ( $ptype eq "avg" ) {
		$prob = int( $prob * 10000 );
		if ( $prob > 0 ) {
		    $graph{$start_state}{$end_state} += $prob;
		    $graph{$end_state}{$start_state} += $prob;
		    $cnts{$start_state}{$end_state} += 1;
		}

	    }
	}
    }
}

close( MDP );

#
# average out the edge weights
#
if ( $ptype eq "avg" ) {
    foreach $ss (keys(%graph)) {
	foreach $es (keys(%{$graph{$ss}})) {
	    $w = $graph{$ss}{$es};
	    $cnt = $cnts{$ss}{$es};
	    if ( $cnt > 0 ) {
		$np = int( $w / $cnt );
		$graph{$ss}{$es} = $np;
		$graph{$es}{$ss} = $np;
		$cnts{$ss}{$es} = 0;
	    }
	}
    }
}

open( METIS, ">$outfn" ) || die "couldn't open $infn: $!\n";

#
# Count the number of edges
#

$nedges = 0;
foreach $ss (keys(%graph)) {
    foreach $es (keys(%{$graph{$ss}})) {
	if ( $es != $ss ) {
	    $nedges++;
	}
    }
}
$nedges /= 2;

print METIS "$nstates $nedges 1\n";

#for ( $i=0; $i<$nstates; $i++ ) {
#    $ary = $graph{$i};
#
#    if ( !defined($ary) || $#ary == -1 ) {
#	print "NO DATA FOR STATE $i!\n";
#	next;
#    }
#
#
#}

#
# Print out the graph in METIS format
#
$i = 0;
foreach $ss ( sort { int($a) <=> int($b) } keys(%graph) ) {
    while ( $i < $ss ) {
	print STDERR "WHOA: unreferenced vertex $i!  Continuing anyway...\n";
	print METIS "\n";
	$i++;
    }

    foreach $es ( sort { int($a)<=>int($b) } keys(%{$graph{$ss}}) ) {
	if ( $es != $ss ) {
	    $w = $graph{$ss}{$es};
	    $v = $es+1;
	    print METIS "$v $w ";
	}
    }

    print METIS "\n";
    $i++;
}

close( METIS );

sub usage() {
    print "usage: mdp2metis.pl infn outfn ptype\n";
    print "  where ptype=<uni|max|avg> indicates the edge weight type\n";
}
