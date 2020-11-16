project('spider', 'c',
  version: '0.0.1',
  license: 'mit')

add_project_arguments(
	[
		'-DWLR_USE_UNSTABLE',
	],
	language: 'c',
)


cc = meson.get_compiler('c')
dl_dep = meson.get_compiler('c').find_library('dl', required : true)
wayland_server_dep = dependency('wayland-server')
wayland_client_dep = dependency('wayland-client')
wayland_egl_dep = dependency('wayland-egl')
glib_dep = dependency('glib-2.0')
gtk_dep = dependency('gtk+-3.0')
gtk_wayland_dep = dependency('gtk+-wayland-3.0')
webkitgtk_dep = dependency('webkit2gtk-4.0')
xkbcommon_dep = dependency('xkbcommon')
wlroots_version = '>=0.6'
wlr_dep = dependency('wlroots', version: wlroots_version)

subdir('protocol')
subdir('common')
subdir('server')
subdir('shell')
subdir('panel')
