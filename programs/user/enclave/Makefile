all: enclave 

enclave: trusted/main.c
	gcc -g -nostdlib -static -o $@ $<

.PHONY: clean

clean:
	rm enclave

