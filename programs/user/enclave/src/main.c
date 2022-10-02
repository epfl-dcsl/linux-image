#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
#include <sys/mman.h>

//TODO put the library somewhere else, modify makefile to include it in the path.
#include "tyche_enclave.h"

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
  ioctl(fd, TYCHE_ENCLAVE_DBG);

  printf("Create an enclave\n");
  struct tyche_encl_create_t enclave;
  ioctl(fd, TYCHE_ENCLAVE_CREATE, &enclave);
  printf("Received handle %ld\n", enclave.handle);
  ioctl(fd, TYCHE_ENCLAVE_CREATE, &enclave);
  printf("Second handle %ld\n", enclave.handle);

  printf("Add a page \n");
  struct tyche_encl_add_region_t region = {
    .handle = enclave.handle,
    .start = 0x1000,
    .end = 0x2000,
    .flags = TE_READ | TE_USER | TE_WRITE,
    .tpe = Confidential,
  };
  if (ioctl(fd, TYCHE_ENCLAVE_ADD_REGION, &region) == -1) {
    printf("Error mapping a random page.\n"); 
  }

  // Try mapping a valid page.
  int* ptr = (int*) mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0); 
  if (ptr == MAP_FAILED) {
    printf("Unabel to mmap!\n");
    goto done;
  }
  // Make sure we can write/populate the region.
  *ptr = 10;
  region.start = ((uint64_t) (&region)) - ((uint64_t)(&region) % 0x1000);
  region.end = region.start + 0x1000;
  if (ioctl(fd, TYCHE_ENCLAVE_ADD_REGION, &region) == -1) {
    printf("Second call to add page failed, it should not have.\n");
    goto done;
  }
  printf("Success mapping a region\n");

done:
  printf("All done!\n");
  close(fd);
  return 0;
}
