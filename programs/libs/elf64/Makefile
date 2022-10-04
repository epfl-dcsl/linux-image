all: elf64.so

elf64.so: src/lib.c
	gcc -shared -Iinclude/ -o $@ -fPIC $< 

.PHONY: clean

clean:
	rm elf64.so
