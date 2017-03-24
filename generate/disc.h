#ifndef _DISC_H
#define _DISC_H

/* discretization support functions */

int disc_std( space_t *s, int i, coord_t *c );
float undisc_std( space_t *s, int i, grid_coord_t *g );
int disc_asym_mid( space_t *s, int i, coord_t *c );
float undisc_asym_mid( space_t *s, int i, grid_coord_t *g );
int disc_asym_edge( space_t *s, int i, coord_t *c );
float undisc_asym_edge( space_t *s, int i, grid_coord_t *g );

#endif
