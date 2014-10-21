#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>

int main(int argc, char **argv) {
  setpwent();
  for (;;) {
    errno = 0;
    struct passwd *pw = getpwent();
    if (!pw) {
      if (errno != 0)
        printf("ERRNO: %u %m\n", errno);
      break;
    }
  }
  endpwent();
  return 0;
}
