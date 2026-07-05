#pragma once

#include "output.h"
#include "ashwc.h"

#include <stdint.h>

void
calculate_masters_dimensions(struct ashwc_output *output, uint32_t master_count,
                             uint32_t slave_count, uint32_t *width, uint32_t *height);

void
calculate_slaves_dimensions(struct ashwc_output *output, uint32_t slave_count,
                            uint32_t *width, uint32_t *height);

bool
toplevel_is_master(struct ashwc_toplevel *toplevel);

bool
toplevel_is_slave(struct ashwc_toplevel *toplevel);

void 
layout_master(struct ashwc_workspace *);

void 
layout_grid(struct ashwc_workspace *);

void 
layout_monocle(struct ashwc_workspace *);

void 
layout_set_pending_state(struct ashwc_workspace *);

void
layout_set_pending_state(struct ashwc_workspace *workspace);

/* this function assumes they are in the same workspace and
 * that t2 comes after t1 if in the same list */
void
layout_swap_tiled_toplevels(struct ashwc_toplevel *t1,
                            struct ashwc_toplevel *t2);

struct ashwc_toplevel *
layout_find_closest_tiled_toplevel(struct ashwc_workspace *workspace, bool master,
                                   enum ashwc_direction side);

struct ashwc_toplevel *
layout_toplevel_at(struct ashwc_workspace *workspace, uint32_t x, uint32_t y);

void
workspace_set_layout(struct ashwc_workspace *workspace,
                     enum ashwc_layout layout);

struct ashwc_toplevel *
layout_monocle_next(struct ashwc_workspace *workspace,
                    struct ashwc_toplevel *current);

struct ashwc_toplevel *
layout_monocle_prev(struct ashwc_workspace *workspace,
                    struct ashwc_toplevel *current);

void
layout_monocle_swap(struct ashwc_toplevel *a,
                    struct ashwc_toplevel *b);

struct ashwc_toplevel *
layout_grid_find_neighbor(struct ashwc_workspace *workspace,
                          struct ashwc_toplevel *current,
                          enum ashwc_direction direction);
