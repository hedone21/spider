test_client_src = [
    'test_client.c',
]
e = executable(
        'test_client', 
        test_client_src,
        link_with: [common_a, server_a],
        dependencies: server_dep,
        include_directories: include_directories('../..'),
        )
test('Client Test', e)

test_client_mngr_src = [
    'test_client_mngr.c',
]
e = executable(
        'test_client_mngr', 
        test_client_mngr_src,
        link_with: [common_a, server_a],
        dependencies: server_dep,
        include_directories: include_directories('../..'),
        )
test('Client Mngr Test', e)

test_event_src = [
    'test_event.c',
]
e = executable(
        'test_event', 
        test_event_src,
        link_with: [common_a, server_a],
        dependencies: server_dep,
        include_directories: include_directories('../..'),
        )
test('Event Test', e)

test_server_src = [
    'test_server.c',
]
e = executable(
        'test_server', 
        test_server_src,
        link_with: [common_a, server_a],
        dependencies: server_dep,
        include_directories: include_directories('../..'),
        )
test('Server Test', e)
