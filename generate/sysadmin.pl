#!/usr/bin/perl

use strict;

# generates an MDP corresponding to the sysadmin problem
# this will be for the ring topology

# XXX The hardest thing is figuring out what the reward should be!

my $n = shift || die "usage: sysadmin.pl num_computers\n";
my ( $i, @state, @sp, $snum, $spnum, $a, $r, $gamma, $prob );
my ( $ndeps, @probs, @deps );

$gamma = 0.95;

for ( $i=0; $i<$n; $i++ ) {
    $state[$i] = 0;
}

# total number of states
$snum = 1 << $n;
print "$snum\n";

do {
    $snum = get_snum( \@state );
#    print "$snum = ", join(" ", @state), "\n";
    print "$snum ",$n+1,"\n";

    $r = get_reward( \@state );

    # there are n+1 actions: one to reboot each computer, and one
    # to reboot no computers. figure out the reboot transprobs.
    for ( $a=0; $a<$n; $a++ ) {

	for ( $i=0; $i<$n; $i++ ) {
	    $sp[$i] = 0;
	}

	@deps = ();
	@probs = ();

	do {
	    $prob = get_tps( \@state, \@sp, $a );
	    if ( $prob >= 0.00001 ) {
		$spnum = get_snum( \@sp );
		push @deps, $spnum;
		push @probs, sprintf( "%.5f", $prob*$gamma );
	    }
	} while ( iterate_over_state( \@sp ) );

	$ndeps = @deps;
	print "$r $ndeps ";
	for ( $i=0; $i<$ndeps; $i++ ) {
	    print "$deps[$i] $probs[$i] ";
	}
	print "\n";
    }

    #
    # figure out the trans probs of the no-reboot action
    #

    for ( $i=0; $i<$n; $i++ ) {
	$sp[$i] = 0;
    }

    @deps = ();
    @probs = ();

    do {
	$prob = get_tps( \@state, \@sp, -1 );
	if ( $prob >= 0.00001 ) {
	    $spnum = get_snum( \@sp );
	    push @deps, $spnum;
	    push @probs, sprintf( "%.5f", $prob*$gamma );
	}
    } while ( iterate_over_state( \@sp ) );

    $ndeps = @deps;
    print "$r $ndeps ";
    for ( $i=0; $i<$ndeps; $i++ ) {
	print "$deps[$i] $probs[$i] ";
    }
    print "\n";


} while ( iterate_over_state( \@state ) );

#
# ----------------------------------------------------------------------------
#

sub get_tps {
    my ( $s, $sp, $a ) = @_;
    my ( $i, $prob, $par, $tmp );

    $prob = 1.0;

    for ( $i=0; $i<$n; $i++ ) {

	if ( $i == $a ) {
	    # if we reboot machine a, there is a 100% probability
	    # that it will be working next time.  Therefore, there
	    # is a zero percent chance that we will transition to
	    # a state where state[a] == 0
	    if ( $$sp[$i] == 0 ) {
		return 0;
	    }
	    # otherwise, there is a 1.0 probability that we will
	    # transition here.  So: $prob *= 1.0. 

	} else {

	    $par = get_parent( $i );

	    # this is p(X'=t|X,Par,A)
	    if ( $$s[$i] == 0 && $$s[$par] == 0 ) {
		$tmp = 0.0238;
	    } elsif ( $$s[$i] == 1 && $$s[$par] == 0 ) {
		$tmp = 0.475;
	    } elsif ( $$s[$i] == 0 && $$s[$par] == 1 ) {
		$tmp = 0.0475;
	    } elsif ( $$s[$i] == 1 && $$s[$par] == 1 ) {
		$tmp = 0.95;
	    }

	    if ( $$sp[$i] == 1 ) {
		$prob *= $tmp;
	    } else {
		$prob *= (1.0-$tmp);
	    }
	}
    }

    return $prob;
}

sub get_parent {
    my $i = shift;

    if ( $i == 0 ) {
	return $n-1;
    }
    return $i-1;
}

sub get_reward {
    my $state = shift;
    my ($i,$reward);

    $reward = 0;
    for ( $i=0; $i<$n; $i++ ) {
	if ( $$state[$i] == 1 ) {
	    $reward++;
	}
    }

    return $reward;
}

sub get_snum {
    my $state = shift;
    my ($i, $tot, $ind);

    $tot = 0;
    $ind = 1;
    for ( $i=0; $i<$n; $i++ ) {
	$tot += $$state[$i] * $ind;
	$ind *= 2;
    }

    return $tot;
}

sub iterate_over_state {
    my $state = shift;
    my $i;

    for ( $i=0; $i<$n; $i++ ) {

	if ( $$state[$i] == 0 ) {
	    $$state[$i] = 1;
	    return 1;
	}
	$$state[$i] = 0;
    }

    return 0;
}
