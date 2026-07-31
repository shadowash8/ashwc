project(
  'ashwc',
  'c',
  version: '0.2.0',
  license: 'MIT',
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wayland = dependency('wayland-server')
wayland_scanner = find_program('wayland-scanner')

wayland_protocols = dependency('wayland-protocols')
protocol_dir = wayland_protocols.get_variable('pkgdatadir')

protocol_files = [
  protocol_dir / 'stable/xdg-shell/xdg-shell.xml',
  protocol_dir / 'unstable/xdg-output/xdg-output-unstable-v1.xml',
  protocol_dir / 'unstable/pointer-constraints/pointer-constraints-unstable-v1.xml',
  protocol_dir / 'staging/cursor-shape/cursor-shape-v1.xml',
  'protocols/wlr-layer-shell-unstable-v1.xml',
]

generated = []
foreach f : protocol_files
  filename = f.split('/')[-1]
  dest = filename.replace('.xml', '-protocol.h')
  generated += custom_target(
    output: dest,
    input: f,
    command: [
      wayland_scanner, 'server-header',
      '@INPUT@', '@OUTPUT@'
    ],
    build_by_default: true
  )
endforeach

src = [
  'src/ashwc.c',
  'src/config/config.c',
  'src/decoration/decoration.c',
  'src/dnd/dnd.c',
  'src/gamma_control/gamma_control.c',
  'src/gestures/gestures.c',
  'src/helpers/helpers.c',
  'src/ipc/ipc.c',
  'src/keybinds/keybinds.c',
  'src/keyboard/keyboard.c',
  'src/layer_surface/layer_surface.c',
  'src/layout/layout.c',
  'src/layout/master.c',
  'src/layout/grid.c',
  'src/layout/monocle.c',
  'src/output/output.c',
  'src/pointer/pointer.c',
  'src/popup/popup.c',
  'src/rendering/rendering.c',
  'src/session_lock/session_lock.c',
  'src/something/something.c',
  'src/toplevel/toplevel.c',
  'src/workspace/workspace.c'
]

inc = [
  '.',
  'src',
  'util'
]

deps = [
  dependency('wayland-server'),
  dependency('scenefx-0.5', version: '>=0.5.0',),
  dependency('wlroots-0.20', version: ['>=0.20.0', '<0.21.0'], fallback: 'wlroots'),
  dependency('xkbcommon'),
  dependency('libinput'),
]

executable('ashwc',
  src,
  generated,
  dependencies: deps,
  include_directories: include_directories(inc),
  link_args : '-lm',
  install: true
)

executable('ashwc-ipc',
  'ashwc-ipc/ashwc-ipc.c',
  include_directories: include_directories('util'),
  install: true
)

install_data('default.conf', install_dir: get_option('datadir') / 'ashwc')
install_data('LICENSE', install_dir: get_option('datadir') / 'licenses/ashwc')
install_data('ashwc.desktop', install_dir: get_option('datadir') / 'wayland-sessions')
install_data('ashwc-portals.conf', install_dir: get_option('datadir') / 'xdg-desktop-portal')
