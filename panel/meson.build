panel_src = [
  'main.c',
  'panel.c',
  ]

panel_dep = [
  wayland_client_dep,
  wlr_dep,
  gtk_dep,
  webkitgtk_dep,
  ]

panel_exe = executable(
  'panel', 
  panel_src,
  xdg_shell_protocol_c,
  xdg_shell_client_protocol_h,
  xdg_shell_unstable_v6_protocol_c,
  xdg_shell_unstable_v6_client_protocol_h,
  wlr_layer_shell_unstable_v1_protocol_c,
  wlr_layer_shell_unstable_v1_client_protocol_h,
  link_with: common_a,
  dependencies : panel_dep, 
  include_directories: include_directories('..', '../protocol'),
  name_prefix: '',
  )

