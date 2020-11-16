libmock_src = [
    'mock.c',
    'server.c',
]

libmock_dep = [
    glib_dep,
    wayland_server_dep,
    wayland_egl_dep,
    xkbcommon_dep,
    wlr_dep,
]

mock_so = shared_library(
        '../../spider_backend_mock',
        libmock_src,
        spider_server_manager_v1_protocol_c,
        spider_server_manager_v1_protocol_h,
        xdg_shell_protocol_c,
        xdg_shell_protocol_h,
        xdg_shell_unstable_v6_protocol_c,
        xdg_shell_unstable_v6_protocol_h,
        wlr_layer_shell_unstable_v1_protocol_c,
        wlr_layer_shell_unstable_v1_protocol_h,
        dependencies: libmock_dep,
        include_directories: include_directories('../../..'),
        name_prefix: '',
        )

# subdir('tests')
