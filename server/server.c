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

/* 
 * This code is based on https://github.com/LambdaSchool/C-Web-Server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include <linux/limits.h>
#include "server/server.h"
#include "server/net.h"
#include "server/parser.h"
#include "server/http-parser/http_parser.h"
#include "common/log.h"

#define PORT "2580"

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, const char *path)
{
	const int request_buffer_size = 65536; // 64K
	char request[request_buffer_size];
	http_parser parser;
	http_parser_settings parser_settings;
	struct spider_client_info *info;
	int nparsed;

	info = calloc(1, sizeof(*info));
	if (info == NULL) {
		spider_err("allocation failed\n");
		return;
	}

	parser_init(&parser, HTTP_REQUEST);
	parser_settings_init(&parser_settings);

	// Read request
	int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

	info->root_path = path;
	info->client_fd = fd;

	parser.data = (void*)info;

	if (bytes_recvd < 0) {
		perror("recv");
		return;
	}else if (bytes_recvd == 0) {
		spider_err("connection closed\n");
		return;
	}

	// spider_dbg("%s\n", request);

	nparsed = http_parser_execute(&parser, &parser_settings, request, bytes_recvd);

	if (nparsed != bytes_recvd) {
		goto connection_close;
	}

	// (Stretch) If POST, handle the post request

connection_close:
	shutdown(fd, SHUT_RDWR);
}

int start_server(const char *path)
{
	int newfd;  // listen on sock_fd, new connection on newfd
	struct sockaddr_storage their_addr; // connector's address information
	char s[INET6_ADDRSTRLEN];

	// Get a listening socket
	int listenfd = get_listener_socket(PORT);

	if (listenfd < 0) {
		spider_err("webserver: fatal error getting listening socket\n");
		exit(1);
	}

	spider_dbg("webserver: waiting for connections on port %s...\n", PORT);

	// This is the main loop that accepts incoming connections and
	// responds to the request. The main parent process
	// then goes back to waiting for new connections.

	while(1) {
		socklen_t sin_size = sizeof their_addr;

		// Parent process will block on the accept() call until someone
		// makes a new connection:
		newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
		if (newfd == -1) {
			perror("accept");
			continue;
		}

		// Print out a message that we got the connection
		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s);
		spider_dbg("server: got connection from %s\n", s);

		// newfd is a new socket descriptor for the new connection.
		// listenfd is still listening for new connections.

		if (fork() == 0) {
			handle_http_request(newfd, path);
			spider_dbg("server: close from %s\n", s);

			close(newfd);
			exit(0);
		}
	}

	// Unreachable code

	return 0;
}
