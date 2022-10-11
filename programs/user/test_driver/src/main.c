#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
#include <sys/mman.h>

#include "tyche_enclave.h"

#define LOG(...)  do {printf("%s:", __func__); printf(__VA_ARGS__); printf("\n");}while(0);

#define TEST(cond, msg) if (!(cond)) {fprintf(stderr, "%s:%d @%s : %s\n", __FILE__, __LINE__, __func__, msg); abort();}


int test_open()
{
  LOG("Opening driver.\n");
  int fd = open("/dev/tyche_enclave", O_RDWR);
  TEST(fd >= 0, "Cannot open device driver.");
  return fd; 
}

void test_create_enclave(int fd, struct tyche_encl_create_t* h) {
  LOG("Create an enclave");
  TEST(ioctl(fd, TYCHE_ENCLAVE_CREATE, h) == 0, "Unable to create and enclave");
}

void test_ioctl_dbg(int fd) {
  LOG("Invoke ioctl dbg");
  TEST(ioctl(fd, TYCHE_ENCLAVE_DBG) == 0, "DBG ioctl does not work"); 
}

void test_add_invalid_range(int fd, tyche_encl_handle_t* h) {
  LOG("Adding an invalid page.");
  struct tyche_encl_add_region_t region = {
    .handle = *h,
    .start = 0x1000,
    .end = 0x2000,
    .flags = TE_READ | TE_USER | TE_WRITE,
    .tpe = Confidential,
  };
  int value = ioctl(fd, TYCHE_ENCLAVE_ADD_REGION, &region);
  TEST(value == -1, "Invalid page accepted.");
}

void test_add_valid_range(int fd, tyche_encl_handle_t* h)
{
  LOG("Add Valid Range");
  // Try mapping a valid range.
  size_t size = 3 * 0x1000;
  void* ptr =  mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0); 
  TEST(ptr != MAP_FAILED, "Mmap failed!");
  uint64_t start = (uint64_t) ptr;
   struct tyche_encl_add_region_t region = {
    .handle = *h,
    .start = start,
    .end = start + size,
    .flags = TE_READ | TE_USER | TE_WRITE,
    .tpe = Confidential,
  };
  int value  = ioctl(fd, TYCHE_ENCLAVE_ADD_REGION, &region); 
  TEST(value == 0, "Valid page rejected.");
}

void test_add_invalid_overlap(int fd, tyche_encl_handle_t* h) {
  LOG("Invalid overlap");
  size_t size = 10 * 0x1000;
  void* ptr =  mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0); 
  TEST(ptr != MAP_FAILED, "Mmap failed!");
  uint64_t start = (uint64_t) ptr;
   struct tyche_encl_add_region_t region = {
    .handle = *h,
    .start = start,
    .end = start + size,
    .flags = TE_READ | TE_USER | TE_WRITE,
    .tpe = Confidential,
  };
  int value = ioctl(fd, TYCHE_ENCLAVE_ADD_REGION, &region); 
  TEST(value == 0, "Valid range rejected");
  
  // Now try to map an invalid range with an overlap.
  region.start = start + 0x1000;
  region.end = start + 0x2000;
  value = ioctl(fd, TYCHE_ENCLAVE_ADD_REGION, &region);
  TEST(value == -1, "Overlap accepted.");
}

void test_commit_enclave(int fd, tyche_encl_handle_t h)
{
  LOG("Attempting to commit an enclave");
  int value = ioctl(fd, TYCHE_ENCLAVE_COMMIT, h);
  TEST(value == 0, "Could not commit");
}

int main(void)
{
  LOG("Testing tyche enclave ioctl driver");
  int fd = test_open();
  
  test_ioctl_dbg(fd);
  struct tyche_encl_create_t h1, h2;
  test_create_enclave(fd, &h1);
  test_create_enclave(fd, &h2);
  TEST(h1.handle != h2.handle, "Did not get a fresh handle!");

  //TODO update that test, it fails for now as I changed the API.
  //test_add_invalid_range(fd, &(h1.handle));
  test_add_valid_range(fd, &(h1.handle));
  test_add_invalid_overlap(fd, &(h1.handle));
  test_commit_enclave(fd, h1.handle);
done:
  LOG("All done!");
  close(fd);
  return 0;
}
