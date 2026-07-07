#include "layout.h"

#include "ashwc.h"
#include "config.h"
#include "toplevel.h"
#include "wlr/util/box.h"

#include <stdint.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>

extern struct ashwc_server server;

void calculate_masters_dimensions(struct ashwc_output *output,
                                  uint32_t master_count, uint32_t slave_count,
                                  uint32_t *width, uint32_t *height) {
  uint32_t outer_gaps = server.config->outer_gaps;
  uint32_t inner_gaps = server.config->inner_gaps;
  double master_ratio = server.config->master_ratio;
  double border_width = server.config->border_width;

  struct wlr_box output_box = output->usable_area;

  uint32_t total_width =
      slave_count > 0 ? output_box.width * master_ratio : output_box.width;

  uint32_t total_decorations =
      slave_count > 0 ? outer_gaps                            // left outer gaps
                            + master_count * 2 * border_width // all borders
                            + (master_count - 1) * 2 *
                                  inner_gaps // inner gaps between masters
                            + inner_gaps     // right inner gaps
                      : outer_gaps           // left outer gaps
                            + master_count * 2 * border_width // all borders
                            + (master_count - 1) * 2 *
                                  inner_gaps // inner gaps between masters
                            + outer_gaps;    // right outer gaps

  *width = (total_width - total_decorations) / master_count;
  *height = output_box.height - 2 * outer_gaps - 2 * border_width;
}

void calculate_slaves_dimensions(struct ashwc_output *output,
                                 uint32_t slave_count, uint32_t *width,
                                 uint32_t *height) {
  uint32_t outer_gaps = server.config->outer_gaps;
  uint32_t inner_gaps = server.config->inner_gaps;
  double master_ratio = server.config->master_ratio;
  double border_width = server.config->border_width;

  struct wlr_box output_box = output->usable_area;

  *width = output_box.width * (1 - master_ratio) - outer_gaps - inner_gaps -
           2 * border_width;
  *height =
      (output_box.height - 2 * outer_gaps - (slave_count - 1) * 2 * inner_gaps -
       slave_count * 2 * border_width) /
      slave_count;
}

bool toplevel_is_master(struct ashwc_toplevel *toplevel) {
  struct ashwc_toplevel *t;
  wl_list_for_each(t, &toplevel->workspace->masters, link) {
    if (toplevel == t)
      return true;
  };
  return false;
}

bool toplevel_is_slave(struct ashwc_toplevel *toplevel) {
  struct ashwc_toplevel *t;
  wl_list_for_each(t, &toplevel->workspace->slaves, link) {
    if (toplevel == t)
      return true;
  };
  return false;
}

void layout_master(struct ashwc_workspace *workspace) {
  /* if there are no masters we are done */
  if (wl_list_empty(&workspace->masters))
    return;

  struct ashwc_output *output = workspace->output;

  uint32_t outer_gaps = server.config->outer_gaps;
  uint32_t inner_gaps = server.config->inner_gaps;
  double master_ratio = server.config->master_ratio;
  double border_width = server.config->border_width;

  uint32_t slave_count = wl_list_length(&workspace->slaves);
  uint32_t master_count = wl_list_length(&workspace->masters);

  uint32_t master_width, master_height;
  calculate_masters_dimensions(output, master_count, slave_count, &master_width,
                               &master_height);

  struct ashwc_toplevel *m;
  size_t i = 0;
  wl_list_for_each(m, &workspace->masters, link) {
    uint32_t master_x = output->usable_area.x + outer_gaps +
                        (master_width + 2 * border_width) * i +
                        2 * inner_gaps * i + border_width;
    uint32_t master_y = output->usable_area.y + outer_gaps + border_width;

    toplevel_set_pending_state(m, master_x, master_y, master_width,
                               master_height);
    i++;
  }

  if (slave_count == 0)
    return;

  /* share the remaining space among slaves */
  uint32_t slave_width, slave_height, slave_x, slave_y;
  calculate_slaves_dimensions(workspace->output, slave_count, &slave_width,
                              &slave_height);

  struct ashwc_toplevel *s;
  i = 0;
  wl_list_for_each(s, &workspace->slaves, link) {
    slave_x = output->usable_area.x + output->usable_area.width * master_ratio +
              inner_gaps + border_width;
    slave_y = output->usable_area.y + outer_gaps +
              i * (slave_height + inner_gaps * 2 + 2 * border_width) +
              border_width;

    toplevel_set_pending_state(s, slave_x, slave_y, slave_width, slave_height);
    i++;
  }
}

