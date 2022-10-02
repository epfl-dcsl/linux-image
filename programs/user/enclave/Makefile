all: enclave

CURRENT_DIR=$(shell pwd)
TYCHE_DRIVER_INCL_PATH=../../../modules/libraries/enclave/include/

enclave: src/main.c
	gcc -I$(TYCHE_DRIVER_INCL_PATH) $< -o $@

.PHONY: clean install

install: enclave
	cp enclave ../../../builds/initramfs/x86-busybox/home/tyche/programs/ 

clean:
	rm enclave
