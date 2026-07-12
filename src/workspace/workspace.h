#pragma once

#include <scenefx/types/wlr_scene.h>

#include "ashwc.h"
#include "config/config.h"
#include "output/output.h"
#include "toplevel/toplevel.h"

#include <wayland-server-protocol.h>
#include <wlr/types/wlr_ext_workspace_v1.h>

struct ashwc_animation;

struct ashwc_workspace {
  struct wl_list link;

  struct ashwc_output *output;
  uint32_t index;
  struct workspace_config *config;
  struct wlr_ext_workspace_handle_v1 *ext_workspace;

  struct wl_list masters;
  struct wl_list slaves;
  struct wl_list floating_toplevels;
  struct ashwc_toplevel *fullscreen_toplevel;
  enum ashwc_layout layout;
};

void workspace_create_for_output(struct ashwc_output *output,
                                 struct workspace_config *config);

void change_workspace(struct ashwc_workspace *workspace, bool keep_focus);

void toplevel_move_to_workspace(struct ashwc_toplevel *toplevel,
                                struct ashwc_workspace *workspace);

struct ashwc_toplevel *
workspace_find_closest_floating_toplevel(struct ashwc_workspace *workspace,
                                         enum ashwc_direction side);
