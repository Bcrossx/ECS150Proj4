#include "fs.h"

int main()
{
  fs_mount("disk3");

  fs_create("file1\0");
  fs_create("file2\0");

  int fd1 = fs_open("file1");
  int fd2 = fs_open("file2");

  //char *buf1 = "123456789999999";
  //char *buf2 = "12345678a";

  //fs_write(fd1, buf1, 10);
  //fs_write(fd2, buf2, 10);

  fs_close(fd1);
  fs_close(fd2);

  return 0;
}
