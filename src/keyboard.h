#pragma once

#include <wlr/types/wlr_keyboard.h>

struct ashwc_keyboard {
  struct wl_list link;
  struct wlr_keyboard *wlr_keyboard;
  /* used for getting raw keysyms for keybinds */
  struct xkb_state *empty;

  struct wl_listener modifiers;
  struct wl_listener key;
  struct wl_listener destroy;
};

void server_handle_new_keyboard(struct wlr_input_device *device);

void keyboard_handle_modifiers(struct wl_listener *listener, void *data);

void keyboard_handle_key(struct wl_listener *listener, void *data);

void keyboard_handle_destroy(struct wl_listener *listener, void *data);

bool keyboard_configure(struct ashwc_keyboard *keyboard);

void keyboard_destroy(void);

void server_handle_new_keyboard_shortcuts_inhibitor(
    struct wl_listener *listener, void *data);
