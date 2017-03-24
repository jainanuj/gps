#include "hotplate_prob.h"
#include "cont_world.h"


/*
 * ----------------------------------------------------------------------
 *  Must implement these functions
 * ----------------------------------------------------------------------
 */

void initialize_space_desc( space_t *s )
{
    s->dim_desc[0].disc_type = DISC_T_STD;
    s->dim_desc[0].min = 0;
    s->dim_desc[0].max = HP_SIZE; /* Not really max in discrete settings */
    s->dim_desc[0].divs = HP_SIZE;
    s->dim_desc[0].options = DIM_OPT_CLIP;

    s->dim_desc[1].disc_type = DISC_T_STD;
    s->dim_desc[1].min = 0;
    s->dim_desc[1].max = HP_SIZE; /* Not really max in discrete settings */
    s->dim_desc[1].divs = HP_SIZE;
    s->dim_desc[1].options = DIM_OPT_CLIP;

    s->actions[0] = 0;
}

/* void initialize_partitioning_desc( part_desc_t *pd ) */
/* { */
/*     pd[0].divs = HP_PART_DIVS; */
/*     pd[1].divs = HP_PART_DIVS; */
/* } */

int tick( tick_interface_t *ti )
{
    /* We are using TICK_SIMPLE, so we don't fill in things like end_c and
     * reward.  We set that in the transition itself, instead.
     */

    int x, y, i;
    grid_coord_t c;

    x = ti->start_gc->coords[0];
    y = ti->start_gc->coords[1];

    if (HP_IS_FIXED(x,y)) {
        ti->t->g_tothe_tau_time = -1;
        if (HP_IS_HI(x,y)) {
            ti->t->reward = HP_HITEMP;
        }
        else if (HP_IS_LO(x,y)) {
            ti->t->reward = HP_LOTEMP;
        }
        /* Absorbing state, so we don't fill other stuff in */
        return TICK_SIMPLE;
    }

    /* Got this far?  Not absorbing.
     *
     * Cycle through the boundary states, setting the ``barycentric
     * coordinates'' to the appropriate weights (boundary or center) and the
     * ``end triangle'' to the actual state numbers.
     *
     */

    c.coords[0] = x;
    c.coords[1] = y;

    i = 0;
    ti->t->end_triangle.vertices[i] = grid_to_state( ti->s, &c );
    ti->t->bary_coords.coords[i] = HP_CENTER_WEIGHT;

    ++i;
    c.coords[1] = y-1;
    ti->t->end_triangle.vertices[i] = grid_to_state( ti->s, &c );
    ti->t->bary_coords.coords[i] = HP_BOUND_WEIGHT;

    ++i;
    c.coords[1] = y+1;
    ti->t->end_triangle.vertices[i] = grid_to_state( ti->s, &c );
    ti->t->bary_coords.coords[i] = HP_BOUND_WEIGHT;

    c.coords[1] = y;

    ++i;
    c.coords[0] = x-1;
    ti->t->end_triangle.vertices[i] = grid_to_state( ti->s, &c );
    ti->t->bary_coords.coords[i] = HP_BOUND_WEIGHT;

    ++i;
    c.coords[0] = x+1;
    ti->t->end_triangle.vertices[i] = grid_to_state( ti->s, &c );
    ti->t->bary_coords.coords[i] = HP_BOUND_WEIGHT;

    ti->t->g_tothe_tau_time = 1;
    ti->t->reward = 0;

    return TICK_SIMPLE;
}
