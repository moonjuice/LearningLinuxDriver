#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int num = 0;
  float scale = 0.002286372761;
  float offset = 375.7788421;
  int fd = open("/dev/HX711", O_RDWR);

  if (fd == -1) {
    printf("open device failed, errno : %s(%d) \n", strerror(errno), errno);
    return 1;
  }
  int i = 0;
  for (i = 0; i < 10; i++) {
    ssize_t ret = read(fd, &num, sizeof(int));
    if (ret != sizeof(int)) {
      printf("read fails, errno : %s(%d) \n", strerror(errno), errno);
      return 1;
    }
    printf("%f\n", (num * scale) - offset);
  }
  close(fd);

  return 0;
}