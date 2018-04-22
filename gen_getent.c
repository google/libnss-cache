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
 * A helper program to let us do simplistic unit testing of
 * libnss-cache functions.  It does this by generating copies of the
 * maps in a data directory, using nss_cache.c get*ent() functions.
 */

#include "nss_cache.h"
#include "nss_test.h"

// getpwent_to_file()
// Call the nss_cache getpwent function to dump the passwd store to a
// file.

static int getpwent_to_file(FILE *output) {
  struct passwd result;
  char *buffer;
  size_t buflen = 1024;
  int errnop;
  enum nss_status ret;

  _nss_cache_setpwent_path(PASSWD_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getpwent_r(&result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      fprintf(output, "%s:%s:%d:%d:%s:%s:%s\n", result.pw_name,
              result.pw_passwd, result.pw_uid, result.pw_gid, result.pw_gecos,
              result.pw_dir, result.pw_shell);
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access passwd test data\n");
      return 1;
    }
  } while (ret == NSS_STATUS_SUCCESS || ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return 0;
}

// getgrent_to_file()
// Call the nss_cache getgrent function to dump the group store to a
// file.

static int getgrent_to_file(FILE *output) {
  struct group result;
  char *buffer;
  size_t buflen = 1024;
  int errnop;
  enum nss_status ret;
  int idx;

  _nss_cache_setgrent_path(GROUP_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getgrent_r(&result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      fprintf(output, "%s:%s:%d:", result.gr_name, result.gr_passwd,
              result.gr_gid);
      // unroll **gr_mem
      for (idx = 0; result.gr_mem[idx] != NULL; idx++) {
        fprintf(output, "%s", result.gr_mem[idx]);
        if (result.gr_mem[idx + 1] != NULL) {
          fprintf(output, ",");
        }
      }
      fprintf(output, "\n");
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      fprintf(stderr, "ERROR: failed to access group test data\n");
      return 1;
    }
  } while (ret == NSS_STATUS_SUCCESS || ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return 0;
}

#ifndef BSD
// getspent_to_file()
// Call the nss_cache getspent function to dump the shadow store to a
// file.

static int getspent_to_file(FILE *output) {
  struct spwd result;
  char *buffer;
  size_t buflen = 1024;
  int errnop;
  enum nss_status ret;

  _nss_cache_setpwent_path(SHADOW_FILE);

  buffer = malloc(buflen);

  do {
    ret = _nss_cache_getspent_r(&result, buffer, buflen, &errnop);
    if (ret == NSS_STATUS_SUCCESS) {
      fprintf(output, "%s:%s:", result.sp_namp, result.sp_pwdp);
      // sigh, empty numberical fields are -1 in the struct,
      // so if necessary convert back to empty fields
      if (result.sp_lstchg != -1) {
        fprintf(output, "%ld:", result.sp_lstchg);
      } else {
        fprintf(output, ":");
      }
      if (result.sp_min != -1) {
        fprintf(output, "%ld:", result.sp_min);
      } else {
        fprintf(output, ":");
      }
      if (result.sp_max != -1) {
        fprintf(output, "%ld:", result.sp_max);
      } else {
        fprintf(output, ":");
      }
      if (result.sp_warn != -1) {
        fprintf(output, "%ld:", result.sp_warn);
      } else {
        fprintf(output, ":");
      }
      if (result.sp_inact != -1) {
        fprintf(output, "%ld:", result.sp_inact);
      } else {
        fprintf(output, ":");
      }
      if (result.sp_expire != -1) {
        fprintf(output, "%ld:", result.sp_expire);
      } else {
        fprintf(output, ":");
      }
      if (result.sp_flag != -1) {
        fprintf(output, "%ld", result.sp_flag);
      }
      fprintf(output, "\n");
    }
    if (ret == NSS_STATUS_TRYAGAIN) {
      buflen = buflen * 2;
      buffer = realloc(buffer, buflen);
    }
    if (ret == NSS_STATUS_UNAVAIL) {
      perror("ERROR: failed to access shadow test data");
      return 1;
    }
  } while (ret == NSS_STATUS_SUCCESS || ret == NSS_STATUS_TRYAGAIN);

  free(buffer);

  return 0;
}
#endif  // ifndef BSD

// gen_getpwent_data()
//
// creates a copy of the passwd map as read by nss_cache.c

static int gen_getpwent_data(void) {
  char filename[NSS_CACHE_PATH_LENGTH];
  FILE *output;
  int ret;

  strncpy(filename, PASSWD_FILE, NSS_CACHE_PATH_LENGTH - 4);
  strncat(filename, ".out", 4);
  output = fopen(filename, "w");

  if (output == NULL) {
    fprintf(stderr, "failed to open %s!\n", filename);
    return 255;
  }

  ret = getpwent_to_file(output);
  fclose(output);

  return ret;
}

// gen_getgrent_data()
//
// creates a copy of the group map as read by nss_cache.c

static int gen_getgrent_data(void) {
  char filename[NSS_CACHE_PATH_LENGTH];
  FILE *output;
  int ret;

  strncpy(filename, GROUP_FILE, NSS_CACHE_PATH_LENGTH - 4);
  strncat(filename, ".out", 4);
  output = fopen(filename, "w");

  if (output == NULL) {
    fprintf(stderr, "failed to open %s!\n", filename);
    return 255;
  }

  ret = getgrent_to_file(output);
  fclose(output);

  return ret;
}

#ifndef BSD
// gen_getspent_data()
//
// creates a copy of the shadow map as read by nss_cache.c

static int gen_getspent_data(void) {
  char filename[NSS_CACHE_PATH_LENGTH];
  FILE *output;
  int ret;

  strncpy(filename, SHADOW_FILE, NSS_CACHE_PATH_LENGTH - 4);
  strncat(filename, ".out", 4);
  output = fopen(filename, "w");

  if (output == NULL) {
    fprintf(stderr, "failed to open %s!\n", filename);
    return 255;
  }

  ret = getspent_to_file(output);
  fclose(output);

  return ret;
}
#endif  // ifndef BSD

// main()
//
// generate the passwd, group and shadow files using nss_cache.c functions
// which are further checked for correctness by 'make test'.

int main(void) {
  int ret;
  int failed_tests = 0;

  ret = gen_getpwent_data();
  if (ret != 0) {
    fprintf(stderr, "Failed to generate password file.\n");
    failed_tests = failed_tests + 1;
  }

  ret = gen_getgrent_data();
  if (ret != 0) {
    fprintf(stderr, "Failed to generate password file.\n");
    failed_tests = failed_tests + 1;
  }

#ifndef BSD
  ret = gen_getspent_data();
  if (ret != 0) {
    fprintf(stderr, "Failed to generate password file.\n");
    failed_tests = failed_tests + 1;
  }
#endif

  printf("generated all files.\n");

  return failed_tests;
}
