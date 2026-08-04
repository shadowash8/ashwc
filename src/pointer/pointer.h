#pragma once

#include <libinput.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_constraints_v1.h>
#include <wlr/types/wlr_relative_pointer_v1.h>

struct ashwc_pointer {
  struct wlr_pointer *wlr_pointer;

  char *name;

  struct wl_listener destroy;

  struct wl_listener swipe_begin;
  struct wl_listener swipe_update;
  struct wl_listener swipe_end;

  struct wl_list link;

  uint32_t swipe_fingers;
  double swipe_dx;
  double swipe_dy;
};

struct ashwc_pointer_constraint {
  struct wlr_pointer_constraint_v1 *wlr_pointer_constraint;

  struct wl_listener destroy;
};

enum ashwc_cursor_mode {
  ASHWC_CURSOR_PASSTHROUGH,
  ASHWC_CURSOR_MOVE,
  ASHWC_CURSOR_RESIZE,
  ASHWC_CURSOR_PAN,
};

void server_handle_new_pointer(struct wlr_input_device *device);

void server_handle_new_virtual_pointer(struct wl_listener *listener,
                                       void *data);

void pointer_handle_destroy(struct wl_listener *listener, void *data);

bool pointer_configure(struct ashwc_pointer *pointer);

void server_reset_cursor_mode(void);

void cursor_handle_motion(uint32_t time);

void pointer_handle_focus(uint32_t time, bool handle_keyboard_focus);

void server_handle_cursor_motion(struct wl_listener *listener, void *data);

void server_handle_cursor_motion_absolute(struct wl_listener *listener,
                                          void *data);

void server_handle_cursor_button(struct wl_listener *listener, void *data);

void server_handle_cursor_axis(struct wl_listener *listener, void *data);

void server_handle_cursor_frame(struct wl_listener *listener, void *data);

void server_handle_new_constraint(struct wl_listener *listener, void *data);

void constraint_handle_destroy(struct wl_listener *listener, void *data);

void constrain_apply_to_move(double *dx, double *dy);

void constraint_remove_current(void);

void constraint_set_as_current(struct ashwc_pointer_constraint *constraint);

void constraint_move_to_hint(struct ashwc_pointer_constraint *constraint);

void server_handle_new_relative_pointer(struct wl_listener *listener,
                                        void *data);

void pointer_handle_swipe_begin(struct wl_listener *listener, void *data);

void pointer_handle_swipe_update(struct wl_listener *listener, void *data);

void pointer_handle_swipe_end(struct wl_listener *listener, void *data);

void server_handle_relative_pointer_manager_destroy(
    struct wl_listener *listener, void *data);

void pointer_destroy(void);
