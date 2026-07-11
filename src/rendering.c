#include <scenefx/types/fx/clipped_region.h>
#include <scenefx/types/wlr_scene.h>

#include "rendering.h"

#include "ashwc.h"
#include "config.h"
#include "helpers.h"
#include "something.h"
#include "toplevel.h"
#include "workspace.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <wayland-util.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

extern struct ashwc_server server;

struct ashwc_buffer_blur {
  struct wlr_scene_blur *blur;
  struct wl_listener destroy;
};

static void buffer_blur_handle_destroy(struct wl_listener *listener, void *data) {
  struct ashwc_buffer_blur *bb = wl_container_of(listener, bb, destroy);
  wl_list_remove(&bb->destroy.link);
  free(bb);
}

/* creates (once) and returns the blur node backing this buffer, stored on
 * buffer->node.data. safe to call every frame -- reuses the existing node
 * after the first call. destroyed automatically when the buffer node is. */
struct wlr_scene_blur *buffer_ensure_blur(struct wlr_scene_buffer *buffer) {
  struct ashwc_buffer_blur *bb = buffer->node.data;
  if (bb != NULL) {
    return bb->blur;
  }

  bb = calloc(1, sizeof(*bb));
  bb->blur = wlr_scene_blur_create(buffer->node.parent, 1, 1);
  wlr_scene_node_place_below(&bb->blur->node, &buffer->node);
  wlr_scene_node_set_enabled(&bb->blur->node, false);

  bb->destroy.notify = buffer_blur_handle_destroy;
  wl_signal_add(&buffer->node.events.destroy, &bb->destroy);

  buffer->node.data = bb;

  return bb->blur;
}

static struct fx_corner_radii config_corner_radii(uint32_t radius) {
  return corner_radii_new(
      server.config->border_radius_corners.top_left ? radius : 0,
      server.config->border_radius_corners.top_right ? radius : 0,
      server.config->border_radius_corners.bottom_right ? radius : 0,
      server.config->border_radius_corners.bottom_left ? radius : 0);
}

void toplevel_draw_borders(struct ashwc_toplevel *toplevel) {
  if (toplevel->border != NULL && toplevel->fullscreen) {
    wlr_scene_node_set_enabled(&toplevel->border->node, false);
    return;
  }

  uint32_t border_width = server.config->border_width;
  uint32_t border_radius = server.config->border_radius;
  struct fx_corner_radii border_radii = config_corner_radii(border_radius);

  float *border_color = toplevel == server.focused_toplevel
                            ? server.config->active_border_color
                            : server.config->inactive_border_color;

  if (toplevel->border == NULL) {
    toplevel->border =
        wlr_scene_rect_create(toplevel->scene_tree, 0, 0, border_color);
    wlr_scene_node_lower_to_bottom(&toplevel->border->node);
    wlr_scene_node_set_position(&toplevel->border->node, -border_width,
                               -border_width);
    wlr_scene_rect_set_corner_radii(toplevel->border, border_radii);
  }

  wlr_scene_node_set_enabled(&toplevel->border->node, true);

  uint32_t width, height;
  toplevel_get_actual_size(toplevel, &width, &height);

  wlr_scene_rect_set_size(toplevel->border, width + 2 * border_width,
                          height + 2 * border_width);

  struct clipped_region clipped_region = {
      .area = {border_width, border_width, width, height},
      .corners= config_corner_radii(
          max((int32_t)border_radius - (int32_t)border_width, 0)),
  };
  wlr_scene_rect_set_clipped_region(toplevel->border, clipped_region);

  wlr_scene_rect_set_color(toplevel->border, border_color);
}

struct iter_scene_buffer_apply_blur_args {
  int32_t root_x;
  int32_t root_y;
  struct wlr_box geometry;
  uint32_t width;
  uint32_t height;
  double width_scale;
  double height_scale;
  double opacity;
  uint32_t border_radius;
};

