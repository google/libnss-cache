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

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>

#ifndef NSS_TEST_H
#define NSS_TEST_H

const char *PASSWD_FILE = ".testdata/passwd.cache";
const char *GROUP_FILE = ".testdata/group.cache";
const char *SHADOW_FILE = ".testdata/shadow.cache";

extern enum nss_status _nss_cache_getpwent_r(struct passwd *result,
                                             char *buffer, size_t buflen, int *errnop);
extern enum nss_status _nss_cache_getpwnam_r(const char *name,
                                             struct passwd *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern enum nss_status _nss_cache_getpwuid_r(uid_t uid,
                                             struct passwd *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern enum nss_status _nss_cache_getgrent_r(struct group *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern enum nss_status _nss_cache_getgrnam_r(const char *name,
                                             struct group *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern enum nss_status _nss_cache_getgrgid_r(gid_t gid, struct group *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern enum nss_status _nss_cache_getspnam_r(const char *name,
                                             struct spwd *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern enum nss_status _nss_cache_getspent_r(struct spwd *result,
                                             char *buffer, size_t buflen,
                                             int *errnop);
extern char* _nss_cache_setpwent_path(const char* path);

#endif /* NSS_TEST_H */
