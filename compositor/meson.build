compositor_src = [
  'main.c',
  'compositor.c',
  'cursor.c',
  ]

compositor_dep = [
  wayland_server_dep,
  wayland_egl_dep,
  xkbcommon_dep,
  wlr_dep,
  ]

compositor_exe = executable(
  'spider-compositor',
  compositor_src,
  spider_protocol_c,
  spider_protocol_h,
  xdg_shell_protocol_c,
  xdg_shell_protocol_h,
  xdg_shell_unstable_v6_protocol_c,
  xdg_shell_unstable_v6_protocol_h,
  dependencies: compositor_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )

