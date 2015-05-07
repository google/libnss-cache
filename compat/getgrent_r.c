/*
 * ----------------------------------------------------------------------
 *  Copyright © 2005-2014 Rich Felker, et al.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  ----------------------------------------------------------------------
 *
 *  Adapted from http://www.musl-libc.org/ for libnss-cache
 */

#include <sys/param.h>

#ifdef __FreeBSD__

#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

static char **mem;

static unsigned atou(char **s)
{
	unsigned x;
	for (x=0; **s-'0'<10U; ++*s) x=10*x+(**s-'0');
	return x;
}

int __getgrent_a(FILE *f, struct group *gr, char **line, size_t *size, char ***mem, size_t *nmem, struct group **res)
{
	/* LIBNSS_CACHE_DEL ssize_t l; */
	char *s, *mems;
	size_t i;
	int rv = 0;
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	for (;;) {
		line[0][(*size)-1] = '\xff';
		/* LIBNSS_CACHE_DIFF if ((l=getline(line, size, f)) < 0) { */
		if ( (fgets(*line, *size, f) == NULL) || ferror(f) || (line[0][(*size)-1] != '\xff') ) {
			/* LIBNSS_CACHE_DIFF rv = ferror(f) ? errno : 0; */
			rv = (line[0][(*size)-1] != '\xff') ? ERANGE : ENOENT;
			/* LIBNSS_CACHE_DEL free(*line); */
			*line = 0;
			gr = 0;
			goto end;
		}
		/* LIBNSS_CACHE_DEL line[0][l-1] = 0; */
		line[0][strcspn(line[0], "\n")] = 0;

		s = line[0];
		gr->gr_name = s++;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; gr->gr_passwd = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; gr->gr_gid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; mems = s;
		break;
	}

	for (*nmem=!!*s; *s; s++)
		if (*s==',') ++*nmem;
	free(*mem);
	*mem = calloc(sizeof(char *), *nmem+1);
	if (!*mem) {
		rv = errno;
		/* LIBNSS_CACHE_DEL free(*line); */
		*line = 0;
		/* LIBNSS_CACHE_DIFF return 0; */
		return rv;
	}
	if (*mems) {
		mem[0][0] = mems;
		for (s=mems, i=0; *s; s++)
			if (*s==',') *s++ = 0, mem[0][++i] = s;
		mem[0][++i] = 0;
	} else {
		mem[0][0] = 0;
	}
	gr->gr_mem = *mem;
end:
	pthread_setcancelstate(cs, 0);
	*res = gr;
	if(rv) errno = rv;
	return rv;
}

int fgetgrent_r(FILE *f, struct group *gr, char *line, size_t size, struct group **res)
{
	size_t nmem=0;
	return __getgrent_a(f, gr, &line, &size, &mem, &nmem, res);
}

#endif // ifdef __FreeBSD__
