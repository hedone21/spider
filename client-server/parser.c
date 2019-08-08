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
#include "client-server/server.h"
#include "client-server/parser.h"
#include "client-server/html.h"
#include "client-server/command.h"
#include "common/log.h"

#define BUF_MAX 65536

static const char* get_http_method(http_parser *parser)
{
	const char *http_method = NULL;
	switch (parser->method) {
	case HTTP_DELETE:
		http_method = "HTTP_METHOD";	
		break;
	case HTTP_GET:
		http_method = "HTTP_GET";
		break;
	case HTTP_HEAD:
		http_method = "HTTP_HEAD";
		break;
	case HTTP_POST:
		http_method = "HTTP_POST";
		break;
	case HTTP_PUT:
		http_method = "HTTP_PUT";
		break;
	case HTTP_CONNECT:
		http_method = "HTTP_CONNECT";
		break;
	case HTTP_OPTIONS:
		http_method = "HTTP_OPTIONS";
		break;
	case HTTP_TRACE:
		http_method = "HTTP_TRACE";
		break;
	case HTTP_COPY:
		http_method = "HTTP_COPY";
		break;
	case HTTP_LOCK:
		http_method = "HTTP_LOCK";
		break;
	case HTTP_MKCOL:
		http_method = "HTTP_MKCOL";
		break;
	case HTTP_MOVE:
		http_method = "HTTP_MOVE";
		break;
	case HTTP_PROPFIND:
		http_method = "HTTP_PROPFIND";
		break;
	case HTTP_PROPPATCH:
		http_method = "HTTP_PROPPATCH";
		break;
	case HTTP_SEARCH:
		http_method = "HTTP_SEARCH";
		break;
	case HTTP_UNLOCK:
		http_method = "HTTP_UNLOCK";
		break;
	case HTTP_BIND:
		http_method = "HTTP_BIND";
		break;
	case HTTP_REBIND:
		http_method = "HTTP_REBIND";
		break;
	case HTTP_UNBIND:
		http_method = "HTTP_UNBIND";
		break;
	case HTTP_ACL:
		http_method = "HTTP_ACL";
		break;
	case HTTP_REPORT:
		http_method = "HTTP_REPORT";
		break;
	case HTTP_MKACTIVITY:
		http_method = "HTTP_MKACTIVITY";
		break;
	case HTTP_CHECKOUT:
		http_method = "HTTP_CHECKOUT";
		break;
	case HTTP_MERGE:
		http_method = "HTTP_MERGE";
		break;
	case HTTP_MSEARCH:
		http_method = "HTTP_MSEARCH";
		break;
	case HTTP_NOTIFY:
		http_method = "HTTP_NOTIFY";
		break;
	case HTTP_SUBSCRIBE:
		http_method = "HTTP_SUBSCRIBE";
		break;
	case HTTP_UNSUBSCRIBE:
		http_method = "HTTP_UNSUBSCRIBE";
		break;
	case HTTP_PATCH:
		http_method = "HTTP_PATCH";
		break;
	case HTTP_PURGE:
		http_method = "HTTP_PURGE";
		break;
	case HTTP_MKCALENDAR:
		http_method = "HTTP_MKCALENDAR";
		break;
	case HTTP_LINK:
		http_method = "HTTP_LINK";
		break;
	case HTTP_UNLINK:
		http_method = "HTTP_UNLINK";
		break;
	case HTTP_SOURCE:
		http_method = "HTTP_SOURCE";
		break;
	default:
		spider_err("Unregistered http method\n");
	}

	return http_method;
}

static int on_url_cb(http_parser *parser, const char *at, size_t length)
{
	struct spider_client_info *info;
	char url[BUF_MAX] = {0, };

	assert(parser);

	info = parser->data;
	if (info == NULL) {
		spider_err("http_parser date is null\n");
		// FIXME return proper error code
		return 1;
	}

	strncpy(url, at, length);
	spider_dbg("%s(%s)\n", url, get_http_method(parser));

	if (strncmp(url, "/", 2) == 0 || strncmp(url, "/index.html", 12) == 0) {
		html_load_index(info);
	}else if (strncmp(url, "/command", 9) == 0) {
		html_load_command(info);
	}

	return 0;
}

static int on_status_cb(http_parser *parser, const char *at, size_t length)
{
	char status[BUF_MAX] = {0, };
	assert(parser);

	strncpy(status, at, length);

	spider_dbg("%s\n", status);

	return 0;
}

static int on_header_field_cb(http_parser *parser, const char *at, size_t length)
{
	char header_field[BUF_MAX] = {0, };
	assert(parser);

	strncpy(header_field, at, length);

	spider_dbg("%s\n", header_field);

	return 0;
}

static int on_header_value_cb(http_parser *parser, const char *at, size_t length)
{
	char header_value[BUF_MAX] = {0, };
	assert(parser);

	strncpy(header_value, at, length);

	spider_dbg("%s\n", header_value);

	return 0;
}

static int on_body_cb(http_parser *parser, const char *at, size_t length)
{
	char body[BUF_MAX] = {0, };
	assert(parser);

	strncpy(body, at, length);

	spider_dbg("%s\n", body);

	if (strncmp(body, "command", 7) == 0) {
		command_launch(body + 8);
	}

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
