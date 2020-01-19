/*
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

#include <string.h>
#include "common/log.h"

int get_nargs(const char *command)
{
	char *ptr = command;
	int nargs = 1;

	while (*ptr != '\0') {
		if (*ptr == ' ')
			nargs++;

		ptr++;
	}

	return nargs;
}

void command_launch(char *command)
{
	char **args;
	char *delim = " ";
	int i = 0;
	spider_dbg("launch command: %s\n", command);

	args = calloc(1, sizeof(char*) * (get_nargs(command) + 1));
	if (!args) {
		spider_err("Failed to alloc\n");
		return;
	}

	char *ptr = strtok(command, delim);
	while(ptr != NULL)
	{
		args[i++] = ptr;
		ptr = strtok(NULL, delim);
	}	

	args[i] = NULL;
	if (fork() == 0) {
		execvp(command, args);
	}

	if (args) {
		free(args);
	}
}
