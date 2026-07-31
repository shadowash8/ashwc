#include "gestures.h"

#include "ashwc.h"

extern struct ashwc_server server;

void handle_swipe(enum gesture_direction direction, uint32_t fingers) {
  struct gesture *g;

  wl_list_for_each(g, &server.config->gestures, link) {
    if (g->type != GESTURE_SWIPE)
      continue;

    if (g->direction != direction)
      continue;

    if (g->fingers != fingers)
      continue;

    if (g->action)
      g->action(g->args);
  }
}
