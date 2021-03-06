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

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "common/global_vars.h"

#define __CLOCK__ ({	\
	char retval[20];						\
	time_t rawtime;							\
	struct tm* timeinfo;						\
	time(&rawtime);							\
	timeinfo = localtime(&rawtime);					\
	strftime (retval, sizeof(retval), "%H:%M:%S", timeinfo);	\
	retval;								\
	})								\

#define __PATH__ (strchr(__FILE__, '/') ? strchr(__FILE__, '/') + 1 : __FILE__)

#define spider_verbose(fmt, ...) 					\
	do {								\
		if (SPIDER_LOGLEVEL < 4) break;				\
		fprintf(stdout, "\033[%s][2;32m[%s (%s)] " fmt, 	\
				__CLOCK__, __func__, 			\
				__PATH__, ## __VA_ARGS__);		\
		fprintf(stderr, "\033[0m");				\
	}while(0)

#define spider_dbg(fmt, ...) 						\
	do {								\
		if (SPIDER_LOGLEVEL < 3) break;				\
		fprintf(stdout, "\033[0;32m[%s][%s (%s)] " fmt,		\
				__CLOCK__, __func__, 			\
				__PATH__, ## __VA_ARGS__);		\
		fprintf(stderr, "\033[0m");				\
	}while(0)

#define spider_log(fmt, ...) 						\
	do {								\
		if (SPIDER_LOGLEVEL < 2) break;				\
		fprintf(stdout, "\033[1;37m[%s][%s (%s)] " fmt,		\
				__CLOCK__, __func__, 			\
				__PATH__, ## __VA_ARGS__);		\
		fprintf(stderr, "\033[0m");				\
	}while(0)

#define spider_err(fmt, ...) 						\
	do {								\
		if (SPIDER_LOGLEVEL < 1) break;				\
		fprintf(stderr, "\033[1;31m[%s][%s (%s)] " fmt,		\
				__CLOCK__, __func__, 			\
				__PATH__, ## __VA_ARGS__);		\
		fprintf(stderr, "\033[0m");				\
	}while(0)

#endif /* __LOG_H__ */
