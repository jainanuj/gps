#!/usr/bin/perl

$sdivs_x = shift;
$sdivs_y = shift;

$stotal = $sdivs_x * $sdivs_y;

$pdivs_x = shift;
$pdivs_y = shift;

$ptotal = $pdivs_x * $pdivs_y;

$s_per_part_x = $sdivs_x / $pdivs_x;
$s_per_part_y = $sdivs_y / $pdivs_y;

$sx = 0;
$sy = 0;

for ( $i=0; $i<$stotal; $i++ ) {

    $px = int( $sx / $s_per_part_x );
    $py = int( $sy / $s_per_part_y );

    if ( $px >= $pdivs_x ) {
	$px = $pdivs_x - 1;
    }
    if ( $py >= $pdivs_y ) {
	$py = $pdivs_y - 1;
    }

    if ( $pnum >= $ptotal ) {
	die "Whoa!\n";
    }

    $pnum = $py * $pdivs_y + $px;

    print "$pnum\n";

    $sx++;
    if ( $sx >= $sdivs_x ) {
	$sx = 0;
	$sy++;
    }
}

