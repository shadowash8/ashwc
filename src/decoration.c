#include "decoration.h"

#include "ashwc.h"
#include "config.h"

#include <wlr/types/wlr_xdg_decoration_v1.h>

extern struct ashwc_server server;
void server_handle_request_xdg_decoration(struct wl_listener *listener,
                                          void *data) {
  struct wlr_xdg_toplevel_decoration_v1 *decoration = data;

  if (!decoration->toplevel->base->initialized) {
    return;
  }

  wlr_xdg_toplevel_decoration_v1_set_mode(
      decoration, server.config->client_side_decorations
                      ? WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE
                      : WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

void decoration_destroy() {
  wl_list_remove(&server.request_xdg_decoration.link);
}
