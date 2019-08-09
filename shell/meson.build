shell_src = [
  'main.c',
  'shell.c',
  ]

shell_dep = [
  wayland_client_dep,
  gtk_dep,
  webkitgtk_dep,
  ]

shell_exe = executable(
  'client', 
  shell_src,
  dependencies : shell_dep, 
  include_directories: include_directories('..'),
  name_prefix: '',
  )
