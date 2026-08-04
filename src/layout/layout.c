#include "layout.h"

#include "ashwc.h"
#include "config/config.h"
#include "toplevel/toplevel.h"
#include "wlr/util/box.h"

#include <stdint.h>
#include <wayland-util.h>
#include <wlr/types/wlr_scene.h>

extern struct ashwc_server server;

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

  default:
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
  if (workspace->layout == layout)
    return;

  if (layout == ASHWC_LAYOUT_CANVAS) {
    struct wl_list snapshot;
    wl_list_init(&snapshot);

    /* move (not copy) every non-fullscreen tiled toplevel's link into a
     * throwaway list first, so masters/slaves stop changing shape while
     * we convert */
    struct ashwc_toplevel *t, *tmp;
    wl_list_for_each_safe(t, tmp, &workspace->masters, link) {
      if (t != workspace->fullscreen_toplevel) {
        wl_list_remove(&t->link);
        wl_list_insert(&snapshot, &t->link);
      }
    }
    wl_list_for_each_safe(t, tmp, &workspace->slaves, link) {
      if (t != workspace->fullscreen_toplevel) {
        wl_list_remove(&t->link);
        wl_list_insert(&snapshot, &t->link);
      }
    }

    /* now convert — masters/slaves are untouched by this point, so any
     * backfill inside toplevel_enter_canvas can't loop back into us */
    wl_list_for_each_safe(t, tmp, &snapshot, link) { toplevel_enter_canvas(t); }
  } else if (workspace->layout == ASHWC_LAYOUT_CANVAS) {
    struct ashwc_toplevel *t, *tmp;
    wl_list_for_each_safe(t, tmp, &workspace->canvas_toplevels, link) {
      toplevel_exit_canvas(t);
    }
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
