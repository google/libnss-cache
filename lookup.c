/* Copyright 2009 Google Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA
 */

/*
 * Perform get*id() and get*nam() lookups given simple lists of
 * ids or names.  Used to test correctness and timings of functions in
 * nss_cache.c
 *
 * There's a fair amount of repeated functions, and I thought briefly about
 * trying to consolidate them with callbacks or put an if clause inside the
 * while look (making one longer one for lookup_foo() functions).
 *
 * But, each function is slightly different:  string or uid?  struct group
 * or struct passwd or struct spwd?  And I decided stupid repetition was just
 * easier and very direct to read.  ymmv.
 *
 */

#include "nss_cache.h"
#include "nss_test.h"

// usage()
//
// print out flags and arguments

static void usage(char *program) {

  fprintf(stderr, "Usage: %s -c nss_function -f filename\n", program);
}

// getpwnam_wrapper()
//
// perform a getpwnam() lookup via nss_cache.c directly

static int getpwnam_wrapper(char *name) {

  struct passwd result;
  char *buffer = NULL;
  size_t buflen = 1024;
  int found = 255;
  int errnop;
  int ret;

  _nss_cache_setpwent_path(PASSWD_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getpwnam_r(name, &result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      // printf("found %s, %d\n", result.pw_name, result.pw_uid);
      found = 0;
    }
    if (ret == NSS_STATUS_NOTFOUND) {
      found = 1;
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access passwd test data\n");
      return 2;
    }
  } while (ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return found;
}

// getpwuid_wrapper()
//
// perform a getpwui() lookup via nss_cache.c directly

static int getpwuid_wrapper(uid_t uid) {

  struct passwd result;
  char *buffer = NULL;
  size_t buflen = 1024;
  int found = 255;
  int errnop;
  int ret;

  _nss_cache_setpwent_path(PASSWD_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getpwuid_r(uid, &result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      // printf("found %s, %d\n", result.pw_name, result.pw_uid);
      found = 0;
    }
    if (ret == NSS_STATUS_NOTFOUND) {
      found = 1;
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access passwd test data\n");
      return 2;
    }
  } while (ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return found;
}

// getgrnam_wrapper()
//
// perform a getgrnam() lookup via nss_cache.c directly

static int getgrnam_wrapper(char *name) {

  struct group result;
  char *buffer = NULL;
  size_t buflen = 1024;
  int found = 255;
  int errnop;
  int ret;

  _nss_cache_setgrent_path(GROUP_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getgrnam_r(name, &result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      // printf("found %s, %d\n", result.gr_name, result.gr_gid);
      found = 0;
    }
    if (ret == NSS_STATUS_NOTFOUND) {
      found = 1;
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access passwd test data\n");
      return 2;
    }
  } while (ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return found;
}

// getgrgid_wrapper()
//
// perform a getgrgid() lookup via nss_cache.c directly

static int getgrgid_wrapper(gid_t gid) {

  struct group result;
  char *buffer = NULL;
  size_t buflen = 1024;
  int found = 255;
  int errnop;
  int ret;

  _nss_cache_setgrent_path(GROUP_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getgrgid_r(gid, &result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      // printf("found %s, %d\n", result.gr_name, result.gr_gid);
      found = 0;
    }
    if (ret == NSS_STATUS_NOTFOUND) {
      found = 1;
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access passwd test data\n");
      return 2;
    }
  } while (ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return found;
}

#ifndef BSD
// getspnam_wrapper()
//
// perform a getspnam() lookup via nss_cache.c directly

static int getspnam_wrapper(char *name) {

  struct spwd result;
  char *buffer = NULL;
  size_t buflen = 1024;
  int found = 255;
  int errnop;
  int ret;

  _nss_cache_setspent_path(SHADOW_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getspnam_r(name, &result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      // printf("found %s, %s\n", result.sp_namp, result.sp_pwdp);
      found = 0;
    }
    if (ret == NSS_STATUS_NOTFOUND) {
      found = 1;
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access passwd test data\n");
      return 2;
    }
  } while (ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return found;
}
#endif // ifndef BSD

// lookup_getpwnam()
//
// call getpwnam() from nss_cache.c on each line of a file

static int lookup_getpwnam(FILE *input) {

  int lines = 0;
  char line[255];
  int ret;

  while (fgets(line, sizeof(line), input)) {
    lines += 1;
    // strip trailing newline
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    ret = getpwnam_wrapper(line);
    if (ret != 0) {
      break;
    }
  }

  if (ret == 0) {
    printf("successfully completed %d lookups\n", lines);
  } else {
    fprintf(stderr, "failed at line %d: %s\n", lines, line);
  }

  return ret;
}

// lookup_getpwuid()
//
// call getpwuid() from nss_cache.c on each line of a file

static int lookup_getpwuid(FILE *input) {

  int lines = 0;
  char line[255];
  int ret;

  while (fgets(line, sizeof(line), input)) {
    lines += 1;
    // strip trailing newline
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    ret = getpwuid_wrapper(atoi(line));
    if (ret != 0) {
      break;
    }
  }

  if (ret == 0) {
    printf("successfully completed %d lookups\n", lines);
  } else {
    fprintf(stderr, "failed at line %d: %s\n", lines, line);
  }

  return ret;
}

// lookup_getgrnam()
//
// call getgrnam() from nss_cache.c on each line of a file

static int lookup_getgrnam(FILE *input) {

  int lines = 0;
  char line[255];
  int ret;

  while (fgets(line, sizeof(line), input)) {
    lines += 1;
    // strip trailing newline
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    ret = getgrnam_wrapper(line);
    if (ret != 0) {
      break;
    }
  }

  if (ret == 0) {
    printf("successfully completed %d lookups\n", lines);
  } else {
    fprintf(stderr, "failed at line %d: %s\n", lines, line);
  }

  return ret;
}

// lookup_getgrgid()
//
// call getgrgid() from nss_cache.c on each line of a file

static int lookup_getgrgid(FILE *input) {

  int lines = 0;
  char line[255];
  int ret;

  while (fgets(line, sizeof(line), input)) {
    lines += 1;
    // strip trailing newline
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    ret = getgrgid_wrapper(atoi(line));
    if (ret != 0) {
      break;
    }
  }

  if (ret == 0) {
    printf("successfully completed %d lookups\n", lines);
  } else {
    fprintf(stderr, "failed at line %d: %s\n", lines, line);
  }

  return ret;
}

#ifndef BSD
// lookup_getspnam()
//
// call getspnam() from nss_cache.c on each line of a file

static int lookup_getspnam(FILE *input) {

  int lines = 0;
  char line[255];
  int ret;

  while (fgets(line, sizeof(line), input)) {
    lines += 1;
    // strip trailing newline
    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }
    ret = getspnam_wrapper(line);
    if (ret != 0) {
      break;
    }
  }

  if (ret == 0) {
    printf("successfully completed %d lookups\n", lines);
  } else {
    fprintf(stderr, "failed at line %d: %s\n", lines, line);
  }

  return ret;
}
#endif // ifndef BSD

// nss_lookup()
//
// call the nss lookup wrapper that was set by getopt()

static int nss_lookup(char *call, FILE *input) {

  int ret;

  if (strncmp(call, "getpwnam", 8) == 0) {
    ret = lookup_getpwnam(input);
  } else if (strncmp(call, "getpwuid", 8) == 0) {
    ret = lookup_getpwuid(input);
  } else if (strncmp(call, "getgrnam", 8) == 0) {
    ret = lookup_getgrnam(input);
  } else if (strncmp(call, "getgrgid", 8) == 0) {
    ret = lookup_getgrgid(input);
#ifndef BSD
  } else if (strncmp(call, "getspnam", 8) == 0) {
    ret = lookup_getspnam(input);
#endif // ifndef BSD
  } else {
    fprintf(stderr, "unknown nss function: %s\n", call);
    ret = 1;
  }

  return ret;
}

// main()
//
// parse commandline options to get the function name and input file.
// send it through nss_lookup() which will ultimately call the
// function on each line in the file and track success.

int main(int argc, char **argv) {

  char *call = NULL;
  char *filename = NULL;
  FILE *input = NULL;
  int ret;
  int c;

  while ((c = getopt(argc, argv, "c:f:")) != -1) {
    switch (c) {
    case 'c':
      call = optarg;
      break;
    case 'f':
      filename = optarg;
      break;
    }
  }

  if (call == NULL || filename == NULL) {
    usage(argv[0]);
    return 1;
  }

  input = fopen(filename, "r");

  if (input == NULL) {
    fprintf(stderr, "failed to open %s\n", filename);
    return 255;
  }

  ret = nss_lookup(call, input);
  fclose(input);

  return ret;
}
