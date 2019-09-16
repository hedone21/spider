common_src = [
  'command.c',
  'webkitapi.c',
  ]

common_dep = [
  gtk_dep,
  webkitgtk_dep,
  ]

common_a = static_library(
  'common',
  common_src,
  dependencies: common_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )
