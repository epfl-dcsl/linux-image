const char* message = "Hello World!\n\t";

char encl_stack[0x4000] __attribute__((section(".encl_stack")));


void print_message(void* dest)
{
  // Handmade memcpy.
  char* ptr = (char*) dest;
  for (int i = 0; i < 14; i++) {
    ptr[i] = message[i];
  } 
}

// Puts hello world inside the shared dest buffer.
void trusted_entry(unsigned long ret_handle, void* args)
{
  if (args == 0) {
    asm(
      "movq $0x500, %%rax\n\t"
      "movq $0xbadb1, %%rcx\n\t"
      "vmcall"
      :
      :
      : "rax", "rcx", "memory");
  }
  print_message(args);
  // Use the return handle.
  asm(
    "movq $0x999, %%rax\n\t"
    "movq %0, %%rcx\n\t"
    "vmcall"
    :
    : "rm" (ret_handle)
    : "rax", "rcx", "memory");
}

int fibonnacci(int n)
{
  if (n <= 0) {
    return 0;
  } else if (n == 1) {
    return 1;
  }
  return (fibonnacci(n-1) + fibonnacci(n-2));
}

void fibonnacci_out()
{
  fibonnacci(10);
}

// Just to look good.
int _start() {
    /* exit system call */
    asm("movl $1,%eax;"
        "xorl %ebx,%ebx;"
        "int  $0x80"
    );
}
