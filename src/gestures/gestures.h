#pragma once

#include "config/config.h"
#include "keyboard/keyboard.h"

typedef void (*keybind_action_func_t)(void *);

enum gesture_type {
  GESTURE_SWIPE,
  GESTURE_PINCH,
};

enum gesture_direction {
  GESTURE_LEFT,
  GESTURE_RIGHT,
  GESTURE_UP,
  GESTURE_DOWN,
  GESTURE_IN,
  GESTURE_OUT,
};

struct gesture {
  struct wl_list link;

  enum gesture_type type;
  uint32_t fingers;
  enum gesture_direction direction;

  keybind_action_func_t action;
  void *args;
};

void handle_swipe(enum gesture_direction direction, uint32_t fingers);