void layout_grid(struct ashwc_workspace *workspace) {
  struct ashwc_output *output = workspace->output;

  uint32_t count =
      wl_list_length(&workspace->masters) + wl_list_length(&workspace->slaves);

  if (count == 0) {
    return;
  }

  uint32_t outer = server.config->outer_gaps;
  uint32_t inner = server.config->inner_gaps;
  uint32_t border = server.config->border_width;

  uint32_t cols = ceil(sqrt((double)count));
  uint32_t rows = (count + cols - 1) / cols;

  uint32_t cell_width = (output->usable_area.width - 2 * outer -
                         (cols - 1) * 2 * inner - cols * 2 * border) /
                        cols;

  uint32_t cell_height = (output->usable_area.height - 2 * outer -
                          (rows - 1) * 2 * inner - rows * 2 * border) /
                         rows;

  uint32_t i = 0;
  struct ashwc_toplevel *t;

#define PLACE_WINDOW(win)                                                      \
  do {                                                                         \
    uint32_t row = i / cols;                                                   \
    uint32_t col = i % cols;                                                   \
    uint32_t x = output->usable_area.x + outer +                               \
                 col * (cell_width + 2 * border + 2 * inner) + border;         \
    uint32_t y = output->usable_area.y + outer +                               \
                 row * (cell_height + 2 * border + 2 * inner) + border;        \
    toplevel_set_pending_state(win, x, y, cell_width, cell_height);            \
    i++;                                                                       \
  } while (0)

  wl_list_for_each(t, &workspace->masters, link) { PLACE_WINDOW(t); }

  wl_list_for_each(t, &workspace->slaves, link) { PLACE_WINDOW(t); }

#undef PLACE_WINDOW
}
void layout_monocle(struct ashwc_workspace *workspace) {
  struct ashwc_output *output = workspace->output;

  uint32_t outer = server.config->outer_gaps;
  uint32_t border = server.config->border_width;

  int x = output->usable_area.x + outer + border;
  int y = output->usable_area.y + outer + border;

  int width = output->usable_area.width - 2 * outer - 2 * border;

  int height = output->usable_area.height - 2 * outer - 2 * border;

  struct ashwc_toplevel *t;

  wl_list_for_each(t, &workspace->masters, link) {
    toplevel_set_pending_state(t, x, y, width, height);
  }

  wl_list_for_each(t, &workspace->slaves, link) {
    toplevel_set_pending_state(t, x, y, width, height);
  }
}

void layout_set_pending_state(struct ashwc_workspace *workspace) {
  if (workspace->fullscreen_toplevel != NULL)
    return;

  switch (workspace->layout) {
  case ASHWC_LAYOUT_MASTER:
    layout_master(workspace);
    break;

  case ASHWC_LAYOUT_MONOCLE:
    layout_monocle(workspace);
    break;

  case ASHWC_LAYOUT_GRID:
    layout_grid(workspace);
    break;
  }
}

/* this function assumes they are in the same workspace and
 * that t2 comes after t1 if in the same list */
void layout_swap_tiled_toplevels(struct ashwc_toplevel *t1,
                                 struct ashwc_toplevel *t2) {
  struct wl_list *before_t1 = t1->link.prev;
  wl_list_remove(&t1->link);
  wl_list_insert(&t2->link, &t1->link);
  wl_list_remove(&t2->link);
  wl_list_insert(before_t1, &t2->link);

  layout_set_pending_state(t1->workspace);
}

struct ashwc_toplevel *
layout_find_closest_tiled_toplevel(struct ashwc_workspace *workspace,
                                   bool master, enum ashwc_direction side) {
  /* this means there are no tiled toplevels */
  if (wl_list_empty(&workspace->masters))
    return NULL;

  struct ashwc_toplevel *first_master =
      wl_container_of(workspace->masters.next, first_master, link);
  struct ashwc_toplevel *last_master =
      wl_container_of(workspace->masters.prev, last_master, link);

  struct ashwc_toplevel *first_slave = NULL;
  struct ashwc_toplevel *last_slave = NULL;
  if (!wl_list_empty(&workspace->slaves)) {
    first_slave = wl_container_of(workspace->slaves.next, first_slave, link);
    last_slave = wl_container_of(workspace->slaves.prev, last_slave, link);
  }

  switch (side) {
  case ASHWC_UP: {
    if (master || first_slave == NULL)
      return first_master;
    return first_slave;
  }
  case ASHWC_DOWN: {
    if (master || last_slave == NULL)
      return first_master;
    return last_slave;
  }
  case ASHWC_LEFT: {
    return first_master;
  }
  case ASHWC_RIGHT: {
    if (last_slave != NULL)
      return last_slave;
    return last_master;
  }
  }
}

