#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include<sys/ioctl.h>
#include <sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <unistd.h>

// Extrernal libs.
#include <encl_loader.h>
#include <tyche_enclave.h>

const char* encl_so = "libs/encl.so";
const char* trusted = "enclave";

int main(void) {
  printf("Let's create an enclave!\n");
  const lib_encl_t* library = init_enclave_loader(encl_so);

  // mmap a shared region.
  void* shared = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);
  if (shared == MAP_FAILED) {
    fprintf(stderr, "Error mapping shared memory region.\n");
    exit(1);
  }
  printf("The shared region is %llx\n", shared);

  struct tyche_encl_add_region_t extra = {
    .start = (uint64_t) shared,
    .end = ((uint64_t)shared)+0x1000,
    .src = (uint64_t) shared,
    .flags = TE_READ|TE_WRITE,
    .tpe = Shared,
    .extra = NULL,
  };
  load_encl_t enclave;
  if (load_enclave(trusted, &enclave, &extra) != 0) {
    fprintf(stderr, "Unable to load the enclave.\n");
    exit(1);
  } 
  enclave_driver_transition(enclave.handle, shared);
  printf("Message from the enclave %s\n", (char*)shared);
  return 0;
}
