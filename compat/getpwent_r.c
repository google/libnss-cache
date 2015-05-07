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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

static unsigned atou(char **s)
{
	unsigned x;
	for (x=0; **s-'0'<10U; ++*s) x=10*x+(**s-'0');
	return x;
}

int __fgetpwent_a(FILE *f, struct passwd *pw, char **line, size_t *size, struct passwd **res)
{
	/* LIBNSS_CACHE_DEL ssize_t l; */
	char *s;
	int rv = 0;
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	for (;;) {
		line[0][(*size)-1] = '\xff';
		/* LIBNSS_CACHE_DEL if ((l=getline(line, size, f)) < 0) { */
		if ( (fgets(*line, *size, f) == NULL) || ferror(f) || (line[0][(*size)-1] != '\xff') ) {
			/* LIBNSS_CACHE_DIFF rv = ferror(f) ? errno : 0; */
			rv = (line[0][(*size)-1] != '\xff') ? ERANGE : ENOENT;
			/* LIBNSS_CACHE_DEL free(*line); */
			*line = 0;
			pw = 0;
			break;
		}
		/* LIBNSS_CACHE_DEL line[0][l-1] = 0; */
		line[0][strcspn(line[0], "\n")] = 0;

		s = line[0];
		pw->pw_name = s++;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_passwd = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_uid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; pw->pw_gid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; pw->pw_gecos = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_dir = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; pw->pw_shell = s;
		break;
	}
	pthread_setcancelstate(cs, 0);
	*res = pw;
	if (rv) errno = rv;
	return rv;
}

int fgetpwent_r(FILE *f, struct passwd *pw, char *line, size_t size, struct passwd **res)
{
	return __fgetpwent_a(f, pw, &line, &size, res);
}

#endif // ifdef __FreeBSD__
