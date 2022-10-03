all: enclave.so

enclave.so: trusted/lib.c
	gcc -nostdlib -shared -o trusted.so -fPIC trusted/lib.c

