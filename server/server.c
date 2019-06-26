/*
 * Copyright (c) 2019 Minyoung.Go <hedone21@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <wayland-server.h>
#include <weston.h>
#include <libweston-6/compositor.h>
#include "server/server.h"
#include "common/log.h"
#include "protocol/spider-server-protocol.h"
#include "protocol/xdg-shell-server-protocol.h"
#include "protocol/xdg-shell-unstable-v6-server-protocol.h"

/* deprecated */
static void get_shell_surface(struct wl_client *client, 
		struct wl_resource *resource, uint32_t id, 
		struct wl_resource *surface_resource)
{
}

static const struct wl_shell_interface wl_shell_implementation = {
	.get_shell_surface = get_shell_surface,
};

static void bind_wl_shell(struct wl_client *client, void *data, 
		uint32_t version, uint32_t id)
{
	struct wl_resource *resource;
	struct ol_shell *shell = data;

	resource = wl_resource_create(client, &wl_shell_interface, version, id);
	wl_resource_set_implementation(resource, &wl_shell_implementation,
			shell, NULL);
}

static const struct xdg_wm_base_interface xdg_shell_implementation = {
	.destroy = NULL,
	.create_positioner = NULL,
	.get_xdg_surface = NULL,
	.pong = NULL,
};

static void bind_xdg_shell(struct wl_client *client, void *data, 
		uint32_t version, uint32_t id)
{
	struct wl_resource *resource;
	struct ol_shell *shell = data;

	resource = wl_resource_create(client, &xdg_wm_base_interface, version, id);
	wl_resource_set_implementation(resource, &xdg_shell_implementation,
			shell, NULL);
}

static const struct spider_interface spider_implementation = {
	.xxx = NULL,
};

static void bind_spider(struct wl_client *client, void *data, 
		uint32_t version, uint32_t id)
{
	struct wl_resource *resource;
	struct ol_shell *shell = data;

	resource = wl_resource_create(client, &spider_interface, 1, id);
	/* TODO: need setting permission */
	wl_resource_set_implementation(resource, &spider_implementation, shell, 
			NULL);
}

static void get_wet_config(struct spider_server *server)
{

}

static void launch_shell_process(void *data)
{
	struct spider_server *server = data;
	char *path;
	int sv[2];
	pid_t pid;
	int ret;
	long flags;
	struct wl_client *client;

	path = getenv("SPIDER_CLIENT");

	spider_log("launching '%s'\n", path);

	ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv);
	if (ret < 0)
		return;

	flags = fcntl(sv[0], F_GETFD);
	fcntl(sv[0], F_SETFD, flags | FD_CLOEXEC);
	flags = fcntl(sv[1], F_GETFD);
	fcntl(sv[0], F_SETFD, flags | FD_CLOEXEC);

	pid = fork();
	if (pid == -1) {
		close(sv[0]);
		close(sv[1]);
		spider_log("weston_client_launch: "
			"fork failed while launching '%s': %m\n", path);
		return;
	}

	if (pid == 0) {
		int cfd;
		char s[32];
		sigset_t allsigs;
		sigfillset(&allsigs);
		sigprocmask(SIG_UNBLOCK, &allsigs, NULL);

		cfd = dup(sv[1]);
		snprintf(s, sizeof s, "%d", cfd);
		setenv("WAYLAND_SOCKET", s, 1);
		spider_log("execl '%s'\n", path);

		execl(path, path, NULL);

		_exit(-1);
	}

	close(sv[1]);

	client = wl_client_create(server->compositor->wl_display, sv[0]);
	if (!client) {
		close(sv[0]);
		return;
	}

	return;
}

WL_EXPORT int
wet_shell_init(struct weston_compositor *ec, int *argc, char *argv[])
{
	struct spider_server *server;
	struct wl_event_loop *loop;

	server = (struct spider_server*)calloc(1, sizeof(*server));
	if (server == NULL) {
		return EXIT_FAILURE;
	}

	server->compositor = ec;

	get_wet_config(server);

	wl_global_create(ec->wl_display, &xdg_wm_base_interface,
			1, server,
			bind_xdg_shell);

	wl_global_create(ec->wl_display, &wl_shell_interface,
			1, server,
			bind_wl_shell);

	weston_layer_init(&server->background_layer, ec);
	weston_layer_set_position(&server->background_layer,
			WESTON_LAYER_POSITION_BACKGROUND);

	if (wl_global_create(ec->wl_display, &spider_interface, 1,
				server, bind_spider) == NULL)

		return -1;

	loop = wl_display_get_event_loop(ec->wl_display);
	wl_event_loop_add_idle(loop, launch_shell_process, server);

	return 0;
}
