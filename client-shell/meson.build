cli_gtk_src = [
  'main.c',
  ]

cli_gtk_dep = [
  gtk_dep,
  webkitgtk_dep,
  ]

cli_gtk_exe = executable(
  'client', 
  'main.c', 
  dependencies : cli_gtk_dep, 
  include_directories: include_directories('..'),
  name_prefix: '',
  )
