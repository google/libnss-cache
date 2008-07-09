/* Copyright 2007 Google Inc.
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

/* An NSS module which adds supports for file maps with a trailing .cache
 * suffix (/etc/passwd.cache, /etc/group.cache, and /etc/shadow.cache)
 */

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <sys/types.h>
#include <nss.h>
#include <errno.h>
#include <string.h>

// Locking implementation: use pthreads.
#include <pthread.h>
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define NSS_CACHE_LOCK()    do { pthread_mutex_lock(&mutex); } while (0)
#define NSS_CACHE_UNLOCK()  do { pthread_mutex_unlock(&mutex); } while (0)

static FILE *p_file, *g_file, *s_file;

#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt, args...)  do { fprintf(stderr, fmt, ##args); } while (0)
#else
#define DEBUG(fmt, ...)      do { } while (0)
#endif

/* Common return code routine for all *ent_r_locked functions.
 * We need to return TRYAGAIN if the underlying files guy raises ERANGE,
 * so that our caller knows to try again with a bigger buffer.
 */

static inline enum nss_status _nss_cache_ent_bad_return_code(int errnoval) {
  enum nss_status ret;

  switch (errnoval) {
    case ERANGE:
      DEBUG("ERANGE: Try again with a bigger buffer\n");
      ret = NSS_STATUS_TRYAGAIN;
      break;
    case ENOENT:
    default:
      DEBUG("ENOENT or default case: Not found\n");
      ret = NSS_STATUS_NOTFOUND;
  };
  return ret;
}

//
// Routines for passwd map defined below here
//

// _nss_cache_setpwent_locked()
// Internal setup routine

static enum nss_status _nss_cache_setpwent_locked(void) {

  DEBUG("Opening passwd.cache\n");
  p_file = fopen("/etc/passwd.cache", "r");

  if (p_file) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_cache_setpwent()
// Called by NSS to open the passwd file
// 'stayopen' parameter is ignored.

enum nss_status _nss_cache_setpwent(int stayopen) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_setpwent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_endpwent_locked()
// Internal close routine

static enum nss_status _nss_cache_endpwent_locked(void) {

