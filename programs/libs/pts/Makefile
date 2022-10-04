all: libpts.a

libpts.a: src/lib.c include/pts_api.h
	gcc -nostdlib -c -Iinclude/ -o pts.o src/lib.c
	ar rcs $@ pts.o
	rm pts.o
