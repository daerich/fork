/*
* ISC License
* 
* Copyright (c) 2022 Erich Ericson
* 
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
* 
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
* OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void err(const char * msg)
{
	fprintf(stderr, "Error during %s!\nTerminating..\n", msg);
	exit(1);
}

static size_t argscnt(char ** args)
{
	size_t res = 0;
	for(;*args != NULL; res++, args++);
	return res;
}

static void usage(void)
{
	printf("fork [-qd] PRGRAM [ARGS...]\n");
	exit(1);
}

static void * xmalloc(size_t size)
{
	void * res = NULL;
	if ((res = malloc(size)) == NULL &&  size != 0)
		err("malloc");
	return res;
}

int main(int argc,char ** argv)
{
	pid_t pid = 0;
	int ch = 0;
	int quiet = 0;
	int detach = 0;
	const char * argopt = NULL;
	char ** args = NULL;
	int len = 0;
#ifdef __OpenBSD__
	if (pledge("stdio exec proc", NULL) == -1)
		err("pledge");
#endif
	if (argc < 2)
		usage();
	while ((ch = getopt(argc, argv, "qd")) != -1) {
		switch(ch) {
		case 'q':
			quiet = 1;
			break;
		case 'd':
			detach = 1;
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		usage();
	argopt = *argv;
	len = argscnt(argv) + 1; /* + NULL-PTR */
	args = xmalloc(len * sizeof(char *));
	memset(args, 0, len);
	for (int i = 0; i < len; i++) {
		args[i] = *argv;
		argv++;
	}
	if ((pid = fork()) == -1)
		err("fork");
	if (pid != 0)
		_exit(0);
	/* child */
	if (quiet) {
		fclose(stdout);
		fclose(stderr);
	}
#ifdef __OpenBSD__
	if (pledge("exec proc", NULL) == -1)
		err("pledge");
#endif
	if (detach && setsid() == -1)
		err("setsid");
	if (execvp(argopt, args) == -1)
		err("exec");
	return 1;
}

