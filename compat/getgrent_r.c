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
 *  Copyright © 2015 Kevin Bowling <k@kev009.com>
 */

// This compat layer is only built for BSD, or Linux without the GNU C
// Library.
#if defined(BSD) || (defined(__linux__) && !defined(__GLIBC__))

#include <grp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define ALIGNBYTES (sizeof(uintptr_t) - 1)
#define ALIGN(p)((((uintptr_t)(p) + ALIGNBYTES) & ~ALIGNBYTES))
static unsigned atou(char **s)
{
	unsigned x;
	for (x=0; **s-'0'<10U; ++*s) x=10*x+(**s-'0');
	return x;
}

int fgetgrent_r(FILE *f, struct group *gr, char *line, size_t size, struct group **res)
{
	char *s, *mems;
	size_t i, nmem, need;
	int rv = 0;
	int ep;
	ptrdiff_t remain;
	for (;;) {
		line[size-1] = '\xff';
		if ( (fgets(line, size, f) == NULL) || ferror(f) || (line[size-1] != '\xff') ) {
			rv = (line[size-1] != '\xff') ? ERANGE : ENOENT;
			line = 0;
			gr = 0;
			goto end;
		}
		ep = strcspn(line, "\n");
		line[ep] = 0;

		s = line;
		gr->gr_name = s++;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; gr->gr_passwd = s;
		if (!(s = strchr(s, ':'))) continue;

		*s++ = 0; gr->gr_gid = atou(&s);
		if (*s != ':') continue;

		*s++ = 0; mems = s;
		break;
	}

	for (nmem=!!*s; *s; s++)
		if (*s==',') ++nmem;

	++ep;
	remain = (void *) &line[size] - (void *) &line[ep];
	need = (sizeof(char *) * (nmem+1)) + ALIGNBYTES;
	if (need > remain) {
		rv = ERANGE;
		line = 0;
		gr = 0;
		goto end;
	}
	memset(&line[ep], 0, need);
	gr->gr_mem = (char **) ALIGN(&line[ep]);

	if (*mems) {
		gr->gr_mem[0] = mems;
		for (s=mems, i=0; *s; s++)
			if (*s==',') *s++ = 0, gr->gr_mem[++i] = s;
		gr->gr_mem[++i] = 0;
	} else {
		gr->gr_mem[0] = 0;
	}
end:
	*res = gr;
	if(rv) errno = rv;
	return rv;
}
#endif //#if defined(BSD) || defined(__linux__) && !defined(__GLIBC__)
