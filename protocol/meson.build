scanner_dep = dependency('wayland-scanner', native: true)
scanner_prog = find_program(scanner_dep.get_pkgconfig_variable('wayland_scanner'))

protocols_dep = dependency('wayland-protocols', version: '>= 1.17')
protocols_base_dir = protocols_dep.get_pkgconfig_variable('pkgdatadir')

protocols_xml = [
  ['spider', 'internal'],
  ['xdg-shell', 'v6' ],
  ['xdg-shell', 'stable' ],
  ]

foreach p : protocols_xml
  protocol_name = p[0]

  if p[1] == 'internal'
    base_file = protocol_name
    protocol_path = '@0@.xml'.format(base_file)
  elif p[1] == 'stable'
    base_file = protocol_name
    protocol_path = '@0@/stable/@1@/@1@.xml'.format(protocols_base_dir, base_file)
  else
    base_file = '@0@-unstable-@1@'.format(protocol_name, p[1])
    protocol_path = '@0@/unstable/@1@/@2@.xml'.format(protocols_base_dir, protocol_name, base_file)
  endif

  foreach output_type: [ 'client-header', 'server-header', 'private-code' ]
    if output_type == 'client-header'
      output_file = '@0@-client-protocol.h'.format(base_file)
    elif output_type == 'server-header'
      output_file = '@0@-protocol.h'.format(base_file)
    else
      output_file = '@0@-protocol.c'.format(base_file)
      if scanner_dep.version().version_compare('< 1.14.91')
	output_type = 'code'
      endif
    endif

    var_name = output_file.underscorify()
    target = custom_target(
      '@0@ @1@'.format(base_file, output_type),
      command: [ scanner_prog, output_type, '@INPUT@', '@OUTPUT@' ],
      input: protocol_path,
      output: output_file,
      )

    set_variable(var_name, target)
  endforeach
endforeach
