server_src = [
  'main.c',
  'server.c',
  'net.c',
  'parser.c',
  'html.c',
  ]

server_obj = [
  'http-parser/libhttp_parser.a',
  ]

server_dep = [

  ]

server_exe = executable(
  'server',
  server_src,
  objects: server_obj,
  link_with: common_a,
  dependencies: server_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )
