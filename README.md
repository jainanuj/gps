			   GPS version 1.0
		     (General Prioritized Solver)

Copyright 2004 David Wingate <wingated@cs.byu.edu>.
This program is free software; you may redistribute it and/or modify it
under the terms of the GNU Public License (see the file COPYING for
details).

INTRODUCTION

GPS is an efficient solution engine for discrete, stationary,
discounted, infinite-horizon, positive bounded MDPs.  It is an
implementation of a prioritized, partitioned solver.  It includes two
different priority metrics, an optional variable reordering
preprocessing stage, the ability to use either value iteration or
policy iteration, and (if using policy iteration) the ability to use
many different linear system solvers (including any supported by the
AZTEC package) as the policy evaluator.

GPS comes with a companion discretization engine which generates
discrete MDPs from continuous state/continuous time minimum-time
optimal control problems (such as the mountain car).  Currently, four
control problems are distributed: mountain car, single-arm pendulum,
double-arm pendulum, and triple-arm pendulum.  The discretizer uses a
hypercube/Kuhn triangle based method.

The distribution includes several scripts designed to translate an MDP
into a format suitable for partitioning by the METIS package.  It can
also generate naive partitions of various geometries.

BUILDING

To build the generators:

  cd generate
  make clean
  make

To build GPS:

  cd solve
  make clean
  make

NOTES

For the generators, different problems are selected at *compile* time,
which is why different executables are generated for each problem.
Currently, the problem desired is selected as a compile define.  Other
options, such as the timestep and gamma, can be set via command-line
parameters.

The gps executable takes a lot of command-line parameters, none
of which are well documented.  However, the "one.pl" script should
help you to quickly select the most commonly used ones, and will
show you other useful options.  "quick.pl" is even more terse.

The number of states, actions, dimensions, and partitions are limited
only by the amount of RAM available.

There is some support for plotting the resulting value function, using
the GEOMVIEW viewer.  To do this, make GPS dump the resulting value
function with the "save_fn" command-line option.  Then, use the
appropriate generator with the "plotvf" option.