struct ashwc_toplevel *layout_toplevel_at(struct ashwc_workspace *workspace,
                                          uint32_t x, uint32_t y) {
  struct ashwc_toplevel *t;
  wl_list_for_each(t, &workspace->masters, link) {
    uint32_t decorations_left =
        t->link.prev == &workspace->masters
            ? server.config->outer_gaps + server.config->border_width
            : server.config->inner_gaps + server.config->border_width;

    uint32_t decorations_right =
        wl_list_empty(&workspace->slaves)
            ? server.config->outer_gaps + server.config->border_width
            : server.config->inner_gaps + server.config->border_width;

    uint32_t decorations_top =
        server.config->outer_gaps + server.config->border_width;

    uint32_t decorations_bottom =
        server.config->outer_gaps + server.config->border_width;

    struct wlr_box box = {
        .x = t->current.x - decorations_left,
        .y = t->current.y - decorations_top,
        .width = t->current.width + decorations_left + decorations_right + 1,
        .height = t->current.height + decorations_top + decorations_bottom + 1,
    };

    if (wlr_box_contains_point(&box, x, y)) {
      return t;
    }
  }

  wl_list_for_each(t, &workspace->slaves, link) {
    uint32_t decorations_left =
        server.config->inner_gaps + server.config->border_width;
    uint32_t decorations_right =
        server.config->outer_gaps + server.config->border_width;

    uint32_t decorations_top =
        t->link.prev == &workspace->slaves
            ? server.config->outer_gaps + server.config->border_width
            : server.config->inner_gaps + server.config->border_width;

    uint32_t decorations_bottom =
        t->link.next == &workspace->slaves
            ? server.config->outer_gaps + server.config->border_width
            : server.config->inner_gaps + server.config->border_width;

    struct wlr_box box = {
        .x = t->current.x - decorations_left,
        .y = t->current.y - decorations_top,
        .width = t->current.width + decorations_left + decorations_right + 1,
        .height = t->current.height + decorations_top + decorations_bottom + 1,
    };

    if (wlr_box_contains_point(&box, x, y)) {
      return t;
    }
  }

  return NULL;
}

void workspace_set_layout(struct ashwc_workspace *workspace,
                          enum ashwc_layout layout) {
  if (workspace == NULL) {
    return;
  }

  if (workspace->layout == layout) {
    return;
  }

  workspace->layout = layout;

  /* Re-arrange tiled windows immediately. */
  layout_set_pending_state(workspace);

  /* Keep keyboard focus on top. */
  if (server.focused_toplevel != NULL &&
      server.focused_toplevel->workspace == workspace) {
    wlr_scene_node_raise_to_top(&server.focused_toplevel->scene_tree->node);
  }
}

struct ashwc_toplevel *layout_monocle_next(struct ashwc_workspace *workspace,
                                           struct ashwc_toplevel *current) {
  struct wl_list *next = current->link.next;

  if (toplevel_is_master(current)) {
    if (next == &workspace->masters) {
      next = workspace->slaves.next;
    }
  } else {
    if (next == &workspace->slaves) {
      next = workspace->masters.next;
    }
  }

  if (next == &workspace->masters || next == &workspace->slaves) {
    return current;
  }

  return wl_container_of(next, current, link);
}

struct ashwc_toplevel *layout_monocle_prev(struct ashwc_workspace *workspace,
                                           struct ashwc_toplevel *current) {
  struct wl_list *prev = current->link.prev;

  if (toplevel_is_master(current)) {
    if (prev == &workspace->masters) {
      prev = workspace->slaves.prev;
    }
  } else {
    if (prev == &workspace->slaves) {
      prev = workspace->masters.prev;
    }
  }

  if (prev == &workspace->masters || prev == &workspace->slaves) {
    return current;
  }

  return wl_container_of(prev, current, link);
}

void layout_monocle_swap(struct ashwc_toplevel *a, struct ashwc_toplevel *b) {
  if (a == NULL || b == NULL || a == b) {
    return;
  }

  layout_swap_tiled_toplevels(a, b);
}

struct ashwc_toplevel *
layout_grid_find_neighbor(struct ashwc_workspace *workspace,
                          struct ashwc_toplevel *current,
                          enum ashwc_direction direction) {
  size_t count =
      wl_list_length(&workspace->masters) + wl_list_length(&workspace->slaves);

  if (count <= 1)
    return current;

  size_t cols = ceil(sqrt((double)count));
  size_t rows = (count + cols - 1) / cols;

  struct ashwc_toplevel *windows[count];

  size_t i = 0;

  struct ashwc_toplevel *t;

  wl_list_for_each(t, &workspace->masters, link) windows[i++] = t;

  wl_list_for_each(t, &workspace->slaves, link) windows[i++] = t;

  size_t index = 0;

  for (i = 0; i < count; i++) {
    if (windows[i] == current) {
      index = i;
      break;
    }
  }

  int row = index / cols;
  int col = index % cols;

  switch (direction) {
  case ASHWC_LEFT:
    col--;
    break;

  case ASHWC_RIGHT:
    col++;
    break;

  case ASHWC_UP:
    row--;
    break;

  case ASHWC_DOWN:
    row++;
    break;
  }

  if (row < 0 || row >= (int)rows)
    return current;

  if (col < 0 || col >= (int)cols)
    return current;

  size_t target = row * cols + col;

  if (target >= count)
    return current;

  return windows[target];
}