void iter_scene_buffer_apply_effects(struct wlr_scene_buffer *buffer, int lx,
                                     int ly, void *data) {
  struct iter_scene_buffer_apply_blur_args *args = data;

  wlr_scene_buffer_set_opacity(buffer, args->opacity);

  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_try_from_buffer(buffer);
  if (scene_surface == NULL)
    return;

  struct wlr_surface *surface = scene_surface->surface;

  uint32_t surface_width = surface->current.width;
  uint32_t surface_height = surface->current.height;

  surface_width *= args->width_scale;
  surface_height *= args->height_scale;

  wlr_scene_buffer_set_dest_size(buffer, surface_width, surface_height);

  /* we dont round or blur popups */
  if (wlr_xdg_popup_try_from_wlr_surface(surface) != NULL)
    return;

  int32_t x = lx - args->root_x;
  int32_t y = ly - args->root_y;

  struct fx_corner_radii corners = corner_radii_none();

  if (server.config->border_radius_corners.top_left && x == 0 && y == 0) {
    corners.top_left = (uint16_t)args->border_radius;
  }

  if (server.config->border_radius_corners.bottom_left && x == 0 &&
      y + surface->current.height == args->geometry.height) {
    corners.bottom_left = (uint16_t)args->border_radius;
  }

  if (server.config->border_radius_corners.top_right &&
      x + surface->current.width == args->geometry.width && y == 0) {
    corners.top_right = (uint16_t)args->border_radius;
  }

  if (server.config->border_radius_corners.bottom_right &&
      x + surface->current.width == args->geometry.width &&
      y + surface->current.height == args->geometry.height) {
    corners.bottom_right = (uint16_t)args->border_radius;
  }

  wlr_scene_buffer_set_corner_radii(buffer, corners);

  /* we dont blur subsurfaces */
  if (wlr_subsurface_try_from_wlr_surface(surface) != NULL)
    return;

  struct wlr_scene_blur *blur = buffer_ensure_blur(buffer);

  if (server.config->blur) {
    wlr_scene_blur_set_size(blur, surface_width, surface_height);
    wlr_scene_node_set_position(&blur->node, buffer->node.x, buffer->node.y);

    wlr_scene_blur_set_corner_radii(
        blur,
        config_corner_radii(args->border_radius));

    struct clipped_region clipped_region = {
        .area = {0, 0, surface_width, surface_height},
        .corners = config_corner_radii(args->border_radius),
    };
    wlr_scene_blur_set_clipped_region(blur, clipped_region);

    wlr_scene_node_set_enabled(&blur->node, true);
  } else {
    wlr_scene_node_set_enabled(&blur->node, false);
  }
}

void toplevel_apply_effects(struct ashwc_toplevel *toplevel) {
  double opacity;
  if (!toplevel->fullscreen || server.config->apply_opacity_when_fullscreen) {
    opacity = toplevel == server.focused_toplevel ? toplevel->active_opacity
                                                  : toplevel->inactive_opacity;
  } else {
    opacity = 1.0;
  }

  uint32_t border_radius =
      toplevel->fullscreen
          ? 0
          : max(server.config->border_radius - server.config->border_width, 0);

  struct wlr_box geometry = toplevel_get_geometry(toplevel);

  uint32_t width, height;
  toplevel_get_actual_size(toplevel, &width, &height);

  struct iter_scene_buffer_apply_blur_args args = {
      .root_x = toplevel->scene_tree->node.x,
      .root_y = toplevel->scene_tree->node.y,
      .geometry = toplevel_get_geometry(toplevel),
      .width = width,
      .height = height,
      .width_scale = (double)width / geometry.width,
      .height_scale = (double)height / geometry.height,
      .opacity = opacity,
      .border_radius = border_radius,
  };

  wlr_scene_node_for_each_buffer(&toplevel->scene_tree->node,
                                 iter_scene_buffer_apply_effects, &args);
}

void toplevel_apply_clip(struct ashwc_toplevel *toplevel) {
  uint32_t width, height;
  toplevel_get_actual_size(toplevel, &width, &height);

  struct wlr_box geometry = toplevel_get_geometry(toplevel);
  struct wlr_box clip_box = (struct wlr_box){
      .x = geometry.x,
      .y = geometry.y,
      .width = width,
      .height = height,
  };

  wlr_scene_subsurface_tree_set_clip(&toplevel->scene_tree->node, &clip_box);

  struct wlr_scene_node *n;
  wl_list_for_each(n, &toplevel->scene_tree->children, link) {
    struct ashwc_something *view = n->data;
    if (view != NULL && view->type == ASHWC_POPUP) {
      wlr_scene_subsurface_tree_set_clip(n, NULL);
    }
  }
}

double find_animation_curve_at(double t) {
  size_t down = 0;
  size_t up = BAKED_POINTS_COUNT - 1;

  size_t middle = (up + down) / 2;
  while (up - down != 1) {
    if (server.config->baked_points[middle].x <= t) {
      down = middle;
    } else {
      up = middle;
    }
    middle = (up + down) / 2;
  }

  return server.config->baked_points[up].y;
}

