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
#include "client-server/server.h"
#include "client-server/net.h"
#include "common/log.h"

#define PORT "2580"

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 * 
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
	const int max_response_size = 262144;
	char response[max_response_size];

	// Build HTTP response and store it in response

	///////////////////
	// IMPLEMENT ME! //
	///////////////////

	// Send it all!
	int rv = send(fd, response, max_response_size, 0);

	if (rv < 0) {
		perror("send");
	}

	return rv;
}

/**
 * Search for the end of the HTTP header
 * 
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
	///////////////////
	// IMPLEMENT ME! // (Stretch)
	///////////////////
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, const char *path)
{
	const int request_buffer_size = 65536; // 64K
	char request[request_buffer_size];
	char *reqline[3];
	const int response_buffer_size = 1024 * 1024 * 4; // 4M
	char response[response_buffer_size];
	int html_fd;
	char html_path[PATH_MAX];

	// Read request
	int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

	if (bytes_recvd < 0) {
		perror("recv");
		return;
	}else if (bytes_recvd == 0) {
		spider_err("connection closed\n");
		return;
	}

	spider_dbg("===[http]===\n%s", request);

	reqline[0] = strtok(request, " \t\n");
	if (strncmp(reqline[0], "GET\0", 4) == 0) {
		reqline[1] = strtok(NULL, " \t");
		reqline[2] = strtok(NULL, " \t\n");

		if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 && strncmp(reqline[2], "HTTP/1.1", 8) != 0) {
			write(fd, "HTTP/1.0 400 Bad Request\n", 25);
			return;
		}

		if (strncmp(reqline[1], "/\0", 2) == 0)
			reqline[1] = "/index.html";

		strncpy(html_path, path, PATH_MAX);
		strncat(html_path, reqline[1], PATH_MAX);

		spider_dbg("\nhttp method: %s\npath: %s\n", reqline[0], html_path);
		if ((html_fd = open(html_path, O_RDONLY)) != -1) {
			send(fd, "HTTP/1.0 200 OK\n\n", 17, 0);
			while((bytes_recvd = read(html_fd, response, response_buffer_size)) > 0) {
				write(fd, response, bytes_recvd);
			}
		}else {
			write(fd, "HTTP/1.0 404 Not Found\n", 23); //FILE NOT FOUND
		}
	}

	shutdown(fd, SHUT_RDWR);
	// (Stretch) If POST, handle the post request
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
