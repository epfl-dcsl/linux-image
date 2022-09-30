all: enclave

enclave: src/main.c
	gcc $< -o $@

.PHONY: clean install

install: enclave
	cp enclave ../../../builds/initramfs/x86-busybox/home/tyche/programs/ 

clean:
	rm enclave
