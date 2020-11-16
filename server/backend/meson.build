libbackend_src = [
    'backend.c',
    'server.c',
]

libbackend_dep = [
    glib_dep,
    wayland_server_dep,
    wayland_egl_dep,
    xkbcommon_dep,
    wlr_dep,
]

backend_a = static_library(
        'spider_backend',
        libbackend_src,
        spider_server_manager_v1_protocol_c,
        spider_server_manager_v1_protocol_h,
        xdg_shell_protocol_c,
        xdg_shell_protocol_h,
        xdg_shell_unstable_v6_protocol_c,
        xdg_shell_unstable_v6_protocol_h,
        wlr_layer_shell_unstable_v1_protocol_c,
        wlr_layer_shell_unstable_v1_protocol_h,
        dependencies: libbackend_dep,
        include_directories: include_directories('../..'),
        name_prefix: '',
        )

# subdir('wlroots')
