#pragma once

#include <stdint.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/box.h>

enum ashwc_border_state {
  ASHWC_BORDER_INVISIBLE,
  ASHWC_BORDER_ACTIVE,
  ASHWC_BORDER_INACTIVE,
};

struct ashwc_animation {
  bool should_animate;
  bool running;
  uint32_t total_frames;
  uint32_t passed_frames;
  struct wlr_box initial;
  struct wlr_box current;
};

double find_animation_curve_at(double t);

struct ashwc_toplevel;

struct wlr_scene_blur *buffer_ensure_blur(struct wlr_scene_buffer *buffer);

void toplevel_draw_borders(struct ashwc_toplevel *toplevel);

void toplevel_draw_shadow(struct ashwc_toplevel *toplevel);

void toplevel_draw_placeholder(struct ashwc_toplevel *toplevel);

double calculate_animation_passed(struct ashwc_animation *animation);

bool toplevel_animation_next_tick(struct ashwc_toplevel *toplevel);

bool toplevel_draw_frame(struct ashwc_toplevel *toplevel);

void toplevel_apply_clip(struct ashwc_toplevel *toplevel);

void toplevel_unclip_size(struct ashwc_toplevel *toplevel);

struct ashwc_workspace;

void workspace_draw_frame(struct ashwc_workspace *workspace);

void toplevel_apply_effects(struct ashwc_toplevel *toplevel);
