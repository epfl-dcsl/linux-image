.text

.globl encl_start
encl_start:
  movq $0x500, %rax
  movq $0xbedb0b, %rcx
  vmcall
