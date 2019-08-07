cli_server_src = [
  'main.c',
  'server.c',
  'net.c',
  'parser.c',
  ]

cli_server_dep = [

  ]

cli_server_exe = executable(
  'client-server',
  cli_server_src,
  objects: 'http-parser/libhttp_parser.a',
  dependencies: cli_server_dep,
  include_directories: include_directories('..'),
  name_prefix: '',
  )
