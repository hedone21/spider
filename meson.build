project('spider', 'c',
  version: '0.0.1',
  license: 'mit')


cc = meson.get_compiler('c')
wayland_server_dep = dependency('wayland-server')
wayland_client_dep = dependency('wayland-client')
wayland_egl_dep = dependency('wayland-egl')
weston_dep = dependency('weston')
libweston6_dep = dependency('libweston-6')
gtk_dep = dependency('gtk+-3.0')
gtk_wayland_dep = dependency('gtk+-wayland-3.0')

subdir('protocol')
subdir('server')
