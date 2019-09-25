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

#include <fcntl.h>
#include <signal.h>
#include "spider/launcher.h"
#include "common/log.h"

static int set_cloexec(int fd)
{
	long flags;

	if (fd < 0)
		return -1;

	flags = fcntl(fd, F_GETFD);
	if (flags < 0)
		return flags;

	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		return -1;

	return 0;
}

static int set_cloexec_or_close(int fd)
{
	if (set_cloexec(fd) != 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static int socketpair_cloexec(int domain, int type, int protocol, int *sv)
{
	int ret;
	ret = socketpair(domain, type | SOCK_CLOEXEC, protocol, sv);
	if (ret == 0)
		return ret;

	sv[0] = set_cloexec_or_close(sv[0]);
	if (sv[0] < 0) {
		goto error;
	}

	sv[1] = set_cloexec_or_close(sv[1]);
	if (sv[1] < 0) {
		goto error;
	}

	return 0;

error:
	spider_err("Failed to make socket_vector w/ cloexec\n");

	close(sv[0]);
	close(sv[1]);
	return -1;
}

static int fork_n_exec(const char *path, int *sv)
{
	int child_pid;

	child_pid = fork();
	if (child_pid == 0) {
		sigset_t sigs;
		int clientfd;
		char s[32];

		sigfillset(&sigs);
		sigprocmask(SIG_UNBLOCK, &sigs, NULL);

		seteuid(getuid());
		clientfd = dup(sv[1]);

		snprintf(s, sizeof s, "%d", clientfd);
		setenv("WAYLAND_SOCKET", s, 1);

		spider_dbg("Launch shell [%s]\n", path);
		execl("/bin/sh", "/bin/sh", "-c", path, (void *)NULL);

		exit(-1);
	}else if (child_pid < 0) {
		spider_err("Failed to fork\n");
		exit(-1);
	}

	return child_pid;
}

void launch_client(void *data)
{
	int sv[2];
	int child_pid;
	struct spider_desktop *desktop = data;

	socketpair_cloexec(AF_UNIX, SOCK_STREAM, 0, sv);
	child_pid = fork_n_exec(g_options.shell, sv);
	desktop->client_shell_pid = child_pid;
	close(sv[1]);
	wl_client_create(desktop->wl_display, sv[0]);

	socketpair_cloexec(AF_UNIX, SOCK_STREAM, 0, sv);
	child_pid = fork_n_exec(g_options.panel, sv);
	desktop->client_panel_pid = child_pid;
	close(sv[1]);
	wl_client_create(desktop->wl_display, sv[0]);
}
