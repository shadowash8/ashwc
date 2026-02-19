PKG_CONFIG?=pkg-config

OUT = ashwc
SRC = ashwc.c

PKGS="wlroots-0.20" wayland-server xkbcommon
CFLAGS_PKG_CONFIG!=$(PKG_CONFIG) --cflags $(PKGS)
CFLAGS += $(CFLAGS_PKG_CONFIG) -DWLR_USE_UNSTABLE
LIBS!=$(PKG_CONFIG) --libs $(PKGS)

all: ${OUT}

${OUT}: ${SRC}
	${CC} $^ -g -Werror ${CFLAGS} ${LDFLAGS} ${LIBS} -o ${OUT}

clean:
	rm -f ${OUT}


