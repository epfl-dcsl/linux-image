#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>

#define TYCHE_ENCLAVE_VALUE _IO('a', 'a')

int main(void)
{
  int fd;
  printf("Testing tyche enclave ioctl driver\n");
  printf("Opening driver.\n");
  fd = open("/dev/tyche_enclave", O_RDWR);
  if (fd < 0)
  {
    printf("Cannot open device file...\n");
    return 0;
  }
  printf("Invoke ioctl!\n");
  ioctl(fd, TYCHE_ENCLAVE_VALUE);
  printf("All done!\n");
  close(fd);
  return 0;
}
