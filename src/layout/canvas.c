#include "layout.h"

extern struct ashwc_server server;

void layout_canvas(struct ashwc_workspace *workspace) {}

void layout_canvas_enter(struct ashwc_workspace *workspace) {
  struct wl_list snapshot;
  wl_list_init(&snapshot);

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

  wl_list_for_each_safe(t, tmp, &snapshot, link) { toplevel_enter_canvas(t); }
}

void layout_canvas_exit(struct ashwc_workspace *workspace) {
  struct ashwc_toplevel *t, *tmp;

  wl_list_for_each_safe(t, tmp, &workspace->canvas_toplevels, link) {
    if (t->canvas_restore_tiled) {
      toplevel_exit_canvas(t);
    }
  }
}
