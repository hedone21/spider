server_src = [
  'server.c',
  ]

server_dep = [
  wayland_server_dep,
  wayland_egl_dep,
  weston_dep,
  libweston6_dep,
  ]

server_so = shared_library(
  'spider-server',
  server_src,
  spider_protocol_c,
  spider_server_protocol_h,
  xdg_shell_protocol_c,
  xdg_shell_server_protocol_h,
  xdg_shell_unstable_v6_protocol_c,
  xdg_shell_unstable_v6_server_protocol_h,
  dependencies: server_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )

