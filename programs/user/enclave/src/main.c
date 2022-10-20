#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

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
  struct tyche_encl_add_region_t extra = {
    .start = (uint64_t) shared,
    .end = ((uint64_t)shared)+0x1000,
    .src = (uint64_t) shared,
    .flags = TE_READ|TE_USER|TE_WRITE,
    .tpe = Shared,
    .extra = NULL,
  };
  tyche_encl_handle_t handle;
  if (load_enclave(trusted, &handle, &extra) != 0) {
    fprintf(stderr, "Unable to load the enclave.\n");
    exit(1);
  } 
  printf("We have a handle! %llx\n", handle);
  return 0;
}
