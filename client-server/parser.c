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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <linux/limits.h>
#include "client-server/server.h"
#include "client-server/parser.h"
#include "common/log.h"

#define BUF_MAX 65536

static int on_url_cb(http_parser *parser, const char *at, size_t length)
{
	struct spider_client_info *info;
	char url[BUF_MAX];
	char html_path[PATH_MAX];
	int html_fd;

	const int response_buffer_size = 1024 * 1024 * 4; // 4M
	char response[response_buffer_size];
	int bytes_recvd;

	assert(parser);

	info = parser->data;
	if (info == NULL) {
		spider_err("http_parser date is null\n");
		// FIXME return proper error code
		return 1;
	}

	strncpy(url, at, length);
	spider_dbg("%s\n", url);

	if (strncmp(url, "/", 2) == 0) {
		strncpy(url, "/index.html", 12);
	}

	strncpy(html_path, info->root_path, PATH_MAX);
	strncat(html_path, url, PATH_MAX);

	spider_dbg("html_path: %s\n", html_path);

	if ((html_fd = open(html_path, O_RDONLY)) != -1) {	
		send(info->client_fd, "HTTP/1.0 200 OK\n\n", 17, 0);
		while((bytes_recvd = read(html_fd, response, response_buffer_size)) > 0) {
			spider_dbg("[response]\n%s\n", response);
			write(info->client_fd, response, bytes_recvd);
		}
	}else {
		write(info->client_fd, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
	}

	close(html_fd);

	return 0;
}

static int on_status_cb(http_parser *parser, const char *at, size_t length)
{
	char status[BUF_MAX];
	assert(parser);

	strncpy(status, at, length);

	spider_dbg("%s\n", status);

	return 0;
}

static int on_header_field_cb(http_parser *parser, const char *at, size_t length)
{
	char header_field[BUF_MAX];
	assert(parser);

	strncpy(header_field, at, length);

	spider_dbg("%s\n", header_field);

	return 0;
}

static int on_header_value_cb(http_parser *parser, const char *at, size_t length)
{
	char header_value[BUF_MAX];
	assert(parser);

	strncpy(header_value, at, length);

	spider_dbg("%s\n", header_value);

	return 0;
}

static int on_body_cb(http_parser *parser, const char *at, size_t length)
{
	char body[BUF_MAX];
	assert(parser);

	strncpy(body, at, length);

	spider_dbg("%s\n", body);

	return 0;
}

void parser_init(http_parser *parser, enum http_parser_type type)
{
	memset(parser, 0, sizeof(*parser));
	http_parser_init(parser, type);
}

void parser_settings_init(http_parser_settings *parser_settings)
{
	memset(parser_settings, 0, sizeof(*parser_settings));
	parser_settings->on_url = on_url_cb;
	parser_settings->on_status = on_status_cb;
	parser_settings->on_header_field = on_header_field_cb;
	parser_settings->on_header_value = on_header_value_cb;
	parser_settings->on_body = on_body_cb;
}
