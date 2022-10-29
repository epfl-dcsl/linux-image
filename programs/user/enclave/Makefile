all: enclave a.out 

# All the library dependencies.
ELF64=../../libs/elf64
LOADER=../../libs/enclave
DRIVER=../../../modules/libraries/enclave
CAPA=../../libs/capabilities

# All the includes
INCLUDES=-I$(ELF64)/include -I$(LOADER)/include -I$(DRIVER)/include -I$(CAPA)/include

# Dynamic libraries
DYN_LIBS= -l:encl_loader.so -lelf64

# Destination for installation.
BUSYBOX=../../../builds/initramfs/x86-busybox/home/tyche/enclaves

a.out: src/main.c enclave 
	mkdir -p libs
	make -C $(ELF64)
	make -C $(LOADER)
	cp $(ELF64)/libelf64.so libs/
	cp $(LOADER)/encl_loader.so libs/
	cp $(LOADER)/encl.so libs/
	gcc -g $(INCLUDES) -Wl,-R ./libs/ -L./libs/ -o $@ $< $(DYN_LIBS)

enclave: trusted/main.c trusted/asm.S
	gcc -g -nostdlib -static -o $@ $^

install: a.out enclave
	mkdir -p $(BUSYBOX)
	cp -r libs $(BUSYBOX) 
	cp -t $(BUSYBOX) $^


.PHONY: clean

clean:
	rm enclave

