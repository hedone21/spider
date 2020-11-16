libserver_src = [
    'client.c',
    'client_mngr.c',
    'iter.c',
    'keybindings.c',
    'message.c',
    'server.c',
]

server_src = [
    'main.c',
]

server_dep = [
    dl_dep,
    glib_dep,
    wayland_server_dep,
    wayland_egl_dep,
    xkbcommon_dep,
    wlr_dep,
]

subdir('backend')

server_a = static_library(
        'spider_server',
        libserver_src,
        link_with: [common_a, backend_a],
        dependencies: server_dep,
        include_directories: include_directories('..'),
        name_prefix: '',
        )

server_exe = executable(
        'spider',
        server_src,
        spider_server_manager_v1_protocol_c,
        spider_server_manager_v1_protocol_h,
        xdg_shell_protocol_c,
        xdg_shell_protocol_h,
        xdg_shell_unstable_v6_protocol_c,
        xdg_shell_unstable_v6_protocol_h,
        wlr_layer_shell_unstable_v1_protocol_c,
        wlr_layer_shell_unstable_v1_protocol_h,
        link_with: [common_a, server_a],
        dependencies: server_dep,
        include_directories: include_directories('..'),
        name_prefix: '',
        )

subdir('tests')
