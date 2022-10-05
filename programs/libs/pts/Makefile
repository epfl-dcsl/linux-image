all: libpts.a

libpts.a: src/lib.c src/x86_64_pt.c include/pts_api.h include/x86_64_pt.h
	gcc -nostdlib -c -Iinclude/ -o pts.o src/lib.c
	gcc -nostdlib -c -Iinclude/ -o x86.o src/x86_64_pt.c
	ar rcs $@ pts.o x86.o
	rm pts.o x86.o
