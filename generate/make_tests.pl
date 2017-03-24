#!/usr/bin/perl

#
# Generate the MCAR and SAP MDPs
#

@divs = ( 100, 150, 200, 250, 300, 350, 400 );

foreach $d ( @divs ) {
    $dsq = $d*$d;

    $cmd = "./g_mcar --dd0=$d --dd1=$d --discount_factor=0.6 --use_variable_timestep --base_timestep=0.001 > /misc/mdp_probs/mcar/mcar-$dsq";

    print STDERR "MCAR - $dsq\n";
    system( $cmd );
}

foreach $d ( @divs ) {
    $dsq = $d*$d;
    $cmd = "./g_sap --dd0=$d --dd1=$d --discount_factor=0.6 --use_variable_timestep --base_timestep=0.001 > /misc/mdp_probs/sap/sap-$dsq";

    print STDERR "SAP - $dsq\n";
    system( $cmd );
}

#
# Generate the DAP MDPs
#

@divs = ( 10, 20, 30, 40, 50, 60, 70 );

foreach $d ( @divs ) {
    $df = $d*$d*$d*$d;

    $cmd = "./g_dap --dd0=$d --dd1=$d --dd2=$d --dd3=$d --discount_factor=0.6 --use_variable_timestep --base_timestep=0.001 > /misc/mdp_probs/dap/dap-$df";

    print STDERR "DAP - $df\n";
    system( $cmd );
}


#
# Generate the TAP MDPs
#

@divs = ( 6, 8, 10, 12, 14, 16, 18 );

foreach $d ( @divs ) {
    $ds = $d*$d*$d*$d*$d*$d;

    $cmd = "./g_tap --dd0=$d --dd1=$d --dd2=$d --dd3=$d --dd4=$d --dd5=$d --discount_factor=0.99 --use_variable_timestep --base_timestep=0.001 > /misc/mdp_probs/tap/tap-$ds";

    print STDERR "TAP - $ds\n";
    system( $cmd );
}
