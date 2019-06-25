server_src = [
  'main.c',
  'server.c',
  ]

server_dep = [
  wayland_server_dep,
  wayland_egl_dep,
  wlr_dep,
  ]

server_so = executable(
  'spider-server',
  server_src,
  dependencies: server_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )

