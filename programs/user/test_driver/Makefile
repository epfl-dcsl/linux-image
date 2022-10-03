all: test_driver 

CURRENT_DIR=$(shell pwd)
TYCHE_DRIVER_INCL_PATH=../../../modules/libraries/enclave/include/

test_driver: src/main.c
	gcc -I$(TYCHE_DRIVER_INCL_PATH) $< -o $@

.PHONY: clean install

install: test_driver 
	cp test_driver ../../../builds/initramfs/x86-busybox/home/tyche/programs/ 

clean:
	rm test_driver 
