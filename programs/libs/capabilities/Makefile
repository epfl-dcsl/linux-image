all: libcapabilities.a libcapabilities.so

LIB_DLL=../dll

libcapabilities.so: src/lib.c include/tyche_capabilities.h
	gcc -g -nostdlib -shared -Iinclude/ -I$(LIB_DLL)/include -o $@ -fPIC $< 

libcapabilities.a: src/lib.c include/tyche_capabilities.h
	gcc -g -nostdlib -c -Iinclude/ -I$(LIB_DLL)/include -o lib.o $<
	ar rcs $@ lib.o
	rm lib.o

.PHONY: clean

clean:
	rm libcapabilities.so libcapabilities.a

