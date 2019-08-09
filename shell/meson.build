shell_src = [
  'main.c',
  ]

shell_dep = [
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
