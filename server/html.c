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
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/limits.h>
#include "server/html.h"
#include "common/log.h"

#define RESP_BUF_MAX 1024 * 1024 * 4 // 4M

static void load_html(int client_fd, const char *path, const char *html)
{
	char html_path[PATH_MAX] = {0, };
	int html_fd;
	char response[RESP_BUF_MAX] = {0, };
	int bytes_recvd;

	strncpy(html_path, path, PATH_MAX);
	strncat(html_path, html, PATH_MAX);

	spider_dbg("html_path: %s\n", html_path);

	if ((html_fd = open(html_path, O_RDONLY | O_CLOEXEC)) != -1) {
		send(client_fd, "HTTP/1.0 200 OK\n\n", 17, 0);
		while((bytes_recvd = read(html_fd, response, RESP_BUF_MAX)) > 0) {
			// spider_dbg("[response]\n%s\n", response);
			write(client_fd, response, bytes_recvd);
		}
	}else {
		write(client_fd, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
	}

	close(html_fd);
}

void html_load_index(struct spider_client_info *info)
{
	load_html(info->client_fd, info->root_path, "/index.html");
}

void html_load_command(struct spider_client_info *info)
{
	load_html(info->client_fd, info->root_path, "/index.html");
}
