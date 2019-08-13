server_src = [
  'main.c',
  'server.c',
  'net.c',
  'parser.c',
  'html.c',
  'command.c',
  ]

server_dep = [

  ]

server_exe = executable(
  'server',
  server_src,
  objects: 'http-parser/libhttp_parser.a',
  dependencies: server_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )
