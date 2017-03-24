#!/usr/bin/perl

#
# Generates a partitioning that uses one state per partition.
# Useful only for theoretical purposes.
#

$stotal = shift;

for ( $i=0; $i<$stotal; $i++ ) {
    print "$i\n";
}

