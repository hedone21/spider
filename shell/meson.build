shell_src = [
  'main.c',
  'shell.c',
  ]

shell_dep = [
  wayland_client_dep,
  wlr_dep,
  gtk_dep,
  webkitgtk_dep,
  ]

shell_exe = executable(
  'client', 
  shell_src,
  spider_protocol_c,
  spider_client_protocol_h,
  xdg_shell_protocol_c,
  xdg_shell_client_protocol_h,
  xdg_shell_unstable_v6_protocol_c,
  xdg_shell_unstable_v6_client_protocol_h,
  wlr_layer_shell_unstable_v1_protocol_c,
  wlr_layer_shell_unstable_v1_client_protocol_h,
  dependencies : shell_dep, 
  include_directories: include_directories('..', '../protocol'),
  name_prefix: '',
  )