  DEBUG("Closing passwd.cache\n");
  if (p_file) {
    fclose(p_file);
    p_file = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_cache_endpwent()
// Called by NSS to close the passwd file

enum nss_status _nss_cache_endpwent(void) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_endpwent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getpwent_r_locked()
// Called internally to return the next entry from the passwd file

static enum nss_status _nss_cache_getpwent_r_locked(struct passwd *result,
                                                   char *buffer, size_t buflen,
                                                   int *errnop) {
  enum nss_status ret = NSS_STATUS_SUCCESS;

  if (p_file == NULL) {
    DEBUG("p_file == NULL, going to setpwent\n");
    ret = _nss_cache_setpwent_locked();
  }

  if (ret == NSS_STATUS_SUCCESS) {
    if (fgetpwent_r(p_file, result, buffer, buflen, &result) == 0) {
      DEBUG("Returning user %d:%s\n", result->pw_uid, result->pw_name);
    } else {
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getpwent_r()
// Called by NSS to look up next entry in passwd file

enum nss_status _nss_cache_getpwent_r(struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_getpwent_r_locked(result, buffer, buflen, errnop);
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getpwuid_r()
// Find a user account by uid

enum nss_status _nss_cache_getpwuid_r(uid_t uid, struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSS_CACHE_LOCK();
  ret = _nss_cache_setpwent_locked();
  DEBUG("Looking for uid %d\n", uid);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_cache_getpwent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (result->pw_uid == uid)
        break;
    }
  }

  _nss_cache_endpwent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

// _nss_cache_getpwnam_r()
// Find a user account by name

enum nss_status _nss_cache_getpwnam_r(const char *name, struct passwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSS_CACHE_LOCK();
  ret = _nss_cache_setpwent_locked();
  DEBUG("Looking for user %s\n", name);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_cache_getpwent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (!strcmp(result->pw_name, name))
        break;
    }
  }

  _nss_cache_endpwent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

//
//  Routines for group map defined here.
//

// _nss_cache_setgrent_locked()
// Internal setup routine

static enum nss_status _nss_cache_setgrent_locked(void) {

  DEBUG("Opening group.cache\n");
  g_file = fopen("/etc/group.cache", "r");

  if (g_file) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_cache_setgrent()
// Called by NSS to open the group file
// 'stayopen' parameter is ignored.

enum nss_status _nss_cache_setgrent(int stayopen) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_setgrent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_endgrent_locked()
// Internal close routine

static enum nss_status _nss_cache_endgrent_locked(void) {

  DEBUG("Closing group.cache\n");
  if (g_file) {
    fclose(g_file);
    g_file = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_cache_endgrent()
// Called by NSS to close the group file

enum nss_status _nss_cache_endgrent(void) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_endgrent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getgrent_r_locked()
// Called internally to return the next entry from the group file

static enum nss_status _nss_cache_getgrent_r_locked(struct group *result,
                                                   char *buffer, size_t buflen,
                                                   int *errnop) {
  enum nss_status ret = NSS_STATUS_SUCCESS;

  if (g_file == NULL) {
    DEBUG("g_file == NULL, going to setgrent\n");
    ret = _nss_cache_setgrent_locked();
  }

  if (ret == NSS_STATUS_SUCCESS) {
    fpos_t position;

    fgetpos(g_file, &position);
    if (fgetgrent_r(g_file, result, buffer, buflen, &result) == 0) {
      DEBUG("Returning group %s (%d)\n", result->gr_name, result->gr_gid);
    } else {
      /* Rewind back to where we were just before, otherwise the data read
       * into the buffer is probably going to be lost because there's no
       * guarantee that the caller is going to have preserved the line we
       * just read.  Note that glibc's nss/nss_files/files-XXX.c does
       * something similar in CONCAT(_nss_files_get,ENTNAME_r) (around
       * line 242 in glibc 2.4 sources).
       */
      fsetpos(g_file, &position);
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getgrent_r()
// Called by NSS to look up next entry in group file

enum nss_status _nss_cache_getgrent_r(struct group *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_getgrent_r_locked(result, buffer, buflen, errnop);
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getgrgid_r()
// Find a group by gid

enum nss_status _nss_cache_getgrgid_r(gid_t gid, struct group *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSS_CACHE_LOCK();
  ret = _nss_cache_setgrent_locked();
  DEBUG("Looking for gid %d\n", gid);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_cache_getgrent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (result->gr_gid == gid)
        break;
    }
  }

  _nss_cache_endgrent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

// _nss_cache_getgrnam_r()
// Find a group by name

enum nss_status _nss_cache_getgrnam_r(const char *name, struct group *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSS_CACHE_LOCK();
  ret = _nss_cache_setgrent_locked();
  DEBUG("Looking for group %s\n", name);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_cache_getgrent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (!strcmp(result->gr_name, name))
        break;
    }
  }

  _nss_cache_endgrent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}

//
//  Routines for shadow map defined here.
//

// _nss_cache_setspent_locked()
// Internal setup routine

static enum nss_status _nss_cache_setspent_locked(void) {

  DEBUG("Opening shadow.cache\n");
  s_file = fopen("/etc/shadow.cache", "r");

  if (s_file) {
    return NSS_STATUS_SUCCESS;
  } else {
    return NSS_STATUS_UNAVAIL;
  }
}

// _nss_cache_setspent()
// Called by NSS to open the shadow file
// 'stayopen' parameter is ignored.

enum nss_status _nss_cache_setspent(int stayopen) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_setspent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_endspent_locked()
// Internal close routine

static enum nss_status _nss_cache_endspent_locked(void) {

  DEBUG("Closing shadow.cache\n");
  if (s_file) {
    fclose(s_file);
    s_file = NULL;
  }
  return NSS_STATUS_SUCCESS;
}

// _nss_cache_endspent()
// Called by NSS to close the shadow file

enum nss_status _nss_cache_endspent(void) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_endspent_locked();
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getspent_r_locked()
// Called internally to return the next entry from the shadow file

static enum nss_status _nss_cache_getspent_r_locked(struct spwd *result,
                                                   char *buffer, size_t buflen,
                                                   int *errnop) {

  enum nss_status ret = NSS_STATUS_SUCCESS;

  if (s_file == NULL) {
    DEBUG("s_file == NULL, going to setspent\n");
    ret = _nss_cache_setspent_locked();
  }

  if (ret == NSS_STATUS_SUCCESS) {
    if (fgetspent_r(s_file, result, buffer, buflen, &result) == 0) {
      DEBUG("Returning shadow entry %s\n", result->sp_namp);
    } else {
      *errnop = errno;
      ret = _nss_cache_ent_bad_return_code(*errnop);
    }
  }

  return ret;
}

// _nss_cache_getspent_r()
// Called by NSS to look up next entry in the shadow file

enum nss_status _nss_cache_getspent_r(struct spwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {
  enum nss_status ret;
  NSS_CACHE_LOCK();
  ret = _nss_cache_getspent_r_locked(result, buffer, buflen, errnop);
  NSS_CACHE_UNLOCK();
  return ret;
}

// _nss_cache_getspnam_r()
// Find a user by name

enum nss_status _nss_cache_getspnam_r(const char *name, struct spwd *result,
                                     char *buffer, size_t buflen,
                                     int *errnop) {

  enum nss_status ret;

  NSS_CACHE_LOCK();
  ret = _nss_cache_setspent_locked();
  DEBUG("Looking for shadow entry %s\n", name);

  if (ret == NSS_STATUS_SUCCESS) {
    while ((ret = _nss_cache_getspent_r_locked(result, buffer, buflen, errnop))
           == NSS_STATUS_SUCCESS) {
      if (!strcmp(result->sp_namp, name))
        break;
    }
  }

  _nss_cache_endspent_locked();
  NSS_CACHE_UNLOCK();

  return ret;
}
