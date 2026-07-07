#pragma once

#include <scenefx/types/wlr_scene.h>

#include "ashwc.h"
#include "config.h"
#include "rendering.h"
#include "something.h"

#include <stdint.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xdg_shell.h>

struct ashwc_toplevel {
  struct wl_list link;
  struct wlr_xdg_toplevel *xdg_toplevel;
  struct ashwc_workspace *workspace;

  struct wlr_scene_tree *scene_tree;
  struct wlr_scene_rect *border;
  struct wlr_scene_shadow *shadow;

  struct ashwc_something something;

  bool floating;
  bool fullscreen;
  /* if a floating toplevel becomes fullscreen, we keep its previous state here
   */
  struct wlr_box prev_geometry;

  bool resizing;

  uint32_t configure_serial;
  bool dirty;

  double inactive_opacity;
  double active_opacity;

  struct wlr_box current;
  /* state to be applied to this toplevel; values of 0 mean that the client
   * should choose its size and need to be handled seperately */
  struct wlr_box pending;

  struct ashwc_animation animation;

  struct wlr_foreign_toplevel_handle_v1 *foreign_toplevel_handle;

  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener commit;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;
  struct wl_listener request_fullscreen;
  struct wl_listener set_app_id;
  struct wl_listener set_title;
};

#define X(t) ((t)->scene_tree->node.x)
#define Y(t) ((t)->scene_tree->node.y)

struct ashwc_token {
  struct wlr_xdg_activation_token_v1 *wlr_token;

  struct wl_listener destroy;
};

void toplevel_get_actual_size(struct ashwc_toplevel *toplevel, uint32_t *width,
                              uint32_t *height);

struct wlr_box toplevel_get_geometry(struct ashwc_toplevel *toplevel);

void server_handle_new_toplevel(struct wl_listener *listener, void *data);

void toplevel_handle_commit(struct wl_listener *listener, void *data);

void toplevel_handle_initial_commit(struct ashwc_toplevel *toplevel);

void toplevel_handle_map(struct wl_listener *listener, void *data);

void toplevel_handle_unmap(struct wl_listener *listener, void *data);

void toplevel_handle_destroy(struct wl_listener *listener, void *data);

void toplevel_start_move(struct ashwc_toplevel *toplevel);

void toplevel_start_resize(struct ashwc_toplevel *toplevel, uint32_t edges);

void toplevel_handle_request_move(struct wl_listener *listener, void *data);

void toplevel_handle_request_resize(struct wl_listener *listener, void *data);

void toplevel_handle_request_maximize(struct wl_listener *listener, void *data);

void toplevel_handle_request_fullscreen(struct wl_listener *listener,
                                        void *data);

void toplevel_handle_set_app_id(struct wl_listener *listener, void *data);

void toplevel_handle_set_title(struct wl_listener *listener, void *data);

bool toplevel_position_changed(struct ashwc_toplevel *toplevel);

bool toplevel_matches_window_rule(struct ashwc_toplevel *toplevel,
                                  struct window_rule_regex *condition);

void toplevel_floating_size(struct ashwc_toplevel *toplevel, uint32_t *width,
                            uint32_t *height);

bool toplevel_should_float(struct ashwc_toplevel *toplevel);

void cursor_jump_focused_toplevel(void);

void toplevel_set_initial_state(struct ashwc_toplevel *toplevel, uint32_t x,
                                uint32_t y, uint32_t width, uint32_t height);

void toplevel_set_pending_state(struct ashwc_toplevel *toplevel, uint32_t x,
                                uint32_t y, uint32_t width, uint32_t height);

void toplevel_commit(struct ashwc_toplevel *toplevel);

void toplevel_set_fullscreen(struct ashwc_toplevel *toplevel);

void toplevel_unset_fullscreen(struct ashwc_toplevel *toplevel);

void toplevel_move(void);

void toplevel_resize(void);

void toplevel_tiled_insert_into_layout(struct ashwc_toplevel *toplevel,
                                       uint32_t x, uint32_t y);

void unfocus_focused_toplevel(void);

void focus_toplevel(struct ashwc_toplevel *toplevel);

struct ashwc_toplevel *
toplevel_find_closest_floating_on_workspace(struct ashwc_toplevel *toplevel,
                                            enum ashwc_direction direction);

struct ashwc_output *
toplevel_get_primary_output(struct ashwc_toplevel *toplevel);

uint32_t toplevel_get_closest_corner(struct wlr_cursor *cursor,
                                     struct ashwc_toplevel *toplevel);

struct ashwc_toplevel *get_pointer_focused_toplevel(void);

void toplevel_recheck_opacity_rules(struct ashwc_toplevel *toplevel);

void xdg_activation_handle_new_token(struct wl_listener *listener, void *data);

void xdg_activation_handle_request(struct wl_listener *listener, void *data);

void xdg_activation_destroy(void);