bool toplevel_animation_next_tick(struct ashwc_toplevel *toplevel) {
  double animation_passed = (double)toplevel->animation.passed_frames /
                            toplevel->animation.total_frames;
  double factor = find_animation_curve_at(animation_passed);

  uint32_t width =
      toplevel->animation.initial.width +
      (toplevel->current.width - toplevel->animation.initial.width) * factor;
  uint32_t height =
      toplevel->animation.initial.height +
      (toplevel->current.height - toplevel->animation.initial.height) * factor;

  uint32_t x = toplevel->animation.initial.x +
               (toplevel->current.x - toplevel->animation.initial.x) * factor;
  uint32_t y = toplevel->animation.initial.y +
               (toplevel->current.y - toplevel->animation.initial.y) * factor;

  wlr_scene_node_set_position(&toplevel->scene_tree->node, x, y);

  toplevel->animation.current = (struct wlr_box){
      .x = x,
      .y = y,
      .width = width,
      .height = height,
  };

  if (animation_passed >= 1.0) {
    toplevel->animation.running = false;
    return false;
  } else {
    toplevel->animation.passed_frames++;
    return true;
  }
}

void toplevel_draw_shadow(struct ashwc_toplevel *toplevel) {
  if (toplevel->shadow != NULL && toplevel->fullscreen) {
    wlr_scene_node_set_enabled(&toplevel->shadow->node, false);
    return;
  }

  uint32_t width, height;
  toplevel_get_actual_size(toplevel, &width, &height);

  uint32_t delta = server.config->shadows_size + server.config->border_width;

  /* we calculate where to clip the shadow */
  struct wlr_box toplevel_box = {
      .x = 0,
      .y = 0,
      .width = width,
      .height = height,
  };

  struct wlr_box shadow_box = {
      .x = server.config->shadows_position.x,
      .y = server.config->shadows_position.y,
      .width = width + 2 * delta,
      .height = height + 2 * delta,
  };

  struct wlr_box intersection_box;
  wlr_box_intersection(&intersection_box, &toplevel_box, &shadow_box);
  /* clipped region takes shadow relative coords, so we translate everything by
   * its position */
  intersection_box.x -= server.config->shadows_position.x;
  intersection_box.y -= server.config->shadows_position.y;

  struct clipped_region clipped_region = {
      .area = intersection_box,
      .corners = config_corner_radii(server.config->border_radius),
  };

  if (toplevel->shadow == NULL) {
    toplevel->shadow = wlr_scene_shadow_create(
        toplevel->scene_tree, shadow_box.width, shadow_box.height,
        server.config->border_radius, server.config->shadows_blur,
        server.config->shadows_color);
    wlr_scene_node_lower_to_bottom(&toplevel->shadow->node);
    wlr_scene_node_set_position(&toplevel->shadow->node, shadow_box.x,
                                shadow_box.y);
  }

  wlr_scene_node_set_enabled(&toplevel->shadow->node, true);

  wlr_scene_shadow_set_size(toplevel->shadow, shadow_box.width,
                            shadow_box.height);
  wlr_scene_shadow_set_clipped_region(toplevel->shadow, clipped_region);
}

bool toplevel_draw_frame(struct ashwc_toplevel *toplevel) {
  bool need_more_frames = false;
  if (toplevel->animation.running) {
    if (toplevel_animation_next_tick(toplevel)) {
      need_more_frames = true;
    }
  } else {
    wlr_scene_node_set_position(&toplevel->scene_tree->node,
                                toplevel->current.x, toplevel->current.y);
  }

  if (server.config->border_width > 0) {
    toplevel_draw_borders(toplevel);
  }
  if (server.config->shadows) {
    toplevel_draw_shadow(toplevel);
  }
  toplevel_apply_clip(toplevel);
  toplevel_apply_effects(toplevel);

  return need_more_frames;
}

void workspace_draw_frame(struct ashwc_workspace *workspace) {
  if (server.grabbed_toplevel != NULL) {
    toplevel_draw_frame(server.grabbed_toplevel);
  }

  bool need_more_frames = false;
  struct ashwc_toplevel *t;
  if (workspace->fullscreen_toplevel != NULL) {
    if (toplevel_draw_frame(workspace->fullscreen_toplevel)) {
      need_more_frames = true;
    }
  } else {
    wl_list_for_each(t, &workspace->floating_toplevels, link) {
      if (toplevel_draw_frame(t)) {
        need_more_frames = true;
      }
    }
    wl_list_for_each(t, &workspace->masters, link) {
      if (toplevel_draw_frame(t)) {
        need_more_frames = true;
      }
    }
    wl_list_for_each(t, &workspace->slaves, link) {
      if (toplevel_draw_frame(t)) {
        need_more_frames = true;
      }
    }
  }

  /* if there are animation that are not finished we request more frames
   * for the output, until all the animations are done */
  if (need_more_frames) {
    wlr_output_schedule_frame(workspace->output->wlr_output);
  }
}
