#pragma once

#include <wlr/types/wlr_compositor.h>

enum ashwc_type {
  ASHWC_TOPLEVEL,
  ASHWC_POPUP,
  ASHWC_LAYER_SURFACE,
  ASHWC_LOCK_SURFACE,
};

struct ashwc_toplevel;
struct ashwc_layer_surface;
struct ashwc_lock_surface;

struct ashwc_something {
  enum ashwc_type type;
  union {
    struct ashwc_toplevel *toplevel;
    struct ashwc_popup *popup;
    struct ashwc_layer_surface *layer_surface;
    struct ashwc_lock_surface *lock_surface;
  };
};

struct ashwc_something *root_parent_of_surface(struct wlr_surface *wlr_surface);

struct ashwc_something *something_at(double lx, double ly,
                                     struct wlr_surface **surface, double *sx,
                                     double *sy);
