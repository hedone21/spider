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

#ifndef __GLOBAL_VARS_H__
#define __GLOBAL_VARS_H__

#define SPIDER_WEB_URL 			"SPIDER_WEB_URL"
#define SPIDER_WEB_URL_PATH 		"localhost:8080"
#define SPIDER_PANEL_URL 		"SPIDER_PANEL_URL"
#define SPIDER_CLIENT_SERVER_PATH 	"SPIDER_CLIENT_SERVER_PATH"

/** 
 * 0: No dbg
 * 1: Error log only
 * 2: Error log, normal log (default)
 * 3: Error log, normal log, dbg log
 * 4: Error log, normal log, dbg log, verbose log
 *
 * Defined in every main.c files
 *
 * usage:
 * 	$ LOGLEVEL=4 ./run.sh
 * 	$ LOGLEVEL_MAIN=3 ./run.sh
 * 	$ LOGLEVEL_SHELL=2 ./run.sh
 * 	$ LOGLEVEL_PANEL=1 ./run.sh
 */
extern int SPIDER_LOGLEVEL;

#endif /* __GLOBAL_VARS_H__ */
