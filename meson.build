project('spider', 'c',
  version: '0.0.1',
  license: 'mit')


cc = meson.get_compiler('c')
wayland_server_dep = dependency('wayland-server')
wayland_client_dep = dependency('wayland-client')
wayland_egl_dep = dependency('wayland-egl')
gtk_dep = dependency('gtk+-3.0')
gtk_wayland_dep = dependency('gtk+-wayland-3.0')
wlr_dep = dependency('wlroots')

subdir('server')
