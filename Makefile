PKG_CONFIG?=pkg-config
OUT = ashwc
PREFIX ?= /usr/local
BINDIR  = $(PREFIX)/bin
SRC = ashwc.c protocols/wlr-layer-shell-unstable-v1-protocol.c protocols/xdg-shell-protocol.c
PKGS="wlroots-0.20" wayland-server xkbcommon libinput
CFLAGS_PKG_CONFIG!=$(PKG_CONFIG) --cflags $(PKGS)
CFLAGS += $(CFLAGS_PKG_CONFIG) -DWLR_USE_UNSTABLE -I. -Iprotocols
LIBS!=$(PKG_CONFIG) --libs $(PKGS)
WAYLAND_SCANNER = `$(PKG_CONFIG) --variable=wayland_scanner wayland-scanner`

all: protocols $(OUT)

protocols:
	mkdir -p protocols

protocols/wlr-layer-shell-unstable-v1-protocol.h: protocols
	$(WAYLAND_SCANNER) server-header /usr/share/wlr-protocols/unstable/wlr-layer-shell-unstable-v1.xml $@

protocols/wlr-layer-shell-unstable-v1-protocol.c: protocols
	$(WAYLAND_SCANNER) private-code /usr/share/wlr-protocols/unstable/wlr-layer-shell-unstable-v1.xml $@

protocols/xdg-shell-protocol.h: protocols
	$(WAYLAND_SCANNER) server-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml $@

protocols/xdg-shell-protocol.c: protocols
	$(WAYLAND_SCANNER) private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml $@

$(OUT): $(SRC) protocols/wlr-layer-shell-unstable-v1-protocol.h protocols/xdg-shell-protocol.h
	$(CC) $(SRC) -g -Werror $(CFLAGS) $(LDFLAGS) $(LIBS) -o $(OUT)

install: $(OUT)
	install -Dm755 $(OUT) $(BINDIR)/$(OUT)

uninstall:
	rm -f $(BINDIR)/$(OUT)

clean:
	rm -f $(OUT)
	rm -rf protocols
