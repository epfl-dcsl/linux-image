const char* message = "Hello world!\n\0";


char encl_stack[0x4000] __attribute__((section(".encl_stack")));


// Puts hello world inside the shared dest buffer.
void trusted_entry(void* dest)
{
  // Handmade memcpy.
  char* ptr = (char*) dest;
  for (int i = 0; i < 14; i++) {
    ptr = ptr + i;
    *ptr = message[i];
  } 
}

// Just to look good.
int _start() {
  return 0;
}
