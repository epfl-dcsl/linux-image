#include <stdio.h>
#include <stdint.h>

char secret[4096] __attribute__((aligned(4096)));

__attribute__((optimize("align-functions=4096")))
void my_aligned_func() {
  secret[0] = 'a'; 
}


void vmcall(uint64_t page) {
      asm volatile (
          "movq $0x777, %%rax\n\t"
          "movq %0, %%rcx\n\t"
          "vmcall"
          : 
          : "r" (page)
          : "rax", "rbx", "memory");
}

int main(void) {
  printf("Hello world %c\n", secret[0]);
  my_aligned_func();
  printf("The secret value %c\n", secret[0]);
  vmcall((uint64_t)my_aligned_func);
  vmcall((uint64_t)&secret);
  return 0;
}
