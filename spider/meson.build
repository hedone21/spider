compositor_src = [
  'main.c',
  'cursor.c',
  'compositor.c',
  'input.c',
  'launcher.c',
  'layer.c',
  'output.c',
  'seat.c',
  'view.c',
  'xdg_shell.c',
  ]

compositor_dep = [
  wayland_server_dep,
  wayland_egl_dep,
  xkbcommon_dep,
  wlr_dep,
  ]

compositor_exe = executable(
  'spider',
  compositor_src,
  spider_compositor_manager_v1_protocol_c,
  spider_compositor_manager_v1_protocol_h,
  xdg_shell_protocol_c,
  xdg_shell_protocol_h,
  xdg_shell_unstable_v6_protocol_c,
  xdg_shell_unstable_v6_protocol_h,
  wlr_layer_shell_unstable_v1_protocol_c,
  wlr_layer_shell_unstable_v1_protocol_h,
  link_with: common_a,
  dependencies: compositor_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )

