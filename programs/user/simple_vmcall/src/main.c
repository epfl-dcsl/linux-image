

__attribute__((force_align_arg_pointer)) void _start() {
    asm("movl $0x888, %eax;"
        "movl $0x777, %ebx;"
        "vmcall");
  __builtin_unreachable();
}
