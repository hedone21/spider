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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <linux/limits.h>
#include "spider/compositor.h"
#include "common/log.h"
#include "common/global_vars.h"

int SPIDER_LOGLEVEL = 2;

struct spider_options g_options = {
	.shell = NULL,
	.panel = NULL,
};

void help()
{
	/* TODO */
}

int main(int argc, char *argv[]) 
{
	char *loglevel;

	static struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"debug", no_argument, NULL, 'd'},
		{"verbose", no_argument, NULL, 'V'},
		{"version", no_argument, NULL, 'v'},
		{"panel", required_argument, NULL, 'p'},
		{"shell", required_argument, NULL, 's'},
		{"server", required_argument, NULL, 'r'},
		{0, 0, 0, 0}
	};

	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "hdVvp:s:r:", long_options, &option_index)) != -1) {
		int arglen;

		switch (c) {
		case 'd':
			g_options.debug = true;
			break;
		case 'V':
			g_options.verbose = true;
			break;
		case 'v':
			/* TODO */
		case 'p':
			arglen = strlen(optarg);
			if (arglen >= PATH_MAX) {
				return -1;
			}

			g_options.panel = malloc(sizeof(char) * (arglen + 1));
			strcpy(g_options.panel, optarg);
			break;
			break;
		case 's':
			arglen = strlen(optarg);
			if (arglen >= PATH_MAX) {
				return -1;
			}

			g_options.shell = malloc(sizeof(char) * (arglen + 1));
			strcpy(g_options.shell, optarg);
			break;
		case 'r':
			arglen = strlen(optarg);
			if (arglen >= PATH_MAX) {
				return -1;
			}

			g_options.server = malloc(sizeof(char) * (arglen + 1));
			strcpy(g_options.server, optarg);
			break;
		case 'h': /* fall through */
		default:
			help();
			return 0;
		}
	}
	if (optind < argc) {
		help();
		return 0;
	}

	loglevel = getenv("LOGLEVEL");
	if (loglevel) {
		SPIDER_LOGLEVEL = atoi(loglevel);
		spider_dbg("LOGLEVEL is set to %s\n", loglevel);
	}

	loglevel = getenv("LOGLEVEL_MAIN");
	if (loglevel) {
		SPIDER_LOGLEVEL = atoi(loglevel);
		spider_dbg("LOGLEVEL(main) is set to %s\n", loglevel);
	}

	preinit_compositor();
	init_compositor();
}
