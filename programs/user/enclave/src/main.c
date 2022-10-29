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

void test_debugging(domain_id_t handle)
{
  //uint64_t new_cr3 = 0;
  int fd = open("/dev/tyche_enclave", O_RDWR);
  if(fd < 0) {
    fprintf(stderr, "Cannot open device driver.\n");
    exit(1);
  }
  if(ioctl(fd, TYCHE_ENCLAVE_DBG, handle) != 0) {
    fprintf(stderr, "DBG ioctl does not work\n");
    exit(1);
  }
  /*printf("\n\n New cr3 %llx\n\n", new_cr3);
  asm volatile (
    "movq $0xdeadbeef, %%rax\n\t"
    "movq %0, %%rcx\n\t"
    "vmcall\n\t"
    "movq $0xdeadbabe, %%rcx\n\t"
    "movq $0x500, %%rax\n\t"
    "vmcall"
    :
    : "rm" (new_cr3)
    : "rax", "rcx");*/
  close(fd);
}


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
    .flags = TE_READ|TE_USER|TE_WRITE,
    .tpe = Shared,
    .extra = NULL,
  };

  tyche_encl_handle_t handle;
  domain_id_t domain_handle;
  if (load_enclave(trusted, &handle, &domain_handle, &extra) != 0) {
    fprintf(stderr, "Unable to load the enclave.\n");
    exit(1);
  } 
  printf("We have a handle! %llx\n", handle);
  // TODO get it from the enclave loader.
  printf("Rcx should be: %llx\n", handle);
  printf("Rdx should be: %llx\n", 0x401000);
  printf("Rsi should be: %llx\n", shared);
  printf("The vmcall_gate is at %llx.\n", library->vmcall_gate);
  test_debugging(domain_handle);
  //library->vmcall_gate(domain_handle, (target_func_t)0x401000, shared);
  return 0;
}
